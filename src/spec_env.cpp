// Calculate spectral envelope using the discrete cepstrum envelope method,
// described in:
//
// "Regularization Techniques for Discrete Cepstrum Estimation"
// Olivier Cappe and Eric Moulines, IEEE Signal Processing Letters, Vol. 3
// No.4, April 1996
//
// Based on code in libsms by Rich Eakin and Jordi Janer at the MTG, UPF, Barcelona.

#include "spec_env.h"

using namespace metamorph;

sample metamorph::sine(sample theta, sample sine_incr, sample* sine_table) {
    int i;
    theta = theta - floor(theta * INV_TWO_PI) * TWO_PI;

    if(theta < 0) {
        i = 0.5 - (theta * sine_incr);
        return -(sine_table[i]);
    }
    else {
        i = theta * sine_incr + .5;
        return sine_table[i];
    }
}

SpectralEnvelope::SpectralEnvelope(int order, int env_size) {
    _lambda = 0.00001;
    _max_freq = 22050;

    _d.num_coeffs = order + 1;
    _d.num_points = env_size;
    _d.cepstrum = new sample[_d.num_coeffs];
    memset(_d.cepstrum, 0, sizeof(sample) * _d.num_coeffs);

    _d.M = gsl_matrix_alloc(_d.num_points, _d.num_coeffs);
    _d.Mt = gsl_matrix_alloc(_d.num_coeffs, _d.num_points);
    _d.R = gsl_matrix_calloc(_d.num_coeffs, _d.num_coeffs);
    _d.MtMR = gsl_matrix_alloc(_d.num_coeffs, _d.num_coeffs);
    _d.Xk = gsl_vector_alloc(_d.num_points);
    _d.MtXk = gsl_vector_alloc(_d.num_coeffs);
    _d.C = gsl_vector_alloc(_d.num_coeffs);
    _d.Perm = gsl_permutation_alloc(_d.num_coeffs);

    sample theta = 0.f;
    _d.sine_table = new sample[SINE_TABLE_SIZE];
    _d.sine_scale = (sample)(TWO_PI) / (sample)(SINE_TABLE_SIZE - 1);
    _d.sine_incr = 1.f / _d.sine_scale;

    for(int i = 0; i < SINE_TABLE_SIZE; i++) {
        theta = _d.sine_scale * i;
        _d.sine_table[i] = sin(theta);
    }

    _d.fft_size = env_size * 2;
    _d.fft_in = (sample*) fftw_malloc(sizeof(sample) * _d.fft_size);
    _d.fft_out = (fftw_complex*) fftw_malloc(sizeof(fftw_complex) * _d.fft_size);
    _d.fft_plan = fftw_plan_dft_r2c_1d(_d.fft_size, _d.fft_in,
                                       _d.fft_out, FFTW_ESTIMATE);
    memset(_d.fft_in, 0, sizeof(sample) * _d.fft_size);
}

SpectralEnvelope::~SpectralEnvelope() {
    if(_d.cepstrum) delete [] _d.cepstrum;

    gsl_matrix_free(_d.M);
    gsl_matrix_free(_d.Mt);
    gsl_matrix_free(_d.R);
    gsl_matrix_free(_d.MtMR);
    gsl_vector_free(_d.Xk);
    gsl_vector_free(_d.MtXk);
    gsl_vector_free(_d.C);
    gsl_permutation_free(_d.Perm);

    if(_d.sine_table) delete [] _d.sine_table;

    fftw_destroy_plan(_d.fft_plan);
    fftw_free(_d.fft_in);
    fftw_free(_d.fft_out);
}

int SpectralEnvelope::env_size() {
    return _d.num_points;
}

void SpectralEnvelope::env_size(int new_env_size) {
    fftw_destroy_plan(_d.fft_plan);
    fftw_free(_d.fft_in);
    fftw_free(_d.fft_out);

    _d.num_points = new_env_size;
    _d.fft_size = new_env_size * 2;
    _d.fft_in = (sample*) fftw_malloc(sizeof(sample) * _d.fft_size);
    _d.fft_out = (fftw_complex*) fftw_malloc(sizeof(fftw_complex) * _d.fft_size);
    _d.fft_plan = fftw_plan_dft_r2c_1d(_d.fft_size, _d.fft_in,
                                       _d.fft_out, FFTW_ESTIMATE);
    memset(_d.fft_in, 0, sizeof(sample) * _d.fft_size);
}

void SpectralEnvelope::discrete_cepstrum(
    int freqs_size, sample* freqs, sample* mags,
    int cepstrum_size, sample* cepstrum) {

    int s;
    sample factor;
    sample norm = PI / (sample) _max_freq;

    // compute matrix M
    for(int i = 0; i < freqs_size; i++) {
        gsl_matrix_set(_d.M, i, 0, 1.f);  // first column is 1
        for(int k = 1; k < cepstrum_size; k++) {
            gsl_matrix_set(
                _d.M, i, k , 2.f *
                sine(PI_2 + norm * k * freqs[i], _d.sine_incr, _d.sine_table)
            );
        }
    }

    // compute transpose of M
    gsl_matrix_transpose_memcpy(_d.Mt, _d.M);

    // compute R diagonal matrix
    factor = COEF * (_lambda / (1.f - _lambda));
    for(int k = 0; k < cepstrum_size; k++) {
        gsl_matrix_set(_d.R, k, k, factor * powf((sample)k, 2.f));
    }

    // MtM = Mt * M
    gsl_blas_dgemm(CblasNoTrans, CblasNoTrans, 1.f, _d.Mt, _d.M, 0.0, _d.MtMR);

    // add R to make MtMR
    gsl_matrix_add(_d.MtMR, _d.R);

    // set Mag in X and multiply with Mt to get MtXk
    for(int k = 0; k < freqs_size; k++) {
        gsl_vector_set(_d.Xk, k, log(mags[k]));
    }
    gsl_blas_dgemv(CblasNoTrans, 1.f, _d.Mt, _d.Xk, 0.f, _d.MtXk);

    // solve x (the cepstrum) in Ax = b, where A=MtMR and b=MtXk
    // using LU decomposition way
    gsl_linalg_LU_decomp(_d.MtMR, _d.Perm, &s);
    gsl_linalg_LU_solve(_d.MtMR, _d.Perm, _d.MtXk, _d.C);

    // copy C to cepstrum
    for(int i = 0; i  < cepstrum_size; i++) {
        cepstrum[i] = gsl_vector_get(_d.C, i);
    }
}

void SpectralEnvelope::discrete_cepstrum_envelope(
    int cepstrum_size, sample* cepstrum, int env_size, sample* env) {
    _d.fft_in[0] = cepstrum[0] * 0.5;
    for(int i = 1; i < cepstrum_size - 1; i++) {
        _d.fft_in[i] = cepstrum[i];
    }

    fftw_execute(_d.fft_plan);

    for(int i = 0; i < env_size; i++) {
        env[i] = powf(EXP, 2.0 * _d.fft_out[i][0]);
    }
}

void SpectralEnvelope::env(int num_peaks, sample* freqs, sample* mags,
                           int env_size, sample* e){
    if(num_peaks <= 0) {
        return;
    }

    discrete_cepstrum(num_peaks, freqs, mags, _d.num_coeffs, _d.cepstrum);
    discrete_cepstrum_envelope(_d.num_coeffs, _d.cepstrum, env_size, e);
}

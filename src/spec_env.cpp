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
    _cepstrum_size = order + 1;
    _env_size = env_size;
    _num_peaks = env_size;
    create_cepstrum_data();
}

SpectralEnvelope::~SpectralEnvelope() {
    destroy_cepstrum_data();
}

void SpectralEnvelope::create_cepstrum_data() {
    _cepstrum = new sample[_cepstrum_size];
    memset(_cepstrum, 0, sizeof(sample) * _cepstrum_size);

    _M = gsl_matrix_alloc(_num_peaks, _cepstrum_size);
    _Mt = gsl_matrix_alloc(_cepstrum_size, _num_peaks);
    _R = gsl_matrix_calloc(_cepstrum_size, _cepstrum_size);
    _MtMR = gsl_matrix_alloc(_cepstrum_size, _cepstrum_size);
    _Xk = gsl_vector_alloc(_num_peaks);
    _MtXk = gsl_vector_alloc(_cepstrum_size);
    _C = gsl_vector_alloc(_cepstrum_size);
    _Perm = gsl_permutation_alloc(_cepstrum_size);

    sample theta = 0.f;
    _sine_table = new sample[SINE_TABLE_SIZE];
    _sine_scale = (sample)(TWO_PI) / (sample)(SINE_TABLE_SIZE - 1);
    _sine_incr = 1.f / _sine_scale;
    for(int i = 0; i < SINE_TABLE_SIZE; i++) {
        theta = _sine_scale * i;
        _sine_table[i] = sin(theta);
    }

    _fft_size = _env_size * 2;
    _fft_in = (sample*) fftw_malloc(sizeof(sample) * _fft_size);
    _fft_out = (fftw_complex*) fftw_malloc(sizeof(fftw_complex) * _fft_size);
    _fft_plan = fftw_plan_dft_r2c_1d(
        _fft_size, _fft_in, _fft_out, FFTW_ESTIMATE
    );
    memset(_fft_in, 0, sizeof(sample) * _fft_size);
}

void SpectralEnvelope::destroy_cepstrum_data() {
    if(_cepstrum) delete [] _cepstrum;

    gsl_matrix_free(_M);
    gsl_matrix_free(_Mt);
    gsl_matrix_free(_R);
    gsl_matrix_free(_MtMR);
    gsl_vector_free(_Xk);
    gsl_vector_free(_MtXk);
    gsl_vector_free(_C);
    gsl_permutation_free(_Perm);

    if(_sine_table) delete [] _sine_table;

    fftw_destroy_plan(_fft_plan);
    fftw_free(_fft_in);
    fftw_free(_fft_out);
}

void SpectralEnvelope::reset() {
    destroy_cepstrum_data();
    create_cepstrum_data();
}

int SpectralEnvelope::env_size() {
    return _env_size;
}

void SpectralEnvelope::env_size(int new_env_size) {
    fftw_destroy_plan(_fft_plan);
    fftw_free(_fft_in);
    fftw_free(_fft_out);

    _env_size = new_env_size;

    _fft_size = _env_size * 2;
    _fft_in = (sample*) fftw_malloc(sizeof(sample) * _fft_size);
    _fft_out = (fftw_complex*) fftw_malloc(sizeof(fftw_complex) * _fft_size);
    _fft_plan = fftw_plan_dft_r2c_1d(
        _fft_size, _fft_in, _fft_out, FFTW_ESTIMATE
    );
    memset(_fft_in, 0, sizeof(sample) * _fft_size);
}

void SpectralEnvelope::discrete_cepstrum(int freqs_size, sample* freqs,
                                         sample* mags) {
    int s;
    sample factor;
    sample norm = PI / (sample) _max_freq;

    // compute matrix M
    for(int i = 0; i < freqs_size; i++) {
        gsl_matrix_set(_M, i, 0, 1.f);  // first column is 1
        for(int k = 1; k < _cepstrum_size; k++) {
            gsl_matrix_set(
                _M, i, k , 2.f *
                sine(PI_2 + norm * k * freqs[i], _sine_incr, _sine_table)
            );
        }
    }

    // compute transpose of M
    gsl_matrix_transpose_memcpy(_Mt, _M);

    // compute R diagonal matrix
    factor = COEF * (_lambda / (1.f - _lambda));
    for(int k = 0; k < _cepstrum_size; k++) {
        gsl_matrix_set(_R, k, k, factor * powf((sample)k, 2.f));
    }

    // MtM = Mt * M
    gsl_blas_dgemm(CblasNoTrans, CblasNoTrans, 1.f, _Mt, _M, 0.0, _MtMR);

    // add R to make MtMR
    gsl_matrix_add(_MtMR, _R);

    // set Mag in X and multiply with Mt to get MtXk
    for(int k = 0; k < freqs_size; k++) {
        gsl_vector_set(_Xk, k, log(mags[k]));
    }
    gsl_blas_dgemv(CblasNoTrans, 1.f, _Mt, _Xk, 0.f, _MtXk);

    // solve x (the cepstrum) in Ax = b, where A=MtMR and b=MtXk
    // using LU decomposition way
    gsl_linalg_LU_decomp(_MtMR, _Perm, &s);
    gsl_linalg_LU_solve(_MtMR, _Perm, _MtXk, _C);

    // copy C to cepstrum
    for(int i = 0; i  < _cepstrum_size; i++) {
        _cepstrum[i] = gsl_vector_get(_C, i);
    }
}

void SpectralEnvelope::discrete_cepstrum_envelope(int env_size, sample* env) {
    memset(_fft_in, 0, sizeof(sample) * _fft_size);

    _fft_in[0] = _cepstrum[0] * 0.5;
    for(int i = 1; i < _cepstrum_size - 1; i++) {
        _fft_in[i] = _cepstrum[i];
    }

    fftw_execute(_fft_plan);

    for(int i = 0; i < env_size; i++) {
        env[i] = powf(E, 2.0 * _fft_out[i][0]);
    }
}

void SpectralEnvelope::env(int num_peaks, sample* freqs, sample* mags,
                           int env_size, sample* e){
    if(num_peaks <= 0) {
        memset(e, 0, sizeof(sample) * env_size);
        return;
    }

    if(_num_peaks != num_peaks) {
        destroy_cepstrum_data();
        _num_peaks = num_peaks;
        create_cepstrum_data();
    }

    discrete_cepstrum(num_peaks, freqs, mags);
    discrete_cepstrum_envelope(env_size, e);
}

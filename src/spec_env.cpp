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

sample metamorph::sine(sample theta, sample sine_incr, std::vector<sample>& sine_table) {
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
    _env_size = env_size;
    _num_peaks = env_size;
    _cepstrum.resize(order + 1);
    create_cepstrum_data();
}

SpectralEnvelope::~SpectralEnvelope() {
    destroy_cepstrum_data();
}

void SpectralEnvelope::create_cepstrum_data() {

    _M = gsl_matrix_alloc(_num_peaks, _cepstrum.size());
    _Mt = gsl_matrix_alloc(_cepstrum.size(), _num_peaks);
    _R = gsl_matrix_calloc(_cepstrum.size(), _cepstrum.size());
    _MtMR = gsl_matrix_alloc(_cepstrum.size(), _cepstrum.size());
    _Xk = gsl_vector_alloc(_num_peaks);
    _MtXk = gsl_vector_alloc(_cepstrum.size());
    _C = gsl_vector_alloc(_cepstrum.size());
    _Perm = gsl_permutation_alloc(_cepstrum.size());

    sample theta = 0.f;
    _sine_table.resize(SINE_TABLE_SIZE);
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
    gsl_matrix_free(_M);
    gsl_matrix_free(_Mt);
    gsl_matrix_free(_R);
    gsl_matrix_free(_MtMR);
    gsl_vector_free(_Xk);
    gsl_vector_free(_MtXk);
    gsl_vector_free(_C);
    gsl_permutation_free(_Perm);

    fftw_destroy_plan(_fft_plan);
    fftw_free(_fft_in);
    fftw_free(_fft_out);
}

void SpectralEnvelope::reset() {
    destroy_cepstrum_data();
    create_cepstrum_data();
}

sample SpectralEnvelope::max_frequency() {
    return _max_freq;
}

void SpectralEnvelope::max_frequency(sample new_max_freq) {
    _max_freq = new_max_freq;
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

void SpectralEnvelope::discrete_cepstrum(std::vector<sample>& freqs,
                                         std::vector<sample>& mags) {
    int s;
    sample factor;
    sample norm = PI / (sample) _max_freq;

    // compute matrix M
    for(int i = 0; i < freqs.size(); i++) {
        gsl_matrix_set(_M, i, 0, 1.f);  // first column is 1
        for(int k = 1; k < _cepstrum.size(); k++) {
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
    for(int k = 0; k < _cepstrum.size(); k++) {
        gsl_matrix_set(_R, k, k, factor * powf((sample)k, 2.f));
    }

    // MtM = Mt * M
    gsl_blas_dgemm(CblasNoTrans, CblasNoTrans, 1.f, _Mt, _M, 0.0, _MtMR);

    // add R to make MtMR
    gsl_matrix_add(_MtMR, _R);

    // set Mag in X and multiply with Mt to get MtXk
    for(int k = 0; k < freqs.size(); k++) {
        gsl_vector_set(_Xk, k, log(mags[k]));
    }
    gsl_blas_dgemv(CblasNoTrans, 1.f, _Mt, _Xk, 0.f, _MtXk);

    // solve x (the cepstrum) in Ax = b, where A=MtMR and b=MtXk
    // using LU decomposition way
    gsl_linalg_LU_decomp(_MtMR, _Perm, &s);
    gsl_linalg_LU_solve(_MtMR, _Perm, _MtXk, _C);

    // copy C to cepstrum
    for(int i = 0; i  < _cepstrum.size(); i++) {
        _cepstrum[i] = gsl_vector_get(_C, i);
    }
}

void SpectralEnvelope::discrete_cepstrum_envelope(std::vector<sample>& env) {
    memset(_fft_in, 0, sizeof(sample) * _fft_size);

    _fft_in[0] = _cepstrum[0] * 0.5;
    for(int i = 1; i < _cepstrum.size() - 1; i++) {
        _fft_in[i] = _cepstrum[i];
    }

    fftw_execute(_fft_plan);

    for(int i = 0; i < env.size(); i++) {
        env[i] = powf(E, 2.0 * _fft_out[i][0]);
    }
}

void SpectralEnvelope::envelope(std::vector<sample>& freqs, std::vector<sample>& mags,
                                std::vector<sample>& env) {
    if(freqs.size() != mags.size()) {
        printf("Warning: sizes of frequency and magnitude peaks passed to "
               "spectral envelope function do not match, ignoring.\n");
        env.assign(env.size(), 0.f);
        return;
    }
    else if(freqs.size() <= 0) {
        env.assign(env.size(), 0.f);
        return;
    }

    if(_num_peaks != freqs.size()) {
        destroy_cepstrum_data();
        _num_peaks = freqs.size();
        create_cepstrum_data();
    }

    discrete_cepstrum(freqs, mags);
    discrete_cepstrum_envelope(env);
}

void SpectralEnvelope::envelope(int num_peaks, sample* freqs, sample* mags,
                                int env_size, sample* env) {
    std::vector<sample> f = vector<sample>(freqs, freqs + num_peaks);
    std::vector<sample> m = vector<sample>(mags, mags + num_peaks);
    std::vector<sample> e = vector<sample>(env_size);

    envelope(f, m, e);

    for(int i = 0; i < env_size; i++) {
        env[i] = e[i];
    }
}

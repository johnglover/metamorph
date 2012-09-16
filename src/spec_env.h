#ifndef METAMORPH_SPECENV_H
#define METAMORPH_SPECENV_H

// Calculate spectral envelope using the discrete cepstrum envelope method,
// described in:
//
// "Regularization Techniques for Discrete Cepstrum Estimation"
// Olivier Cappe and Eric Moulines, IEEE Signal Processing Letters, Vol. 3
// No.4, April 1996
//
// Based on code in libsms by Rich Eakin and Jordi Janer at the MTG, UPF, Barcelona.

#include <math.h>
#include <gsl/gsl_matrix.h>
#include <gsl/gsl_linalg.h>
#include <gsl/gsl_blas.h>
#include <fftw3.h>

#include "simpl/simpl.h"


namespace metamorph
{

#define PI 3.141592653589793238462643
#define TWO_PI 6.28318530717958647692
#define INV_TWO_PI (1 / TWO_PI)
#define PI_2 1.57079632679489661923
#define E 2.7182818284590451
#define SINE_TABLE_SIZE 4096
#define COEF (8 * powf(PI, 2))

typedef double sample;

sample sine(sample theta, sample sine_incr, sample* sine_table);


class SpectralEnvelope {
    private:
        sample _lambda;
        sample _max_freq;
        int _cepstrum_size;
        int _num_peaks;
        int _env_size;
        sample* _cepstrum;

        gsl_matrix* _M;
        gsl_matrix* _Mt;
        gsl_matrix* _R;
        gsl_matrix* _MtMR;
        gsl_vector* _Xk;
        gsl_vector* _MtXk;
        gsl_vector* _C;
        gsl_permutation* _Perm;

        sample _sine_scale;
        sample _sine_incr;
        sample* _sine_table;

        int _fft_size;
        sample* _fft_in;
        fftw_complex* _fft_out;
        fftw_plan _fft_plan;

        void create_cepstrum_data();
        void destroy_cepstrum_data();
        void reset();
        void discrete_cepstrum(int freqs_size, sample* freqs, sample* mags);
        void discrete_cepstrum_envelope(int env_size, sample* env);

    public:
        SpectralEnvelope(int order, int env_size);
        ~SpectralEnvelope();
        int env_size();
        void env_size(int new_env_size);
        void env(int num_peaks, sample* freqs, sample* mags,
                 int env_size, sample* e);
};


} // end of namespace metamorph

#endif

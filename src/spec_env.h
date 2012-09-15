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
#define SINE_TABLE_SIZE 4096
#define COEF (8 * powf(PI, 2))

typedef double sample;

typedef struct
{
    int num_points;
    int num_coeffs;
    sample* cepstrum;

    gsl_matrix* M;
    gsl_matrix* Mt;
    gsl_matrix* R;
    gsl_matrix* MtMR;
    gsl_vector* Xk;
    gsl_vector* MtXk;
    gsl_vector* C;
    gsl_permutation* Perm;

    sample sine_scale;
    sample sine_incr;
    sample* sine_table;

    int fft_size;
    sample* fft_in;
    fftw_complex* fft_out;
    fftw_plan fft_plan;
} CepstrumData;

sample sine(sample theta, sample sine_incr, sample* sine_table);

class SpectralEnvelope {
    private:
        int _order;
        int _num_coeffs;
        sample _lambda;
        sample _max_freq;
        CepstrumData _d;

        void discrete_cepstrum(int freqs_size, sample* freqs, sample* mags,
                               int cepstrum_size, sample* cepstrum);
        void discrete_cepstrum_envelope(int cepstrum_size, sample* cepstrum,
                                        int env_size, sample* env);

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

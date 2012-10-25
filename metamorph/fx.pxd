import numpy as np
cimport numpy as np
np.import_array()
from libcpp.vector cimport vector
from libcpp cimport bool

dtype = np.float64
ctypedef np.double_t dtype_t


cdef extern from "../src/fx.h" namespace "metamorph":
    cdef cppclass c_FX "metamorph::FX":
        c_FX()
        int frame_size()
        void frame_size(int new_frame_size)
        int hop_size()
        void hop_size(int new_hop_size)
        int max_partials()
        void max_partials(int new_max_partials)
        void add_harmonic_transformation(c_HarmonicTransformation* h)
        void clear_harmonic_transformations()
        double harmonic_scale()
        void harmonic_scale(double new_harmonic_scale)
        bool preserve_envelope()
        void preserve_envelope(bool preserve)
        double env_interp()
        void env_interp(double new_env_interp)
        void apply_envelope(int env_size, double* env)
        void clear_envelope()
        double residual_scale()
        void residual_scale(double new_residual_scale)
        double transient_scale()
        bool preserve_transients()
        void preserve_transients(bool preserve)
        bool transient_substitution()
        void transient_substitution(bool substitute)
        void new_transient(int new_transient_size, double* new_transient)
        void clear_new_transient()
        void transient_scale(double new_transient_scale)
        void process_frame(int input_size, double* input,
                           int output_size, double* output)
        void process(long input_size, double* input,
                     long output_size, double* output)


cdef extern from "../src/transformations.h" namespace "metamorph":
    cdef cppclass c_HarmonicTransformation "metamorph::HarmonicTransformation":
        c_HarmonicTransformation()

    cdef cppclass c_Transposition \
        "metamorph::Transposition"(c_HarmonicTransformation):
        c_Transposition()
        c_Transposition(double new_transposition)
        double transposition()
        void transposition(double new_transposition)

    cdef cppclass c_HarmonicDistortion \
        "metamorph::HarmonicDistortion"(c_HarmonicTransformation):
        c_HarmonicDistortion()
        c_HarmonicDistortion(double new_harmonic_distortion,
                             double new_fundamental)
        double harmonic_distortion()
        void harmonic_distortion(double new_harmonic_distortion)
        double fundamental_frequency()
        void fundamental_frequency(double new_fundamental_frequency)

cdef extern from "../src/time_scale.h" namespace "metamorph":
    cdef cppclass c_TimeScale "metamorph::TimeScale"(c_FX):
        c_TimeScale()
        double scale_factor()
        void scale_factor(double new_scale_factor)


cdef extern from "../src/spec_env.h" namespace "metamorph":
    cdef cppclass c_SpectralEnvelope "metamorph::SpectralEnvelope":
        c_SpectralEnvelope(int order, int env_size)
        int env_size()
        void env_size(int new_env_size)
        void envelope(int num_peaks, double* freqs, double* mags,
                      int env_size, double* e)

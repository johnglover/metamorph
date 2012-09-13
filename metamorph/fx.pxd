import numpy as np
cimport numpy as np
np.import_array()
from libcpp.vector cimport vector

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
        double harmonic_scale()
        void harmonic_scale(double new_harmonic_scale)
        double residual_scale()
        void residual_scale(double new_residual_scale)
        double transient_scale()
        void transient_scale(double new_transient_scale)
        double harmonic_distortion()
        void harmonic_distortion(double new_harmonic_distortion)
        double fundamental_frequency()
        void fundamental_frequency(double new_fundamental_frequency)
        void process_frame(int input_size, double* input,
                           int output_size, double* output)
        void process(long input_size, double* input,
                     long output_size, double* output)


cdef extern from "../src/time_scale.h" namespace "metamorph":
    cdef cppclass c_TimeScale "metamorph::TimeScale"(c_FX):
        c_TimeScale()
        double scale_factor()
        void scale_factor(double new_scale_factor)

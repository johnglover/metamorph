import numpy as np
cimport numpy as np
np.import_array()
from libcpp.vector cimport vector

dtype = np.float64
ctypedef np.double_t dtype_t


cdef extern from "../src/fx.h" namespace "metamorph":
    cdef cppclass c_FX "metamorph::FX":
        c_FX()
        int hop_size()
        void hop_size(int new_hop_size)
        void process_frame(int input_size, double* input,
                           int output_size, double* output)
        void process(long input_size, double* input,
                     long output_size, double* output)


cdef extern from "../src/time_scale.h" namespace "metamorph":
    cdef cppclass c_TimeScale "metamorph::TimeScale"(c_FX):
        c_TimeScale()
        double scale_factor()
        void scale_factor(double new_scale_factor)

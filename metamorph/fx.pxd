import numpy as np
cimport numpy as np
np.import_array()
from libcpp.vector cimport vector


cdef extern from "../src/fx.h" namespace "metamorph":
    cdef cppclass c_FX "metamorph::FX":
        c_FX()

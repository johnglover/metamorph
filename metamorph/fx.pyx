import numpy as np
cimport numpy as np
np.import_array()
from libcpp.vector cimport vector


cdef class FX:
    cdef c_FX* thisptr

    def __cinit__(self):
        self.thisptr = new c_FX()

    def __dealloc__(self):
        if self.thisptr:
            del self.thisptr

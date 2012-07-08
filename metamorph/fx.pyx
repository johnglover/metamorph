import numpy as np
cimport numpy as np
np.import_array()
from libcpp.vector cimport vector

cimport fx


cdef class FX:
    cdef c_FX* thisptr

    def __cinit__(self):
        self.thisptr = new c_FX()

    def __dealloc__(self):
        if self.thisptr:
            del self.thisptr

    property hop_size:
        def __get__(self): return self.thisptr.hop_size()
        def __set__(self, int n): self.thisptr.hop_size(n)

    def process_frame(self, np.ndarray[dtype_t, ndim=1] audio):
        cdef np.ndarray[dtype_t, ndim=1] output = np.zeros(len(audio))
        self.thisptr.process_frame(len(audio), <double*> audio.data,
                                   len(output), <double*> output.data)
        return output

    def process(self, np.ndarray[dtype_t, ndim=1] audio):
        cdef np.ndarray[dtype_t, ndim=1] output = np.zeros(len(audio))
        self.thisptr.process(len(audio), <double*> audio.data,
                             len(output), <double*> output.data)
        return output


cdef class TimeScale(FX):
    def __cinit__(self):
        if self.thisptr:
            del self.thisptr
        self.thisptr = new c_TimeScale()

    def __dealloc__(self):
        if self.thisptr:
            del self.thisptr
            self.thisptr = <c_TimeScale*>0

    property scale_factor:
        def __get__(self): return (<c_TimeScale*>self.thisptr).scale_factor()
        def __set__(self, double n): (<c_TimeScale*>self.thisptr).scale_factor(n)

    def process(self, np.ndarray[dtype_t, ndim=1] audio):
        cdef np.ndarray[dtype_t, ndim=1] output = np.zeros(len(audio) * self.scale_factor)
        self.thisptr.process(len(audio), <double*> audio.data,
                             len(output), <double*> output.data)
        return output

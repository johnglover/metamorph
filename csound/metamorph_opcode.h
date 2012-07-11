#ifndef METAMORPH_OPCODE_H
#define METAMORPH_OPCODE_H

#include "csdl.h"
#include "metamorph.h"

typedef struct NoTr NoTr;

typedef struct {
    // structure holding csound global data (esr, ksmps, etc.)
    OPDS h;  	

    // output
    MYFLT *output;

    // parameters
    MYFLT *input, *harmonic_scale, *residual_scale, *transient_scale;    

    // opcode internal data
    NoTr *data;
} NOTR;

#endif

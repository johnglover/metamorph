#ifndef METAMORPH_OPCODE_H
#define METAMORPH_OPCODE_H

#include "csdl.h"
#include "metamorph.h"

// ----------------------------------------------------------------------------
// Main Metamorph Opcode
// ----------------------------------------------------------------------------
typedef struct Mm Mm;

typedef struct {
    // structure holding csound global data (esr, ksmps, etc.)
    OPDS h;  	

    // output
    MYFLT *output;

    // parameters
    MYFLT *input, *harmonic_scale, *residual_scale, *transient_scale,
          *transposition_factor, *preserve_envelope;

    // opcode internal data
    Mm *data;
} MM;

#endif

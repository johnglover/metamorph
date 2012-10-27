#include "metamorph_opcode.h"

using namespace metamorph;

// ----------------------------------------------------------------------------
// Main Metamorph Opcode
// ----------------------------------------------------------------------------
struct Mm {
    FX* fx;
    Transposition* transpose;
    Mm(CSOUND *csound, MM* params);
    ~Mm(void);
};

Mm::Mm(CSOUND *csound, MM* params) {
    fx = new FX();
    fx->hop_size(csound->ksmps);

    transpose = new Transposition();
    fx->add_harmonic_transformation(transpose);
}

Mm::~Mm() {
    delete fx;
    delete transpose;
}

extern "C" int mm_cleanup(CSOUND *, void * p);

extern "C" int mm_setup(CSOUND *csound, MM* p) {
    p->data = new Mm(csound, p);

    csound->RegisterDeinitCallback(
        csound, p, (int (*)(CSOUND*, void*))mm_cleanup
    );
    return OK;
}

extern "C" int mm(CSOUND *csound, MM* p) {
    int nsmps = csound->ksmps;
    MYFLT *output = p->output;
    MYFLT *input = p->input;

    memset(output, 0, sizeof(MYFLT) * nsmps);

    p->data->fx->harmonic_scale((*p->harmonic_scale));
    p->data->fx->residual_scale((*p->residual_scale));
    p->data->fx->transient_scale((*p->transient_scale));

    if(*p->preserve_transients == 1) {
        p->data->fx->preserve_transients(true);
    }
    else {
        p->data->fx->preserve_transients(false);
    }

    p->data->transpose->transposition(*p->transposition_factor);

    p->data->fx->process_frame(nsmps, input, nsmps, output);
    return OK;
}

extern "C" int mm_cleanup(CSOUND *csound, void * p) {
    MM* pp = (MM *)p;
    delete pp->data;
    pp->data = 0;
    return OK;
}

// ----------------------------------------------------------------------------
// csound plugin
// ----------------------------------------------------------------------------
extern "C" {
    static OENTRY localops[] =
    {
        {(char *)"mm",  sizeof(MM),  5, (char *)"a", (char *)"akkkpoooo",
         (SUBR)mm_setup, 0, (SUBR)mm}
    };

    LINKAGE
}

#include "metamorph_opcode.h"

using namespace metamorph;

// ----------------------------------------------------------------------------
// Noisiness/Transience (notr) definition
// ----------------------------------------------------------------------------
struct NoTr {
    FX* fx;
    NoTr(CSOUND *csound, NOTR* params);
    ~NoTr(void);
};

// ----------------------------------------------------------------------------
// Noisiness/Transience (notr) contructor/destructor
// ----------------------------------------------------------------------------
NoTr::NoTr(CSOUND *csound, NOTR* params) {
    fx = new FX();
    fx->hop_size(csound->ksmps);
}

NoTr::~NoTr() {
    delete fx;
}

extern "C" int notr_cleanup(CSOUND *, void * p);

// ----------------------------------------------------------------------------
// Noisiness/Transience (notr) setup
// ----------------------------------------------------------------------------
extern "C" int notr_setup(CSOUND *csound, NOTR* p) {
    p->data = new NoTr(csound, p);
    csound->RegisterDeinitCallback(
        csound, p, (int (*)(CSOUND*, void*))notr_cleanup
    );
    return OK;
}

// ---------------------------------------------------------------------------
// Noisiness/Transience (notr)
// ---------------------------------------------------------------------------
extern "C" int notr(CSOUND *csound, NOTR* p) {
    int nsmps = csound->ksmps;
    MYFLT *output = p->output;
    MYFLT *input = p->input;

    p->data->fx->harmonic_scale((*p->harmonic_scale));
    p->data->fx->residual_scale((*p->residual_scale));
    p->data->fx->transient_scale((*p->transient_scale));

    p->data->fx->process_frame(nsmps, input, nsmps, output);

    return OK;
}

// ----------------------------------------------------------------------------
// Noisiness/Transience (notr) cleanup
// ----------------------------------------------------------------------------
extern "C" int notr_cleanup(CSOUND *csound, void * p) {
    NOTR* pp = (NOTR *)p;
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
        {(char *)"mmnotr",  sizeof(NOTR),  5, (char *)"a", (char *)"akkk",
         (SUBR)notr_setup, 0, (SUBR)notr}
    };

    LINKAGE
}

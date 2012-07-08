#include "fx.h"

using namespace metamorph;

FX::FX() {
    _hop_size = 512;
    _current_segment = NONE;

    _pd = new simpl::SMSPeakDetection();
    _pt = new simpl::SMSPartialTracking();
    _synth = new simpl::SMSSynthesis();
    _residual = new simpl::SMSResidual();

    _pd->hop_size(_hop_size);
    _synth->hop_size(_hop_size);
    _residual->hop_size(_hop_size);
}

FX::~FX() {
    if(_pd) {
        delete _pd;
    }
    if(_pt) {
        delete _pt;
    }
    if(_synth) {
        delete _synth;
    }
    if(_residual) {
        delete _residual;
    }
}

int FX::hop_size() {
    return _hop_size;
}

void FX::hop_size(int new_hop_size) {
    _hop_size = new_hop_size;
}

void FX::process_frame(int input_size, sample* input,
                       int output_size, sample* output) {
    _current_segment = _ns.segment(input_size, input);
}

void FX::process(long input_size, sample* input,
                 long output_size, sample* output) {
    for(long i = 0; i < output_size - _hop_size; i += _hop_size) {
        process_frame(_hop_size, &input[i], _hop_size, &output[i]);
    }
}

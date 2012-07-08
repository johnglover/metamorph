#include "fx.h"

using namespace metamorph;

FX::FX() {
    _hop_size = 512;
    _current_segment = NONE;
    _previous_segment = NONE;
    _fade_in = NULL;
    _fade_out = NULL;

    _frame = new simpl::Frame(_hop_size, true);
    _pd = new simpl::SMSPeakDetection();
    _pt = new simpl::SMSPartialTracking();
    _synth = new simpl::SMSSynthesis();
    _residual = new simpl::SMSResidual();

    _pd->hop_size(_hop_size);
    _synth->hop_size(_hop_size);
    _residual->hop_size(_hop_size);

    recreate_fade_windows();
    reset();
}

FX::~FX() {
    if(_frame) {
        delete _frame;
    }
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

void FX::recreate_fade_windows() {
    if(_fade_in) {
        delete [] _fade_in;
    }

    if(_fade_out) {
        delete [] _fade_out;
    }

    _fade_in = new sample[_hop_size];
    _fade_out = new sample[_hop_size];

    sample step = 1.f / _hop_size;
    for(int i = 0; i < _hop_size; i++) {
        _fade_in[i] = i * step;
        _fade_out[i] = (_hop_size - i) * step;
    }
}

void FX::reset() {
    _ns.reset();
}

int FX::hop_size() {
    return _hop_size;
}

void FX::hop_size(int new_hop_size) {
    _hop_size = new_hop_size;
    if(_frame) {
        delete _frame;
    }
    _frame = new simpl::Frame(_hop_size, true);
    _pd->hop_size(_hop_size);
    _synth->hop_size(_hop_size);
    _residual->hop_size(_hop_size);
    recreate_fade_windows();
}

void FX::process_frame(int input_size, sample* input,
                       int output_size, sample* output) {
    _previous_segment = _current_segment;
    _current_segment = _ns.segment(input_size, input);
    _frame->audio(input);

    if(_current_segment == ONSET) {
        reset();
    }

    if(_current_segment == ONSET || _current_segment == ATTACK) {
        for(int i = 0; i < output_size; i++) {
            output[i] += input[i];
        }
    }
    else {
        _pd->find_peaks_in_frame(_frame);
        _pt->update_partials(_frame);
        _synth->synth_frame(_frame);
        _residual->synth_frame(_frame);

        if(_current_segment == SUSTAIN &&
           (_previous_segment == ONSET || _previous_segment == ATTACK)) {
            // end of transient section, crossfade
            for(int i = 0; i < output_size; i++) {
                output[i] += input[i] * _fade_out[i];
                output[i] += _frame->synth()[i] * _fade_in[i];
                output[i] += _frame->synth_residual()[i] * _fade_in[i];
            }
        }
        else {
            for(int i = 0; i < output_size; i++) {
                output[i] += _frame->synth()[i] + _frame->synth_residual()[i];
            }
        }
    }
}

void FX::process(long input_size, sample* input,
                 long output_size, sample* output) {
    for(long i = 0; i < output_size - _hop_size; i += _hop_size) {
        process_frame(_hop_size, &input[i], _hop_size, &output[i]);
    }
}

#include "fx.h"

using namespace metamorph;

FX::FX() {
    _hop_size = 512;
    _max_partials = 100;
    _current_segment = NONE;
    _previous_segment = NONE;

    _harmonic_scale = 1.f;
    _residual_scale = 1.f;
    _transient_scale = 1.f;

    _fade_in = NULL;
    _fade_out = NULL;

    _frame = new simpl::Frame(_hop_size, true);
    _pd = new simpl::SMSPeakDetection();
    _pd->hop_size(_hop_size);
    ((simpl::SMSPeakDetection*)_pd)->realtime(1);
    _pt = new simpl::SMSPartialTracking();
    _synth = new simpl::SMSSynthesis();
    _synth->hop_size(_hop_size);
    _residual = new simpl::SMSResidual();
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

int FX::max_partials() {
    return _max_partials;
}

void FX::max_partials(int new_max_partials) {
    _max_partials = new_max_partials;
}

sample FX::harmonic_scale() {
    return _harmonic_scale;
}

void FX::harmonic_scale(sample new_harmonic_scale) {
    _harmonic_scale = new_harmonic_scale;
}

sample FX::residual_scale() {
    return _residual_scale;
}

void FX::residual_scale(sample new_residual_scale) {
    _residual_scale = new_residual_scale;
}

sample FX::transient_scale() {
    return _transient_scale;
}

void FX::transient_scale(sample new_transient_scale) {
    _transient_scale = new_transient_scale;
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
    _frame->clear();
    _frame->audio(input);

    if(_current_segment == ONSET) {
        reset();
    }

    if(_current_segment == ONSET || _current_segment == ATTACK) {
        for(int i = 0; i < output_size; i++) {
            output[i] += input[i] * _transient_scale;
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
                output[i] += input[i] * _fade_out[i] * _transient_scale;
                output[i] += _frame->synth()[i] * _fade_in[i] * _harmonic_scale;
                output[i] += _frame->synth_residual()[i] * _fade_in[i] * _residual_scale;
            }
        }
        else {
            for(int i = 0; i < output_size; i++) {
                output[i] += _frame->synth()[i] * _harmonic_scale;
                output[i] += _frame->synth_residual()[i] * _residual_scale;
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

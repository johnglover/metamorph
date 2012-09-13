#include "fx.h"

using namespace metamorph;

FX::FX() {
    _frame_size = 2048;
    _hop_size = 512;
    _max_partials = 500;
    _current_segment = NONE;
    _previous_segment = NONE;

    _harmonic_scale = 1.f;
    _residual_scale = 1.f;
    _transient_scale = 1.f;

    _harmonic_distortion = -1;
    _fundamental_frequency = 0;

    _fade_in = NULL;
    _fade_out = NULL;

    _frame = new simpl::Frame(_frame_size, true);
    _frame->max_partials(_max_partials);
    _residual_frame = new simpl::Frame(_frame_size, true);
    _prev_frame = new simpl::Frame(_frame_size, true);

    _pd = new simpl::LorisPeakDetection();
    _pd->frame_size(_frame_size);
    _pd->hop_size(_hop_size);

    _pt = new simpl::LorisPartialTracking();
    _pt->max_partials(_max_partials);

    _synth = new simpl::SMSSynthesis();
    ((simpl::SMSSynthesis*)_synth)->det_synthesis_type(0);
    _synth->hop_size(_hop_size);
    _synth->max_partials(_max_partials);

    _residual = new simpl::SMSResidual();
    _residual->frame_size(_frame_size);
    _residual->hop_size(_hop_size);

    recreate_fade_windows();
    reset();
}

FX::~FX() {
    if(_frame) delete _frame;
    if(_residual_frame) delete _residual_frame;
    if(_prev_frame) delete _prev_frame;
    if(_pd) delete _pd;
    if(_pt) delete _pt;
    if(_synth) delete _synth;
    if(_residual) delete _residual;
}

void FX::reset() {
    _ns.reset();
}

void FX::recreate_fade_windows() {
    if(_fade_in) delete [] _fade_in;
    if(_fade_out) delete [] _fade_out;

    _fade_in = new sample[_hop_size];
    _fade_out = new sample[_hop_size];

    sample step = 1.f / _hop_size;
    for(int i = 0; i < _hop_size; i++) {
        _fade_in[i] = i * step;
        _fade_out[i] = (_hop_size - i) * step;
    }
}

int FX::frame_size() {
    return _frame_size;
}

void FX::frame_size(int new_frame_size) {
    if(new_frame_size < _hop_size) {
        printf("Error: new frame size was less than current hop size, "
               "setting frame size to equal current hop size.\n");
        _frame_size = _hop_size;
    }
    else {
        _frame_size = new_frame_size;
    }

    if(_frame) delete _frame;
    _frame = new simpl::Frame(_frame_size, true);
    _frame->max_partials(_max_partials);

    if(_residual_frame) delete _residual_frame;
    _residual_frame = new simpl::Frame(_frame_size, true);

    if(_prev_frame) delete _prev_frame;
    _prev_frame = new simpl::Frame(_frame_size, true);

    _pd->frame_size(_frame_size);
    _residual->frame_size(_frame_size);
}

int FX::hop_size() {
    return _hop_size;
}

void FX::hop_size(int new_hop_size) {
    _hop_size = new_hop_size;
    _pd->hop_size(_hop_size);
    _synth->hop_size(_hop_size);
    _residual->hop_size(_hop_size);
    recreate_fade_windows();
}

int FX::max_partials() {
    return _max_partials;
}

void FX::max_partials(int new_max_partials) {
    _max_partials = new_max_partials;
    _pt->max_partials(_max_partials);
    _frame->max_partials(_max_partials);
    _synth->max_partials(_max_partials);
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

sample FX::harmonic_distortion() {
    return  _harmonic_distortion;
}

void FX::harmonic_distortion(sample new_harmonic_distortion) {
    _harmonic_distortion = new_harmonic_distortion;
}

sample FX::fundamental_frequency() {
    return _fundamental_frequency;
}

void FX::fundamental_frequency(sample new_fundamental_frequency) {
    _fundamental_frequency = new_fundamental_frequency;
}

sample FX::f0() {
    if(_fundamental_frequency > 0) {
        return _fundamental_frequency;
    }

    // TODO: estimate fundamental frequency if not set
    return 440.f;
}

void FX::process_frame(int input_size, sample* input,
                       int output_size, sample* output) {
    _previous_segment = _current_segment;
    _current_segment = _ns.segment(input_size, input);
    _frame->clear();
    _residual_frame->clear();

    // get audio input, appending to previous samples if necessary
    if(_hop_size == _frame_size) {
        _frame->audio(input);
        _residual_frame->audio(input);
    }
    else {
        memcpy(_frame->audio(), _prev_frame->audio() + _hop_size,
               sizeof(sample) * (_frame_size - _hop_size));
        memcpy(_frame->audio() + (_frame_size - _hop_size), input,
               sizeof(sample) * _hop_size);

        memcpy(_residual_frame->audio(), _prev_frame->audio() + _hop_size,
               sizeof(sample) * (_frame_size - _hop_size));
        memcpy(_residual_frame->audio() + (_frame_size - _hop_size), input,
               sizeof(sample) * _hop_size);
    }

    // reset any processes that rely on the current note segment
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

        // harmonic distortion
        if(_harmonic_distortion >= 0) {
            sample f = f0();
            for(int i = 0; i < _frame->num_partials(); i++) {
                _frame->partial(i)->frequency =
                    (_harmonic_distortion * _frame->partial(i)->frequency) +
                    ((1 - _harmonic_distortion) * (f * (i + 1)));
            }
        }

        _synth->synth_frame(_frame);

        if(_residual_scale > 0) {
            _residual->synth_frame(_residual_frame);
        }

        if(_current_segment == SUSTAIN &&
           (_previous_segment == ONSET || _previous_segment == ATTACK)) {
            // end of transient section, crossfade
            for(int i = 0; i < output_size; i++) {
                output[i] += input[i] * _fade_out[i] * _transient_scale;
                output[i] += _frame->synth()[i] * _fade_in[i] * _harmonic_scale;
                output[i] += _residual_frame->synth_residual()[i] *
                             _fade_in[i] * _residual_scale;
            }
        }
        else {
            for(int i = 0; i < output_size; i++) {
                output[i] += _frame->synth()[i] * _harmonic_scale;
                output[i] += _residual_frame->synth_residual()[i] *
                             _residual_scale;
            }
        }
    }

    // save samples if frames are larger than hops
    if(_frame_size > _hop_size) {
        memcpy(_prev_frame->audio(), _frame->audio(),
               sizeof(sample) * _frame_size);
    }
}

void FX::process(long input_size, sample* input,
                 long output_size, sample* output) {
    for(long i = 0; i < output_size - _hop_size; i += _hop_size) {
        process_frame(_hop_size, &input[i], _hop_size, &output[i]);
    }
}

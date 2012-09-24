#include "fx.h"

using namespace metamorph;

FX::FX() {
    _frame_size = 2048;
    _hop_size = 512;
    _max_partials = 256;
    _current_segment = NONE;
    _previous_segment = NONE;

    _harmonic_scale = 1.f;
    _residual_scale = 1.f;
    _transient_scale = 1.f;

    _preserve_transients = true;

    _harmonic_distortion = -1.f;
    _fundamental_frequency = 0.f;
    _transposition = 0.f;

    _fade_in = NULL;
    _fade_out = NULL;

    _frame = new simpl::Frame(_frame_size, true);
    _frame->max_peaks(_max_partials);
    _frame->max_partials(_max_partials);
    _residual_frame = new simpl::Frame(_frame_size, true);
    _prev_frame = new simpl::Frame(_frame_size, true);

    _pd = new simpl::LorisPeakDetection();
    _pd->frame_size(_frame_size);
    _pd->hop_size(_hop_size);
    _pd->max_peaks(_max_partials);

    _pt = new simpl::SMSPartialTracking();
    ((simpl::SMSPartialTracking*)_pt)->realtime(true);
    _pt->max_partials(_max_partials);

    _synth = new simpl::SMSSynthesis();
    ((simpl::SMSSynthesis*)_synth)->det_synthesis_type(0);
    _synth->hop_size(_hop_size);
    _synth->max_partials(_max_partials);

    _residual = new simpl::SMSResidual();
    _residual->frame_size(_frame_size);
    _residual->hop_size(_hop_size);

    _create_env = false;
    _apply_env = false;
    _env_interp = 0.f;
    _env_size = 256;
    _env_order = 128;
    _env_stable_partial_duration = 3;
    _env_frame = NULL;
    _env_pt = NULL;
    _spec_env = NULL;

    _transient_substitution = false;
    _new_transient_size = 0;
    _new_transient = NULL;

    reset_fade_windows();
    reset_envelope_data();
    reset();
}

FX::~FX() {
    if(_fade_in) delete [] _fade_in;
    if(_fade_out) delete [] _fade_out;
    if(_frame) delete _frame;
    if(_residual_frame) delete _residual_frame;
    if(_prev_frame) delete _prev_frame;
    if(_pd) delete _pd;
    if(_pt) delete _pt;
    if(_synth) delete _synth;
    if(_residual) delete _residual;
    if(_new_transient) delete [] _new_transient;
    if(_env_frame) delete _env_frame;
    if(_env_pt) delete _env_pt;
    if(_spec_env) delete _spec_env;
}

void FX::reset() {
    _ns.reset();
}

void FX::reset_fade_windows() {
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

void FX::reset_envelope_data() {
    _bin_size = 22050.0 / _env_size;

    _env.resize(_env_size);
    _new_env.resize(_env_size);
    _env_freqs.resize(_env_size);
    _env_mags.resize(_env_size);

    _partial_lifetimes.resize(_max_partials);

    if(_env_frame) delete _env_frame;
    _env_frame = new simpl::Frame(_frame_size, true);
    _env_frame->max_peaks(_max_partials);
    _env_frame->max_partials(_max_partials);

    if(_env_pt) delete _env_pt;
    _env_pt = new simpl::SMSPartialTracking();
    ((simpl::SMSPartialTracking*)_env_pt)->realtime(true);
    ((simpl::SMSPartialTracking*)_env_pt)->harmonic(true);
    _env_pt->max_partials(_max_partials);

    if(_spec_env) delete _spec_env;
    _spec_env = new SpectralEnvelope(_env_order, _env_size);
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
    _frame->max_peaks(_max_partials);
    _frame->max_partials(_max_partials);

    if(_residual_frame) delete _residual_frame;
    _residual_frame = new simpl::Frame(_frame_size, true);

    if(_prev_frame) delete _prev_frame;
    _prev_frame = new simpl::Frame(_frame_size, true);

    _pd->frame_size(_frame_size);
    _residual->frame_size(_frame_size);

    if(_env_frame) delete _env_frame;
    _env_frame = new simpl::Frame(_frame_size, true);
    _env_frame->max_peaks(_max_partials);
    _env_frame->max_partials(_max_partials);
}

int FX::hop_size() {
    return _hop_size;
}

void FX::hop_size(int new_hop_size) {
    _hop_size = new_hop_size;
    _pd->hop_size(_hop_size);
    _synth->hop_size(_hop_size);
    _residual->hop_size(_hop_size);
    reset_fade_windows();
}

int FX::max_partials() {
    return _max_partials;
}

void FX::max_partials(int new_max_partials) {
    _max_partials = new_max_partials;
    _pt->max_partials(_max_partials);
    _frame->max_peaks(_max_partials);
    _frame->max_partials(_max_partials);
    _synth->max_partials(_max_partials);
    reset_envelope_data();
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

// ---------------------------------------------------------------------------
// Transposition
// ---------------------------------------------------------------------------
sample FX::transposition() {
    return _transposition;
}

void FX::transposition(sample new_transposition) {
    _transposition = new_transposition;
}

sample FX::semitones_to_freq(sample semitones) {
    return powf(TWELFTH_ROOT_2, semitones);
}

void FX::transposition(simpl::Frame* frame) {
    if(_transposition != 0) {
        for(int i = 0; i < frame->num_partials(); i++) {
            frame->partial(i)->frequency *= semitones_to_freq(_transposition);
        }
    }
}

// ---------------------------------------------------------------------------
// Spectral Envelope
// ---------------------------------------------------------------------------
bool FX::preserve_envelope() {
    return _create_env && _apply_env;
}

void FX::preserve_envelope(bool preserve) {
    _create_env = preserve;

    if(_env_interp == 0) {
        _apply_env = preserve;
    }
}

sample FX::env_interp() {
    return _env_interp;
}

void FX::env_interp(sample new_env_interp) {
    _env_interp = new_env_interp;
}

void FX::create_envelope(simpl::Frame* frame) {
    if(!_create_env) {
        return;
    }

    // copy peaks to _env_frame so a separate partial tracking
    // phase can be performed. Partial tracking with the SMS harmonic
    // flag set to true results in smoother and more consistent
    // envelopes.
    _env_frame->clear_peaks();

    for(int i = 0; i < frame->num_peaks(); i++) {
        simpl::Peak* p = new simpl::Peak();
        p->amplitude = frame->peak(i)->amplitude;
        p->frequency = frame->peak(i)->frequency;
        p->phase = frame->peak(i)->phase;
        _env_frame->add_peak(p);
    }
    _env_pt->update_partials(_env_frame);

    _env_freqs.clear();
    _env_mags.clear();

    for(int i = 0; i < _env_frame->num_partials(); i++) {
        if(_env_frame->partial(i)->amplitude > 0.f) {
            _env_freqs.push_back(_env_frame->partial(i)->frequency);
            _env_mags.push_back(_env_frame->partial(i)->amplitude);
        }
    }
    _spec_env->envelope(_env_freqs, _env_mags, _env);
}

void FX::apply_envelope(simpl::Frame* frame) {
    if(!_apply_env) {
        return;
    }

    if(_env_interp > 0) {
        sample amp1, amp2;
        for(int i = 0; i < _env.size(); i++) {
            amp1 = _env[i];
            amp2 = _new_env[i];

            if(amp1 <= 0) {
              amp1 = amp2;
            }

            if(amp2 <= 0) {
              amp2 = amp1;
            }

            _env[i] = amp1 + (_env_interp * (amp2 - amp1));
        }
    }

    int bin;
    sample bin_frac;
    sample max_amp = 0;

    // calculate the maximum partial amplitude in the current frame,
    // and how long each partial has been active for
    for(int i = 0; i < frame->num_partials(); i++) {
        if(frame->partial(i)->amplitude > max_amp) {
            max_amp = frame->partial(i)->amplitude;
        }

        if(frame->partial(i)->amplitude > 0.f) {
            _partial_lifetimes[i]++;
        }
        else {
            _partial_lifetimes[i] = 0;
        }
    }
    for(int i = frame->num_partials(); i < _max_partials; i++) {
        _partial_lifetimes[i] = 0;
    }

    // set the amplitude of large amplitude partials and long-lived
    // partials to match the spectral envelope value
    for(int i = 0; i < frame->num_partials(); i++) {
        if((frame->partial(i)->amplitude <= (max_amp * 0.1)) ||
           (_partial_lifetimes[i] < _env_stable_partial_duration)) {
            continue;
        }

        bin = (int)(frame->partial(i)->frequency / _bin_size);
        bin_frac = (frame->partial(i)->frequency / (sample)_bin_size) - (sample)bin;

        if(bin < _env.size() - 1) {
            frame->partial(i)->amplitude =
                ((1.0 - bin_frac) * _env[bin]) + (bin_frac * _env[bin + 1]);
        }
        else if(bin == _env.size() - 1){
            frame->partial(i)->amplitude = _env[bin];
        }
        else {
            frame->partial(i)->amplitude = 0.f;
        }
    }
}

void FX::apply_envelope(int env_size, sample* env) {
    if(env_size != _env_size) {
        printf("Error: could not apply envelope as the new envelope "
               "size (%d) does not match the current envelope "
               "size (%d).\n", env_size, _env_size);
        return;
    }

    _apply_env = true;
    // memcpy(_new_env, env, sizeof(sample) * env_size);
}

void FX::clear_envelope() {
    _apply_env = false;
    // memset(_new_env, 0, sizeof(sample) * _max_partials);
}

// ---------------------------------------------------------------------------
// Harmonic Distortion
// ---------------------------------------------------------------------------
sample FX::harmonic_distortion() {
    return  _harmonic_distortion;
}

void FX::harmonic_distortion(sample new_harmonic_distortion) {
    _harmonic_distortion = new_harmonic_distortion;
}

void FX::harmonic_distortion(simpl::Frame* frame) {
    if(_harmonic_distortion < 0) {
        return;
    }

    sample f = f0();
    for(int i = 0; i < frame->num_partials(); i++) {
        frame->partial(i)->frequency =
            (_harmonic_distortion * frame->partial(i)->frequency) +
            ((1 - _harmonic_distortion) * (f * (i + 1)));
    }
}

// ---------------------------------------------------------------------------
// Transient Processing
// ---------------------------------------------------------------------------
bool FX::preserve_transients() {
    return _preserve_transients;
}

void FX::preserve_transients(bool preserve) {
    _preserve_transients = preserve;
}

bool FX::transient_substitution() {
    return _transient_substitution;
}

void FX::transient_substitution(bool substitute) {
    _transient_substitution = substitute;
}

void FX::new_transient(int new_transient_size, sample* new_transient) {
    if(new_transient_size <= 0) {
        return;
    }

    _new_transient = new sample[new_transient_size];
    memcpy(_new_transient, new_transient, sizeof(sample) * new_transient_size);
}

void FX::clear_new_transient() {
    if(_new_transient) delete [] _new_transient;
    _new_transient = NULL;
    _new_transient_size = 0;
}

// ---------------------------------------------------------------------------
// Process Frame
// ---------------------------------------------------------------------------
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

    // find sinusoidal peaks and partials
    _pd->find_peaks_in_frame(_frame);
    _pt->update_partials(_frame);

    // don't use synthesis output for transient region if
    // _preserve_transients is set to true
    if(_preserve_transients && (_current_segment == ONSET ||
                                _current_segment == ATTACK))  {
        for(int i = 0; i < output_size; i++) {
            output[i] += input[i] * _transient_scale;
        }
    }
    else {
        create_envelope(_frame);
        transposition(_frame);
        harmonic_distortion(_frame);
        apply_envelope(_frame);

        _synth->synth_frame(_frame);

        if(_residual_scale > 0) {
            _residual->synth_frame(_residual_frame);
        }

        if(_preserve_transients && _current_segment == SUSTAIN &&
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

// ---------------------------------------------------------------------------
// Process
// ---------------------------------------------------------------------------
void FX::process(long input_size, sample* input,
                 long output_size, sample* output) {
    for(long i = 0; i < output_size - _hop_size; i += _hop_size) {
        process_frame(_hop_size, &input[i], _hop_size, &output[i]);
    }
}

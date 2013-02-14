#include "fx.h"

using namespace metamorph;

FX::FX() {
    _frame_size = 2048;
    _hop_size = 512;
    _max_partials = 256;
    _current_segment = notesegmentation::NONE;
    _previous_segment = notesegmentation::NONE;

    _harmonic_scale = 1.f;
    _residual_scale = 1.f;
    _transient_scale = 1.f;

    _preserve_transients = true;

    _input.resize(_hop_size);

    _fade_duration = _hop_size;
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
    _transient_sample = 0;
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

    _harm_trans.clear();
    _specenv_trans.clear();
    _residual_trans.clear();
    _transient_trans.clear();
    _input.clear();
    _env.clear();
    _new_env.clear();
    _env_freqs.clear();
    _env_mags.clear();
    _partial_lifetimes.clear();
}

void FX::reset() {
    _ns.reset();
    _pt->reset();
    _transient_sample = 0;
}

void FX::reset_fade_windows() {
    if(_fade_in) delete [] _fade_in;
    if(_fade_out) delete [] _fade_out;

    _fade_in = new sample[_fade_duration];
    _fade_out = new sample[_fade_duration];

    sample step = 1.f / _fade_duration;
    for(int i = 0; i < _fade_duration; i++) {
        sample t = (float)i / _fade_duration;
        _fade_in[i] = pow(0.5 - (0.5 * cos(M_PI * t)), 0.5);
        _fade_out[i] = pow(0.5 + (0.5 * cos(M_PI * t)), 0.5);
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

void FX::setup_frame(int input_size, int output_size) {
    if(input_size < _hop_size ||
       output_size < _hop_size) {
        throw Exception(std::string("Audio frame size less than ") +
                        std::string("FX hop size"));
    }
    else if((input_size % _hop_size != 0) ||
            (output_size % _hop_size != 0)) {
        throw Exception(std::string("Audio frame size not a multiple ") +
                        std::string("of FX hop size"));
    }

    _frame->clear();
    _residual_frame->clear();

    if(_hop_size == _frame_size) {
        _frame->audio(&(_input[0]));
        _residual_frame->audio(&(_input[0]));
    }
    else {
        memcpy(_frame->audio(),
               _prev_frame->audio() + _hop_size,
               sizeof(sample) * (_frame_size - _hop_size));
        memcpy(_frame->audio() + (_frame_size - _hop_size),
               &(_input[0]),
               sizeof(sample) * _hop_size);

        memcpy(_residual_frame->audio(),
               _prev_frame->audio() + _hop_size,
               sizeof(sample) * (_frame_size - _hop_size));
        memcpy(_residual_frame->audio() + (_frame_size - _hop_size),
               &(_input[0]),
               sizeof(sample) * _hop_size);
    }
}

void FX::cleanup_frame() {
    // save samples if frames are larger than hop size
    if(_frame_size > _hop_size) {
        memcpy(_prev_frame->audio(), _frame->audio(),
               sizeof(sample) * _frame_size);
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
    _frame->synth_size(_hop_size);
    _frame->max_peaks(_max_partials);
    _frame->max_partials(_max_partials);

    if(_residual_frame) delete _residual_frame;
    _residual_frame = new simpl::Frame(_frame_size, true);
    _residual_frame->synth_size(_hop_size);

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
    _frame->synth_size(_hop_size);
    _residual->hop_size(_hop_size);
    _residual_frame->synth_size(_hop_size);
    _input.resize(_hop_size);
    _ns.frame_size(_hop_size);
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

void FX::add_transformation(HarmonicTransformation* trans) {
    _harm_trans.push_back(trans);
}

void FX::clear_harmonic_transformations() {
    _harm_trans.clear();
}

void FX::add_transformation(SpecEnvTransformation* trans) {
    _specenv_trans.push_back(trans);
}

void FX::clear_specenv_transformations() {
    _specenv_trans.clear();
}

void FX::add_transformation(ResidualTransformation* trans) {
    _residual_trans.push_back(trans);
}

void FX::clear_residual_transformations() {
    _residual_trans.clear();
}

void FX::add_transformation(TransientTransformation* trans) {
    _transient_trans.push_back(trans);
}

void FX::clear_transient_transformations() {
    _transient_trans.clear();
}

void FX::clear_transformations() {
    clear_harmonic_transformations();
    clear_specenv_transformations();
    clear_residual_transformations();
    clear_transient_transformations();
}

// ---------------------------------------------------------------------------
// Harmonic Transformations
// ---------------------------------------------------------------------------
sample FX::harmonic_scale() {
    return _harmonic_scale;
}

void FX::harmonic_scale(sample new_harmonic_scale) {
    _harmonic_scale = new_harmonic_scale;
}


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

int FX::env_size() {
    return _env_size;
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

    if(_env.size() != _new_env.size()) {
        printf("Error: could not apply envelope as the new envelope "
               "size (%d) does not match the current envelope "
               "size (%d).\n", (int)_env.size(), (int)_new_env.size());
        return;
    }

    if(_env_interp > 0) {
        for(int i = 0; i < _env.size(); i++) {
            _env[i] = ((1.0 - _env_interp) * _env[i]) +
                      (_env_interp * _new_env[i]);
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
        bin_frac = (frame->partial(i)->frequency / (sample)_bin_size) -
                   (sample)bin;

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
    _new_env.assign(env, env + env_size);
}

void FX::apply_envelope(std::vector<sample>& env) {
    if(env.size() != _env_size) {
        printf("Error: could not apply envelope as the new envelope "
               "size (%d) does not match the current envelope "
               "size (%d).\n", (int)env.size(), _env_size);
        return;
    }

    _apply_env = true;

    _new_env.assign(env.begin(), env.end());
}

bool FX::apply_envelope() {
    return _apply_env;
}

void FX::apply_envelope(bool new_apply_env) {
    _apply_env = new_apply_env;
}

void FX::clear_envelope() {
    _apply_env = false;
    _new_env.assign(_env_size, 0.f);
}

// ---------------------------------------------------------------------------
// Residual Transformations
// ---------------------------------------------------------------------------
sample FX::residual_scale() {
    return _residual_scale;
}

void FX::residual_scale(sample new_residual_scale) {
    _residual_scale = new_residual_scale;
}

// ---------------------------------------------------------------------------
// Transient Processing and Transformations
// ---------------------------------------------------------------------------
sample FX::transient_scale() {
    return _transient_scale;
}

void FX::transient_scale(sample new_transient_scale) {
    _transient_scale = new_transient_scale;
}

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
// Processing
// ---------------------------------------------------------------------------
void FX::process_frame(int input_size, sample* input,
                       int output_size, sample* output) {
    // setup for current frame
    _input.assign(input, input + _hop_size);
    setup_frame(input_size, output_size);

    // calculate current temporal region
    _previous_segment = _current_segment;
    _current_segment = _ns.segment(input_size, input);

    // if at onset, reset any processes that rely on the current note segment
    if(_current_segment == notesegmentation::ONSET) {
        reset();
    }

    // find sinusoidal peaks and partials
    _pd->find_peaks_in_frame(_frame);
    _pt->update_partials(_frame);

    // don't use synthesis output for transient region if
    // _preserve_transients is set to true
    if(_preserve_transients && (_transient_scale > 0) &&
       (_current_segment == notesegmentation::ONSET ||
        _current_segment == notesegmentation::ATTACK)) {
        // perform all transient transformations
        for(int i = 0; i < _transient_trans.size(); i++) {
            _transient_trans[i]->process_frame(_input);
        }

        if(_transient_substitution) {
            for(int i = 0; i < _hop_size; i++) {
                if(_transient_sample < _new_transient_size) {
                    _input[i] = _new_transient[i];
                    _transient_sample++;
                }
                else {
                    break;
                }
            }
        }

        for(int i = 0; i < _hop_size; i++) {
            output[i] += _input[i] * _transient_scale;
        }
    }
    else {
        // perform all harmonic transformations
        if(_harmonic_scale > 0) {
            create_envelope(_frame);

            for(int i = 0; i < _harm_trans.size(); i++) {
                _harm_trans[i]->process_frame(_frame);
            }

            for(int i = 0; i < _specenv_trans.size(); i++) {
                _specenv_trans[i]->process_frame(_frame, _new_env);
            }

            apply_envelope(_frame);
            _synth->synth_frame(_frame);
        }

        // perform all residual transformations
        if(_residual_scale > 0) {
            for(int i = 0; i < _residual_trans.size(); i++) {
                _residual_trans[i]->process_frame(_residual_frame);
            }

            _residual->synth_frame(_residual_frame);
        }

        if(_preserve_transients &&
           _current_segment == notesegmentation::SUSTAIN &&
           (_previous_segment == notesegmentation::ONSET ||
            _previous_segment == notesegmentation::ATTACK)) {

            // perform all transient transformations
            for(int i = 0; i < _transient_trans.size(); i++) {
                _transient_trans[i]->process_frame(_input);
            }

            if(_transient_substitution) {
                for(int i = 0; i < _hop_size; i++) {
                    if(_transient_sample < _new_transient_size) {
                        _input[i] = _new_transient[i];
                        _transient_sample++;
                    }
                    else {
                        break;
                    }
                }
            }

            // end of transient section, crossfade
            for(int i = 0; i < _fade_duration; i++) {
                output[i] += _input[i] * _fade_out[i] * _transient_scale;
                output[i] += _frame->synth()[i] * _fade_in[i] *
                             _harmonic_scale;
                output[i] += _residual_frame->synth_residual()[i] *
                             _fade_in[i] * _residual_scale;
            }

            for(int i = _fade_duration; i < _hop_size; i++) {
                output[i] += _frame->synth()[i] * _harmonic_scale;
                output[i] += _residual_frame->synth_residual()[i] *
                             _residual_scale;
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

    cleanup_frame();
}

void FX::process(long input_size, sample* input,
                 long output_size, sample* output) {
    for(long i = 0; i < output_size - _hop_size; i += _hop_size) {
        process_frame(_hop_size, &input[i], _hop_size, &output[i]);
    }
}

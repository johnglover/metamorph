#include "transformations.h"

using namespace metamorph;


// ---------------------------------------------------------------------------
// Transposition
// ---------------------------------------------------------------------------
Transposition::Transposition() {
    _transposition = 0;
    _transposition_hz = 0;
}

Transposition::Transposition(sample new_transposition) {
    _transposition = new_transposition;
    _transposition_hz = semitones_to_freq(_transposition);
}

sample Transposition::transposition() {
    return _transposition;
}

void Transposition::transposition(sample new_transposition) {
    _transposition = new_transposition;
    _transposition_hz = semitones_to_freq(_transposition);
}

sample Transposition::semitones_to_freq(sample semitones) {
    return pow(TWELFTH_ROOT_2, semitones);
}

void Transposition::process_frame(simpl::Frame* frame) {
    if(_transposition != 0) {
        for(int i = 0; i < frame->num_partials(); i++) {
            frame->partial(i)->frequency *= _transposition_hz;
        }
    }
}


// ---------------------------------------------------------------------------
// HarmonicDistortion
// ---------------------------------------------------------------------------
HarmonicDistortion::HarmonicDistortion() {
    _harmonic_distortion = 1.0;
    _fundamental_frequency = 0.0;
    _num_harmonics = 0;
}

HarmonicDistortion::HarmonicDistortion(sample new_harmonic_distortion,
                                       sample new_fundamental) {
    _harmonic_distortion = new_harmonic_distortion;
    _fundamental_frequency = new_fundamental;
    _num_harmonics = 22050 / _fundamental_frequency;
}

sample HarmonicDistortion::harmonic_distortion() {
    return _harmonic_distortion;
}

void HarmonicDistortion::harmonic_distortion(sample new_harmonic_distortion) {
    _harmonic_distortion = new_harmonic_distortion;
}

sample HarmonicDistortion::fundamental_frequency() {
    return _fundamental_frequency;
}

void HarmonicDistortion::fundamental_frequency(sample new_fundamental) {
    _fundamental_frequency = new_fundamental;
    _num_harmonics = 22050 / _fundamental_frequency;
}

void HarmonicDistortion::process_frame(simpl::Frame* frame) {
    int harm = 0;
    sample f = 0;

    for(int i = 0; i < frame->num_partials(); i++) {
        if(frame->partial(i)->amplitude > 0) {
            f = frame->partial(i)->frequency;
            harm = (f / _num_harmonics) + 1;

            frame->partial(i)->frequency = (_harmonic_distortion * f) +
                ((1 - _harmonic_distortion) * (_fundamental_frequency * harm));
        }
    }
}


// ---------------------------------------------------------------------------
// TransientLPF
//
// Filter transients using a Butterworth low-pass filter (2nd order).
// Based on Victor Lazzarini's implementation from The SndObj library.
// ---------------------------------------------------------------------------
TransientLPF::TransientLPF() {
    _frequency = 0.0;
    _sampling_rate = 44100;
    _delay = NULL;
    reset();
}

TransientLPF::TransientLPF(sample frequency) {
    _frequency = frequency;
    _sampling_rate = 44100;
    _delay = NULL;
    reset();
}

TransientLPF::~TransientLPF() {
    if(_delay) delete [] _delay;
    _delay = NULL;
}

void TransientLPF::reset() {
    if(_delay) delete [] _delay;
    _delay = new sample[2];
    _delay[0] = _delay[1] = 0.0;

    _c = 1.0 / (tan(M_PI * _frequency / _sampling_rate));
    _a = 1.0 / (1.0 + ROOT_2 * _c + (_c * _c));
    _a1 = 2.0 * _a;
    _a2 = _a;
    _b1 = 2.0 * (1.0 - (_c * _c)) *_a;
    _b2 = (1.0 - ROOT_2 * _c + (_c * _c)) * _a;
}

void TransientLPF::process_frame(std::vector<sample>& samples) {
    sample t = 0.0;

    for(int n = 0; n < samples.size(); n++) {
        t = samples[n] - _b1 * _delay[0] - _b2 * _delay[1];
        samples[n] = t * _a + _a1 * _delay[0] + _a2 * _delay[1];

        _delay[1] = _delay[0];
        _delay[0] = t;
    }
}


// ---------------------------------------------------------------------------
// TransientHPF
//
// Filter transients using a Butterworth high-pass filter (2nd order).
// Based on Victor Lazzarini's implementation from The SndObj library.
// ---------------------------------------------------------------------------
TransientHPF::TransientHPF() {
    _frequency = 0.0;
    _sampling_rate = 44100;
    _delay = NULL;
    reset();
}

TransientHPF::TransientHPF(sample frequency) {
    _frequency = frequency;
    _sampling_rate = 44100;
    _delay = NULL;
    reset();
}

TransientHPF::~TransientHPF() {
    if(_delay) delete [] _delay;
    _delay = NULL;
}

void TransientHPF::reset() {
    if(_delay) delete [] _delay;
    _delay = new sample[2];
    _delay[0] = _delay[1] = 0.0;

    _c = tan(M_PI * _frequency / _sampling_rate);
    _a = 1.0 / (1 + ROOT_2 * _c + (_c * _c));
    _a1 = -2.0 * _a;
    _a2 = _a;
    _b1 = 2.0 * ((_c * _c) - 1.0) * _a;
    _b2 = (1.0 - ROOT_2 * _c + (_c * _c)) * _a;
}

void TransientHPF::process_frame(std::vector<sample>& samples) {
    sample t = 0.0;

    for(int n = 0; n < samples.size(); n++) {
        t = samples[n] - _b1 * _delay[0] - _b2 * _delay[1];
        samples[n] = t * _a + _a1 * _delay[0] + _a2 * _delay[1];

        _delay[1] = _delay[0];
        _delay[0] = t;
    }
}

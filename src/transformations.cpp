#include "transformations.h"

using namespace metamorph;


// ---------------------------------------------------------------------------
// Transposition
// ---------------------------------------------------------------------------
Transposition::Transposition() {
    _transposition = 0;
}

Transposition::Transposition(sample new_transposition) {
    _transposition = new_transposition;
}

sample Transposition::transposition() {
    return _transposition;
}

void Transposition::transposition(sample new_transposition) {
    _transposition = new_transposition;
}

sample Transposition::semitones_to_freq(sample semitones) {
    return powf(TWELFTH_ROOT_2, semitones);
}

void Transposition::process_frame(simpl::Frame* frame) {
    if(_transposition != 0) {
        for(int i = 0; i < frame->num_partials(); i++) {
            frame->partial(i)->frequency *= semitones_to_freq(_transposition);
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

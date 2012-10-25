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

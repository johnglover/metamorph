#include "time_scale.h"

using namespace metamorph;

TimeScale::TimeScale() {
    _scale_factor = 1.0;
}

TimeScale::~TimeScale() {
}

sample TimeScale::scale_factor() {
    return _scale_factor;
}

void TimeScale::scale_factor(sample new_scale_factor) {
    _scale_factor = new_scale_factor;
}

void TimeScale::process(long input_size, sample* input,
                        long output_size, sample* output) {

    // pass 1: get analysis frames and calculate locations of transient regions
    reset();
    _segments.clear();
    _segments.resize(input_size / _hop_size);

    simpl::Frames peaks = _pd->find_peaks(input_size, input);
    simpl::Frames frames = _pt->find_partials(peaks);

    long transient_length = 0;

    for(long i = 0; i < input_size - _hop_size; i += _hop_size) {
        _previous_segment = _current_segment;
        _current_segment = _ns.segment(_hop_size, &input[i]);
        _segments[i / _hop_size] = _current_segment;

        if(_current_segment == notesegmentation::NONE ||
           _current_segment == notesegmentation::ONSET ||
           _current_segment == notesegmentation::ATTACK) {
            transient_length += _hop_size;
        }
    }

    // pass 2: time scale the signal
    //
    // as no time scaling ocurs during transient regions,
    // adjust scaling factor for remaining signal regions
    // so as the total time scale factor is achieved
    long target_output_size = (long)(input_size * _scale_factor);
    long scaled_samples = target_output_size - transient_length;
    sample step_size = (sample)(input_size - transient_length) /
                               scaled_samples;
    sample current_frame = 0;
    long n = 0;
    int f = 0;

    while((current_frame < frames.size() &&
          (n < (output_size - _hop_size)))) {
        f = (int)floor(current_frame);

        _residual_frame->clear();
        _residual_frame->audio(frames[f]->audio());

        _synth->synth_frame(frames[f]);
        _residual->synth_frame(_residual_frame);

        if(_segments[f] == notesegmentation::NONE ||
           _segments[f] == notesegmentation::ONSET ||
           _segments[f] == notesegmentation::ATTACK) {
            for(int i = 0; i < _hop_size; i++) {
                output[n] = frames[f]->audio()[i] * _transient_scale;
                n++;
            }
            current_frame += 1;
        }
        else if(_segments[f] == notesegmentation::SUSTAIN &&
                (_segments[f - 1] == notesegmentation::ONSET ||
                 _segments[f - 1] == notesegmentation::ATTACK)) {
            for(int i = 0; i < _fade_duration; i++) {
                output[n] = frames[f]->audio()[i] * _fade_out[i] *
                            _transient_scale;
                output[n] += frames[f]->synth()[i] * _fade_in[i] *
                            _harmonic_scale;
                output[n] += _residual_frame->synth_residual()[i] *
                             _fade_in[i] * _residual_scale;
                n++;
            }
            for(int i = _fade_duration; i < _hop_size; i++) {
                output[n] += frames[f]->synth()[i] * _harmonic_scale;
                output[n] += _residual_frame->synth_residual()[i] *
                             _fade_in[i] * _residual_scale;
                n++;
            }
            current_frame += 1;
        }
        else {
            for(int i = 0; i < _hop_size; i++) {
                output[n] = frames[f]->synth()[i] * _harmonic_scale;
                output[n] += _residual_frame->synth_residual()[i] *
                             _residual_scale;
                n++;
            }
            current_frame += step_size;
        }
    }
}

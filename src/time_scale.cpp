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

        if(_current_segment == NONE || _current_segment == ONSET ||
           _current_segment == ATTACK) {
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
    long output_sample = 0;

    while((current_frame < frames.size() &&
          (output_sample < (output_size - _hop_size)))) {
        int n = (int)floor(current_frame);

        _synth->synth_frame(frames[n]);

        if(_segments[n] == NONE || _segments[n] == ONSET ||
           _segments[n] == ATTACK) {
            for(int i = 0; i < _hop_size; i++) {
                output[output_sample] = frames[n]->audio()[i];
                output_sample++;
            }
            current_frame += 1;
        }
        else if(_segments[n] == SUSTAIN &&
                (_segments[n - 1] == ONSET || _segments[n - 1] == ATTACK)) {
            for(int i = 0; i < _fade_duration; i++) {
                output[output_sample] = frames[n]->audio()[i] * _fade_out[i];
                output[output_sample] += frames[n]->synth()[i] * _fade_in[i];
                output_sample++;
            }
            for(int i = _fade_duration; i < _hop_size; i++) {
                output[output_sample] += frames[n]->synth()[i];
                output_sample++;
            }
            current_frame += 1;
        }
        else {
            for(int i = 0; i < _hop_size; i++) {
                output[output_sample] = frames[n]->synth()[i];
                output_sample++;
            }
            current_frame += step_size;
        }
    }
}

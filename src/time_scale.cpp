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

bool TimeScale::is_transient_region(long sample_number) {
    for(int i = 0; i < _transients.size(); i++) {
        if(sample_number >= _transients[i].start &&
           sample_number <= _transients[i].end) {
            return true;
        }
    }
    return false;
}

void TimeScale::process(long input_size, sample* input,
                        long output_size, sample* output) {

    // pass 1: get analysis frames and calculate locations of transient regions
    reset();
    _transients.clear();

    simpl::Frames peaks = _pd->find_peaks(input_size, input);
    simpl::Frames frames = _pt->find_partials(peaks);

    long transient_length = 0;

    for(long i = 0; i < input_size - _hop_size; i += _hop_size) {
        _previous_segment = _current_segment;
        _current_segment = _ns.segment(_hop_size, &input[i]);

        if(_current_segment == ONSET) {
            TransientRegion t;
            t.start = i;
            _transients.push_back(t);
            transient_length += _hop_size;
        }
        else if(_current_segment == ATTACK) {
            transient_length += _hop_size;
        }
        else if(_current_segment == SUSTAIN &&
                (_previous_segment == ONSET || _previous_segment == ATTACK)) {
            _transients.back().end = i;
        }
    }

    // allocate memory for synthesised harmonic and stochastic components
    for(int i = 0; i < frames.size(); i++) {
        sample* synth_audio = new sample[_hop_size];
        sample* residual = new sample[_hop_size];
        sample* synth_residual = new sample[_hop_size];
        memset(synth_audio, 0.0, sizeof(sample) * _hop_size);
        memset(residual, 0.0, sizeof(sample) * _hop_size);
        memset(synth_residual, 0.0, sizeof(sample) * _hop_size);
        frames[i]->synth(synth_audio);
        frames[i]->residual(residual);
        frames[i]->synth_residual(synth_residual);
    }

    // pass 2: time scale the signal
    //
    // as no time scaling ocurs during transient regions,
    // adjust scaling factor for remaining signal regions
    // so as the total time scale factor is achieved
    long target_output_size = (long)(input_size * _scale_factor);
    long scaled_samples = target_output_size - transient_length;
    sample step_size = (sample)(input_size - transient_length) / scaled_samples;

    sample current_frame = 0;
    long output_sample = 0;

    while((current_frame < frames.size() && 
          (output_sample < (output_size - _hop_size)))) {
        int n = (int)floor(current_frame);

        _synth->synth_frame(frames[n]);

        for(int i = 0; i < _hop_size; i++) {
            output[output_sample] += frames[n]->synth()[i];
            output_sample++;
        }

        if(!is_transient_region(n * _hop_size)) {
            current_frame += step_size;
        }
        else {
            current_frame += 1;
        }
    }

    for(int i = 0; i < frames.size(); i++) {
        delete [] frames[i]->synth();
        delete [] frames[i]->residual();
        delete [] frames[i]->synth_residual();
    }
}

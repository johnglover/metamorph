#ifndef METAMORPH_TRANSFORMATIONS_H
#define METAMORPH_TRANSFORMATIONS_H


#include "simpl/simpl.h"

#ifndef TWELFTH_ROOT_2
#define TWELFTH_ROOT_2 1.0594630943592953
#endif

namespace metamorph
{


typedef double sample;

class HarmonicTransformation {
    public:
        virtual void process_frame(simpl::Frame* frame) = 0;
};


class Transposition : public HarmonicTransformation {
    private:
        sample _transposition;
        sample semitones_to_freq(sample semitones);

    public:
        Transposition();
        Transposition(sample new_transposition);
        sample transposition();
        void transposition(sample new_transposition);
        void process_frame(simpl::Frame* frame);
};


class HarmonicDistortion : public HarmonicTransformation {
    private:
        sample _harmonic_distortion;
        sample _fundamental_frequency;
        int _num_harmonics;

    public:
        HarmonicDistortion();
        HarmonicDistortion(sample new_harmonic_distortion,
                           sample new_fundamental);
        sample harmonic_distortion();
        void harmonic_distortion(sample new_harmonic_distortion);
        sample fundamental_frequency();
        void fundamental_frequency(sample new_fundamental);
        void process_frame(simpl::Frame* frame);
};

} // end of namespace metamorph


#endif

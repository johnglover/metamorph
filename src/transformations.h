#ifndef METAMORPH_TRANSFORMATIONS_H
#define METAMORPH_TRANSFORMATIONS_H


#include <math.h>
#include "simpl/simpl.h"

#ifndef TWELFTH_ROOT_2
#define TWELFTH_ROOT_2 1.0594630943592953
#endif

#ifndef ROOT_2
#define ROOT_2 1.4142135623730950488
#endif

namespace metamorph
{


typedef double sample;


class Transformation {
    public:
        virtual void process_frame(simpl::Frame* frame) = 0;
};


class HarmonicTransformation : public Transformation {};


class ResidualTransformation : public Transformation {};


class SpecEnvTransformation {
    public:
        virtual void process_frame(simpl::Frame* frame,
                                   std::vector<sample>& env) = 0;
};


class TransientTransformation {
    public:
        virtual void process_frame(std::vector<sample>& samples) = 0;
};


class Transposition : public HarmonicTransformation {
    private:
        sample _transposition;
        sample _transposition_hz;
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


class TransientLPF : public TransientTransformation {
    private:
        sample _frequency;
        int _sampling_rate;
        sample* _delay;
        sample _a;
        sample _a1;
        sample _a2;
        sample _b1;
        sample _b2;
        sample _c;
        void reset();

    public:
        TransientLPF();
        TransientLPF(sample frequency);
        ~TransientLPF();
        void process_frame(std::vector<sample>& samples);
};


class TransientHPF : public TransientTransformation {
    private:
        sample _frequency;
        int _sampling_rate;
        sample* _delay;
        sample _a;
        sample _a1;
        sample _a2;
        sample _b1;
        sample _b2;
        sample _c;
        void reset();

    public:
        TransientHPF();
        TransientHPF(sample frequency);
        ~TransientHPF();
        void process_frame(std::vector<sample>& samples);
};


} // end of namespace metamorph


#endif

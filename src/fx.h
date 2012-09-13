#ifndef METAMORPH_FX_H
#define METAMORPH_FX_H

#include "notesegmentation/segmentation.h"
#include "simpl/simpl.h"

using namespace std;


namespace metamorph
{

typedef double sample;

class FX {
    protected:
        int _frame_size;
        int _hop_size;
        int _max_partials;
        int _current_segment;
        int _previous_segment;

        sample _harmonic_scale;
        sample _residual_scale;
        sample _transient_scale;

        sample _harmonic_distortion;
        sample _fundamental_frequency;

        sample* _fade_in;
        sample* _fade_out;

        GLT _ns;

        simpl::Frame* _frame;
        simpl::Frame* _residual_frame;
        simpl::Frame* _prev_frame;
        simpl::PeakDetection* _pd;
        simpl::PartialTracking* _pt;
        simpl::Synthesis* _synth;
        simpl::Residual* _residual;

        void recreate_fade_windows();
        virtual sample f0();

    public:
        FX();
        ~FX();

        void reset();

        int frame_size();
        virtual void frame_size(int new_frame_size);
        int hop_size();
        virtual void hop_size(int new_hop_size);
        int max_partials();
        virtual void max_partials(int new_max_partials);

        sample harmonic_scale();
        virtual void harmonic_scale(sample new_harmonic_scale);
        sample residual_scale();
        virtual void residual_scale(sample new_residual_scale);
        sample transient_scale();
        virtual void transient_scale(sample new_transient_scale);

        sample harmonic_distortion();
        void harmonic_distortion(sample new_harmonic_distortion);
        sample fundamental_frequency();
        virtual void fundamental_frequency(sample new_fundamental_frequency);

        virtual void process_frame(int input_size, sample* input,
                                   int output_size, sample* output);
        virtual void process(long input_size, sample* input,
                             long output_size, sample* output);
};


} // end of namespace metamorph

#endif

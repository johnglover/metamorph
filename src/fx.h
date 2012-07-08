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
        int _hop_size;
        int _current_segment;
        int _previous_segment;
        sample* _fade_in;
        sample* _fade_out;
        GLT _ns;
        simpl::Frame* _frame;
        simpl::PeakDetection* _pd;
        simpl::PartialTracking* _pt;
        simpl::Synthesis* _synth;
        simpl::Residual* _residual;

        void recreate_fade_windows();

    public:
        FX();
        ~FX();

        void reset();
        int hop_size();
        virtual void hop_size(int new_hop_size);
        virtual void process_frame(int input_size, sample* input,
                                   int output_size, sample* output);
        virtual void process(long input_size, sample* input,
                             long output_size, sample* output);
};


} // end of namespace metamorph

#endif

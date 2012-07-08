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
        GLT _ns;
        simpl::PeakDetection* _pd;
        simpl::PartialTracking* _pt;
        simpl::Synthesis* _synth;
        simpl::Residual* _residual;

    public:
        FX();
        ~FX();

        int hop_size();
        virtual void hop_size(int new_hop_size);
        virtual void process_frame(int input_size, sample* input,
                                   int output_size, sample* output);
        virtual void process(long input_size, sample* input,
                             long output_size, sample* output);
};


} // end of namespace metamorph

#endif

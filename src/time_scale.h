#ifndef METAMORPH_TIME_SCALE_H
#define METAMORPH_TIME_SCALE_H

#include <vector>
#include "fx.h"

using namespace std;


namespace metamorph
{


class TimeScale : public FX {
    private:
        sample _scale_factor;
        std::vector<int> _segments;

    public:
        TimeScale();
        ~TimeScale();

        sample scale_factor();
        void scale_factor(sample new_scale_factor);

        void process(long input_size, sample* input,
                     long output_size, sample* output);
};


} // end of namespace metamorph

#endif

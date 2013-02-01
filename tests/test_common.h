#ifndef METAMORPH_TEST_COMMON_H
#define METAMORPH_TEST_COMMON_H

#include <iostream>
#include <vector>
#include <sndfile.hh>

#include "exceptions.h"

namespace metamorph
{


typedef double sample;
static const double PRECISION = 0.001;
static const char* TEST_AUDIO_FILE = "../tests/audio/flute.wav";


} // end of namespace metamorph

#endif

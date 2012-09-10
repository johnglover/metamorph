#include <iostream>
#include <cppunit/ui/text/TextTestRunner.h>
#include <cppunit/TestResult.h>
#include <cppunit/TestResultCollector.h>
#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/BriefTestProgressListener.h>
#include <cppunit/extensions/TestFactoryRegistry.h>
#include <sndfile.hh>

#include "../src/metamorph.h"

namespace metamorph
{

// ---------------------------------------------------------------------------
//	TestFX
// ---------------------------------------------------------------------------
class TestFX : public CPPUNIT_NS::TestCase {
    CPPUNIT_TEST_SUITE(TestFX);
    CPPUNIT_TEST(test_basic);
    CPPUNIT_TEST_SUITE_END();

protected:
    static const double PRECISION = 0.001;
    SndfileHandle sf;
    int num_samples;
    FX* fx;

public:
    void setUp() {
        fx = new FX();
        sf = SndfileHandle("../tests/audio/flute.wav");
        num_samples = 4096;
    }

    void tearDown() {
        delete fx;
    }

    void test_basic() {
        sample* input = new sample[num_samples];
        memset(input, 0, sizeof(sample) * num_samples);
        sf.read(input, num_samples);

        sample* output = new sample[num_samples];
        memset(output, 0, sizeof(sample) * num_samples);

        fx->process(num_samples, input, num_samples, output);

        for(int i = 0; i < num_samples - fx->hop_size(); i += fx->hop_size()) {
            double energy = 0.f;
            for(int j = 0; j < fx->hop_size(); j++) {
                energy += output[i + j] * output[i + j];
            }
            CPPUNIT_ASSERT(energy > 0.f);
        }

        delete [] input;
        delete [] output;
    }
};


// ---------------------------------------------------------------------------
//	TestTimeScale
// ---------------------------------------------------------------------------
class TestTimeScale : public CPPUNIT_NS::TestCase {
    CPPUNIT_TEST_SUITE(TestTimeScale);
    CPPUNIT_TEST(test_basic);
    CPPUNIT_TEST_SUITE_END();

protected:
    static const double PRECISION = 0.001;
    SndfileHandle sf;
    int num_samples;
    TimeScale* ts;

public:
    void setUp() {
        ts = new TimeScale();
        sf = SndfileHandle("../tests/audio/flute.wav");
        num_samples = 4096;
    }

    void tearDown() {
        delete ts;
    }

    void test_basic() {
        sample* input = new sample[num_samples];
        memset(input, 0, sizeof(sample) * num_samples);
        sf.read(input, num_samples);

        sample* output = new sample[num_samples];
        memset(output, 0, sizeof(sample) * num_samples);

        ts->scale_factor(1.0);
        ts->process(num_samples, input, num_samples, output);

        for(int i = 0; i < num_samples - ts->hop_size(); i += ts->hop_size()) {
            double energy = 0.f;
            for(int j = 0; j < ts->hop_size(); j++) {
                energy += output[i + j] * output[i + j];
            }
            CPPUNIT_ASSERT(energy > 0.f);
        }

        delete [] input;
        delete [] output;
    }
};

} // end of namespace metamorph

CPPUNIT_TEST_SUITE_REGISTRATION(metamorph::TestFX);
CPPUNIT_TEST_SUITE_REGISTRATION(metamorph::TestTimeScale);

int main(int arg, char **argv) {
    CppUnit::TextTestRunner runner;
    runner.addTest(CppUnit::TestFactoryRegistry::getRegistry().makeTest());
    return runner.run("", false);
}

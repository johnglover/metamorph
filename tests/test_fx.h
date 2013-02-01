#include <cppunit/extensions/HelperMacros.h>

#include "test_common.h"
#include "metamorph.h"

namespace metamorph
{


// ---------------------------------------------------------------------------
//	TestFX
// ---------------------------------------------------------------------------
class TestFX : public CPPUNIT_NS::TestCase {
    CPPUNIT_TEST_SUITE(TestFX);
    CPPUNIT_TEST(test_basic);
    CPPUNIT_TEST(test_transposition_with_env);
    CPPUNIT_TEST_SUITE_END();

protected:
    SndfileHandle _sf;
    FX _fx;

public:
    void setUp();

    void test_basic();
    void test_transposition_with_env();
};


// ---------------------------------------------------------------------------
//	TestTimeScale
// ---------------------------------------------------------------------------
class TestTimeScale : public CPPUNIT_NS::TestCase {
    CPPUNIT_TEST_SUITE(TestTimeScale);
    CPPUNIT_TEST(test_basic);
    CPPUNIT_TEST_SUITE_END();

protected:
    SndfileHandle _sf;
    TimeScale _ts;

public:
    void setUp();
    void test_basic();
};


} // end of namespace metamorph

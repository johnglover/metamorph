#include <cppunit/ui/text/TextTestRunner.h>
#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/extensions/TestFactoryRegistry.h>

#include "test_fx.h"
 
CPPUNIT_TEST_SUITE_REGISTRATION(metamorph::TestFX);
CPPUNIT_TEST_SUITE_REGISTRATION(metamorph::TestTimeScale);

int main(int arg, char **argv) {
    CppUnit::TextTestRunner runner;
    runner.addTest(CppUnit::TestFactoryRegistry::getRegistry().makeTest());
    return runner.run("", false);
}

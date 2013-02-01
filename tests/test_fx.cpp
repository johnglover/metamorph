#include "test_fx.h"

using namespace metamorph;

void TestFX::setUp() {
    _sf = SndfileHandle(TEST_AUDIO_FILE);

    if(_sf.error() > 0) {
        throw Exception(std::string("Could not open audio file: ") +
                        std::string(TEST_AUDIO_FILE));
    }
}


void TestFX::test_basic() {
    std::vector<sample> audio(_sf.frames(), 0.0);
    _sf.read(&audio[0], (int)_sf.frames());
    std::vector<sample> audio_out(_sf.frames(), 0.0);

    _fx.reset();
    _fx.process(audio.size(), &audio[0], audio_out.size(), &audio_out[0]);

    for(int i = 0; i < audio_out.size() - _fx.hop_size(); i += _fx.hop_size()) {
        double energy = 0.f;
        for(int j = 0; j < _fx.hop_size(); j++) {
            energy += audio_out[i + j] * audio_out[i + j];
        }
        CPPUNIT_ASSERT(energy > 0.f);
    }
}

void TestFX::test_transposition_with_env() {
    std::vector<sample> audio(_sf.frames(), 0.0);
    _sf.read(&audio[0], (int)_sf.frames());
    std::vector<sample> audio_out(_sf.frames(), 0.0);

    _fx.reset();
    _fx.preserve_envelope(true);

    Transposition trans(4);
    _fx.add_transformation(&trans);

    _fx.process(audio.size(), &audio[0], audio_out.size(), &audio_out[0]);

    for(int i = 0; i < audio_out.size() - _fx.hop_size(); i += _fx.hop_size()) {
        double energy = 0.f;
        for(int j = 0; j < _fx.hop_size(); j++) {
            energy += audio_out[i + j] * audio_out[i + j];
        }
        CPPUNIT_ASSERT(energy > 0.f);
    }

    _fx.clear_harmonic_transformations();
}


void TestTimeScale::setUp() {
    _sf = SndfileHandle(TEST_AUDIO_FILE);

    if(_sf.error() > 0) {
        throw Exception(std::string("Could not open audio file: ") +
                        std::string(TEST_AUDIO_FILE));
    }
}

void TestTimeScale::test_basic() {
    std::vector<sample> audio(_sf.frames(), 0.0);
    _sf.read(&audio[0], (int)_sf.frames());
    std::vector<sample> audio_out(_sf.frames(), 0.0);

    _ts.scale_factor(1.0);
    _ts.process(audio.size(), &audio[0], audio_out.size(), &audio_out[0]);

    for(int i = 0; i < audio_out.size() - _ts.hop_size(); i += _ts.hop_size()) {
        double energy = 0.f;
        for(int j = 0; j < _ts.hop_size(); j++) {
            energy += audio_out[i + j] * audio_out[i + j];
        }
        CPPUNIT_ASSERT(energy > 0.f);
    }
}

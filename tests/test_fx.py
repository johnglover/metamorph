import os
from nose.tools import assert_almost_equals
import numpy as np
import metamorph

float_precision = 5
frame_size = 512
hop_size = 512
audio_path = os.path.join(
    os.path.dirname(__file__), 'audio/sax.wav'
)


class TestFX(object):
    @classmethod
    def setup_class(cls):
        cls.audio = metamorph.read_wav(audio_path)[0]
        cls.audio = cls.audio[len(cls.audio) / 2:(len(cls.audio) / 2) + 4096]

    def test_basic(self):
        fx = metamorph.FX()
        output = fx.process(self.audio)
        assert len(output) == len(self.audio)

    def test_scales_0(self):
        fx = metamorph.FX()
        fx.harmonic_scale = 0
        fx.residual_scale = 0
        fx.transient_scale = 0
        output = fx.process(self.audio)
        assert len(output) == len(self.audio)
        for i in range(len(output)):
            assert_almost_equals(output[i], 0.0, float_precision)

    def test_harmonic_non_0(self):
        fx = metamorph.FX()
        fx.harmonic_scale = 1
        fx.residual_scale = 0
        fx.transient_scale = 0
        output = fx.process(self.audio)
        assert len(output) == len(self.audio)
        assert np.max(output) > 0

    def test_harmonic_distortion(self):
        fx = metamorph.FX()
        fx.fundamental_frequency = 440
        fx.harmonic_distortion = 0
        fx.residual_scale = 0
        fx.transient_scale = 0
        output = fx.process(self.audio)
        assert len(output) == len(self.audio)
        assert np.max(output) > 0


class TestTimeScale(object):
    @classmethod
    def setup_class(cls):
        cls.audio = metamorph.read_wav(audio_path)[0]

    def test_basic(self):
        ts = metamorph.TimeScale()
        ts.scale_factor = 1.0
        output = ts.process(self.audio)
        assert len(output) == len(self.audio)

    def test_scale_2(self):
        ts2 = metamorph.TimeScale()
        ts2.scale_factor = 2.0
        output2 = ts2.process(self.audio)
        assert len(output2) == 2 * len(self.audio)

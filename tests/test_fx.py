import os
from nose.tools import assert_almost_equals
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

    def test_basic(self):
        fx = metamorph.FX()
        output = fx.process(self.audio)
        assert len(output) == len(self.audio)


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

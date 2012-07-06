import os
import numpy as np
from nose.tools import assert_almost_equals
import metamorph

float_precision = 5
frame_size = 512
hop_size = 512
audio_path = os.path.join(
    os.path.dirname(__file__), 'audio/flute.wav'
)


class TestFX(object):
    @classmethod
    def setup_class(cls):
        cls.audio = metamorph.read_wav(audio_path)[0]

    def test_basic(self):
        fx = metamorph.FX()
        print fx

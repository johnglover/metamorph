import numpy as np
import scipy.io.wavfile as wav
import fx

dtype = np.double

FX = fx.FX


def read_wav(file):
    'return floating point values between -1 and 1'
    audio = wav.read(file)
    return np.asarray(audio[1], dtype=dtype) / 32768.0, audio[0]

import numpy as np
import scipy.io.wavfile as wav
import fx

dtype = np.double

FX = fx.FX
Transposition = fx.Transposition
HarmonicDistortion = fx.HarmonicDistortion
TransientLPF = fx.TransientLPF
TransientHPF = fx.TransientHPF
TimeScale = fx.TimeScale
SpectralEnvelope = fx.SpectralEnvelope


def read_wav(file):
    'return floating point values between -1 and 1'
    try:
        from scikits.audiolab import wavread
        audio, sampling_rate, enc = wavread(file)
        audio = np.asarray(audio, dtype=dtype)
    except ImportError:
        sampling_rate, audio = wav.read(file)
        audio = np.asarray(audio, dtype=dtype) / 32768.0

    # if wav file has more than 1 channel, just take the first one
    if audio.ndim > 1:
        audio = audio.T[0]

    return audio, sampling_rate

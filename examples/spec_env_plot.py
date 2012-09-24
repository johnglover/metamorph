import sys
import scipy as sp
import numpy as np
import matplotlib.pyplot as plt
import simpl
import metamorph

if not len(sys.argv) == 2:
    print 'Usage:', __file__, '<input wav file>'
    sys.exit(1)

order = 25
env_size = 64
frame_size = 512

input_path = sys.argv[1]
audio, sampling_rate = metamorph.read_wav(input_path)

print 'Plotting spectral envelope from 1 frame in the middle of the sample'

pd = simpl.LorisPeakDetection()
pd.frame_size = frame_size
pd.max_peaks = env_size
frames = pd.find_peaks(audio)

frame = frames[len(frames) / 2]
freqs = np.array([p.frequency for p in frame.peaks])
amps = np.array([p.amplitude for p in frame.peaks])

e = metamorph.SpectralEnvelope(order, env_size)
env = e.envelope(freqs, amps)

if np.max(amps) > 0:
    amps = 20 * sp.log10(amps)

if np.max(env) > 0:
    env = 20 * sp.log10(env)

env_freqs = np.linspace(0, sampling_rate / 2, len(env))

plt.plot(freqs, amps, 'r+')
plt.plot(env_freqs, env, 'b-')
plt.ylabel('Magnitude (dB)')
plt.xlabel('Frequency (Hz)')
plt.show()

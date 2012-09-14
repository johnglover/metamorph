import sys
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
audio = audio[len(audio) / 2: (len(audio) / 2) + frame_size]

print 'Plotting spectral envelope from 1 frame in the middle of the sample'

e = metamorph.SpectralEnvelope(order, env_size)
pd = simpl.LorisPeakDetection()
pd.frame_size = frame_size
pd.max_peaks = env_size
frames = pd.find_peaks(audio)

freqs = np.zeros(len(frames[0].peaks))
mags = np.zeros(len(frames[0].peaks))

for i in range(len(freqs)):
    freqs[i] = frames[0].peaks[i].frequency
    mags[i] = frames[0].peaks[i].amplitude

env = e.env(freqs, mags)
env_freqs = np.linspace(0, sampling_rate / 2, len(env))

plt.plot(freqs, mags, 'r+')
plt.plot(env_freqs, env, 'b-')
plt.show()

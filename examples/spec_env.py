import sys
import numpy as np
import scipy.io.wavfile as wav
import simpl
import metamorph

if not len(sys.argv) == 4:
    print 'Usage:', __file__,\
        '<main input wav file> <wav file to take envelope from>',\
        '<output wav file>'
    sys.exit(1)

input_path = sys.argv[1]
env_path = sys.argv[2]
output_path = sys.argv[3]

order = 25
env_size = 256
frame_size = 2048

audio, sampling_rate = metamorph.read_wav(input_path)

env_audio = metamorph.read_wav(env_path)[0]
env_audio = env_audio[len(env_audio) / 2: (len(env_audio) / 2) + frame_size]

print 'Applying a (static) spectral envelope from', env_path

# get a spectral envelope from the centre of second wav file
pd = simpl.LorisPeakDetection()
pd.frame_size = frame_size
pd.hop_size = frame_size
pd.max_peaks = env_size
frames = pd.find_peaks(env_audio)

if not len(frames[0].peaks):
    print 'No peaks found in', env_path
    print 'Exiting'
    sys.exit()

freqs = np.zeros(len(frames[0].peaks))
mags = np.zeros(len(frames[0].peaks))
for i in range(len(freqs)):
    freqs[i] = frames[0].peaks[i].frequency
    mags[i] = frames[0].peaks[i].amplitude

e = metamorph.SpectralEnvelope(order, env_size)
env = e.envelope(freqs, mags)

# apply the spectral envelope to the partials in the first wav file
fx = metamorph.FX()
fx.new_envelope(env)
fx.env_interp = 1
fx.residual_scale = 0
output = fx.process(audio)

wav.write(output_path, sampling_rate, np.array(output * 32768, dtype=np.int16))

print 'Done. Saved to', output_path

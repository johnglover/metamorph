import sys
import numpy as np
import scipy.io.wavfile as wav
import metamorph

if not len(sys.argv) == 4:
    print 'Usage:', __file__, '<input wav file> <fundamental frequency>',
    print '<output wav file>'
    sys.exit(1)

input_path = sys.argv[1]
fundamental_frequency = float(sys.argv[2])
output_path = sys.argv[3]

audio, sampling_rate = metamorph.read_wav(input_path)

print 'Applying harmonic distortion (setting partials to multiples of 220 Hz)'

fx = metamorph.FX()
fx.fundamental_frequency = fundamental_frequency
fx.harmonic_distortion = 0
fx.residual_scale = 0

output = fx.process(audio)
wav.write(output_path, sampling_rate, np.array(output * 32768, dtype=np.int16))

print 'Done. Saved to', output_path

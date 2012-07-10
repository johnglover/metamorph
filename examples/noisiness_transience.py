import sys
import numpy as np
import scipy.io.wavfile as wav
import metamorph

if not len(sys.argv) == 3:
    print 'Usage:', __file__, '<input wav file> <output wav file>'
    sys.exit(1)

input_path = sys.argv[1]
output_path = sys.argv[2]

audio, sampling_rate = metamorph.read_wav(input_path)

print 'Synthesising just the harmonic component (no transient or residual)...'

fx = metamorph.FX()
fx.residual_scale = 0
fx.transient_scale = 0

output = fx.process(audio)
wav.write(output_path, sampling_rate, np.array(output * 32768, dtype=np.int16))

print 'Done. Saved to', output_path

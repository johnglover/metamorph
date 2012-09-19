import sys
import numpy as np
import scipy.io.wavfile as wav
import metamorph

if not len(sys.argv) == 6:
    print 'Usage:', __file__, '<input wav file>',
    print '<harmonic scale> <residual scale> <transient scale>',
    print '<output wav file>'
    sys.exit(1)

input_path = sys.argv[1]
output_path = sys.argv[5]
audio, sampling_rate = metamorph.read_wav(input_path)

print 'Scaling the harmonic component by:', sys.argv[2]
print 'Scaling the residual component by:', sys.argv[3]
print 'Scaling the transient component(s) by:', sys.argv[4]

fx = metamorph.FX()
fx.harmonic_scale = float(sys.argv[2])
fx.residual_scale = float(sys.argv[3])
fx.transient_scale = float(sys.argv[4])
output = fx.process(audio)

wav.write(output_path, sampling_rate, np.array(output * 32768, dtype=np.int16))
print 'Done. Saved to', output_path

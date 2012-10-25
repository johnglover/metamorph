import sys
import numpy as np
import scipy.io.wavfile as wav
import metamorph

if not len(sys.argv) == 4:
    print 'Usage:', __file__, '<input wav file> <time scale factor>',
    print '<output wav file>'
    sys.exit(1)

input_path = sys.argv[1]
time_scale_factor = float(sys.argv[2])
output_path = sys.argv[3]

audio, sampling_rate = metamorph.read_wav(input_path)

print 'Time scaling by a factor of', time_scale_factor

ts = metamorph.TimeScale()
ts.scale_factor = time_scale_factor
ts.residual_scale = 0

output = ts.process(audio)
wav.write(output_path, sampling_rate, np.array(output * 32768, dtype=np.int16))

print 'Done. Saved to', output_path

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

print 'Time scaling by a factor of 2...'

ts = metamorph.TimeScale()
ts.scale_factor = 2.0

output = ts.process(audio)
wav.write(output_path, sampling_rate, np.array(output * 32768, dtype=np.int16))

print 'Done. Saved to', output_path

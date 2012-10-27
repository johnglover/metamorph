#include <iostream>
#include <sndfile.hh>
#include "metamorph.h"

using namespace metamorph;


class EnvInterp : public SpecEnvTransformation {
    private:
        FX* _fx;
        int _num_frames;
        double _interp;
        double _interp_step;
        std::vector<double> _env;

    public:
        EnvInterp(FX* fx, int num_frames) {
            _fx = fx;
            _num_frames = num_frames;

            _interp = 0.0;
            _interp_step = 1.0 / num_frames;

            _env.resize(fx->env_size());

            // create a linear ramp from 0.5 to 0 over the duration
            // of the envelope
            for(int n = _env.size() - 1; n >= 0; n--) {
                _env[n] = ((double)n / 2.0) / (_env.size() - 1);
            }
            fx->apply_envelope(_env);
        }

        void process_frame(simpl::Frame* frame, std::vector<double>& new_env) {
            // gradually change envelope interpolation until the applied
            // envelope is the linear ramp
            _interp += _interp_step;
            _fx->env_interp(_interp);
        }
};


int main(int argc, char* argv[]) {
    if(argc != 3) {
        std::cout << "Usage: " << argv[0] << " <input file> <output file>";
        std::cout << std::endl;
        exit(1);
    }

    SndfileHandle input_file = SndfileHandle(argv[1]);
    int num_samples = (int)input_file.frames();
    int hop_size = 512;
    int frame_size = 2048;
    int num_frames = (num_samples - frame_size) / hop_size;

    FX fx;
    fx.hop_size(hop_size);
    fx.frame_size(frame_size);
    fx.preserve_envelope(true);

    EnvInterp env_interp = EnvInterp(&fx, num_frames);
    fx.add_specenv_transformation(&env_interp);

    std::vector<double> input(num_samples);
    std::vector<double> output(num_samples);

    input_file.read(&(input[0]), num_samples);

    for(int n = 0; n < (num_samples - hop_size); n += hop_size) {
        fx.process_frame(hop_size, &(input[n]), hop_size, &(output[n]));
    }

    int output_format = SF_FORMAT_WAV | SF_FORMAT_PCM_16;
    int output_channels = 1;
    int output_sampling_rate = 44100;

    SndfileHandle output_file = SndfileHandle(argv[2],
                                              SFM_WRITE,
                                              output_format,
                                              output_channels,
                                              output_sampling_rate);
    output_file.write(&(output[0]), num_samples);

    return 0;
}

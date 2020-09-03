#pragma once

#include <kfr/base.hpp> // kfr core
#include <kfr/dft.hpp>  // dft
#include <kfr/dsp.hpp>  // window functions

namespace spectrum {
    using namespace kfr;

        univector<float> compute_dft(const univector<float>& _audio_data, uint32_t _samplerate) {
            univector<float>            tmp;
            univector<float>            dft_data;

            uint32_t seconds = _audio_data.size() / _samplerate;
            // compute and sum dft data
            for(int i = 0; i < seconds; i++) {
                tmp = _audio_data.slice(i * _samplerate, _samplerate) * window_hann(_samplerate);
                univector<float> dft_output = cabs(realdft(tmp));
                dft_data.insert(dft_data.begin(), dft_output.begin(), dft_output.end());
            }

            return dft_data;
        }

        univector<float> max_bins(const univector<float>& dft_data, uint32_t _samplerate) {
            uint32_t seconds = (dft_data.size()) / _samplerate;
            univector<float> max_bins(seconds);
            univector2d<float> freq_graph(seconds);
            // compute and sum dft data
            for(int i = 0; i < seconds; i++) {
                
                univector<float> magnitudes = dft_data.slice(i * _samplerate, _samplerate);
                float mean_magnitude = mean(magnitudes);
                float max_magnitude = absmaxof(magnitudes);
                float min_magnitude = absminof(magnitudes);
                
                float max_freq = 0;
                for(int j = 0; j < magnitudes.size(); j++) {
                    float f = magnitudes[j];
                    if(f <= 5 * min_magnitude) {
                        float freq = j * _samplerate / magnitudes.size();
                        if(freq > max_freq) {max_freq = freq;}
                    }
                }
                max_bins[i] = max_freq;
            }
            return max_bins;
        }

        univector<float> average(const univector<float>& _data, uint32_t window) {
            univector<float> padded(_data.size() + window);
            univector<float> smooth_data(_data.size());

            std::fill(padded.begin(), padded.begin()+window, _data[0]);
            std::copy(_data.begin(), _data.end(), padded.begin()+window);
            for(int i = 0;i < padded.size()-window; i++) {
                smooth_data[i] = mean(padded.slice(i, window));
            }

            return smooth_data;
        }


        float find_cutoff(const univector<float>& smooth_data, uint32_t window, float diff, float limit) {
            uint32_t cutoff = smooth_data.size();
            float last = smooth_data[smooth_data.size()-1];
            for(int i = smooth_data.size()-1; i > window; i--) {
                float a = smooth_data[i];
                float b = smooth_data[i-window];
                if(a / last > limit) {
                    break;
                }
                if( (b - a) > diff) {
                    cutoff = i;
                    break;
                }
            }
            return cutoff;
        }
};
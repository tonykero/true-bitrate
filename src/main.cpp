#include <string>
#include <iostream>
#include <numeric>

// imgui
#include <imgui.h>
#include <imgui_internal.h>
#include <implot.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

// OpenGL & GLFW
#include <glad/glad.h>
#include <GLFW/glfw3.h>

// kfr
#include <kfr/base.hpp>
#include <kfr/dft.hpp>
#include <kfr/dsp.hpp>
#include <kfr/io.hpp>

#include "spectrum.hpp"
#include "gfx.hpp"

static void glfw_error_callback(int error, const char* description)
{
    std::cout << "[GLFW] Error " << error << ":\t" << description << "\n";
}

using namespace kfr;

template <typename T>
std::shared_ptr<audio_reader<T>> reader_from_audio(const std::string& _filename) {
    auto file_reader = open_file_for_reading(_filename);

    std::string ext = _filename.substr(_filename.find_last_of('.')+1);
    
    if(ext == "flac")   return std::make_shared<audio_reader_flac<T>>(file_reader);
    if(ext == "wav")    return std::make_shared<audio_reader_wav<T>>(file_reader);

    return std::make_shared<audio_reader_mp3<T>>(file_reader);
}

int main(int argc, char** argv)
{
    univector<float> final_data, smooth_data, db_data;
    univector<float> max_bins;
    uint32_t window = 0, cutoff = 0, seconds = 0;
    if(argc > 1) {
        const std::string filename(argv[1]);
        auto reader_ptr = reader_from_audio<float>(filename);

        float duration      = reader_ptr->format().length / reader_ptr->format().samplerate;
        float sample_rate   = reader_ptr->format().samplerate;

        println("Sample Rate  = ", sample_rate);
        println("Channels     = ", reader_ptr->format().channels);
        println("Length       = ", reader_ptr->format().length);
        println("Duration (s) = ", duration);
        println("Bit depth    = ", audio_sample_bit_depth(reader_ptr->format().type));
        
        seconds    = min<uint32_t, uint32_t>(30, duration);
        uint32_t freq       = sample_rate;
        
        // raw data
        univector<float> audio_data = reader_ptr->read(freq*seconds);
        // perform dft
        univector<float> dft_data = spectrum::compute_dft(audio_data, freq);

        uint32_t size = sample_rate / 2;
        // sum frequencies to get magnitudes
        final_data.resize(size, 0);
        for(int i = 0; i < seconds; i++) {
            final_data += dft_data.slice(i * (size), size);
        }
        final_data = final_data.slice(0, final_data.size()/2);

        // store magnitudes as decibels
        db_data = 20 * log10(2 * final_data / size);
        
        // normalize magnitudes
        final_data /= seconds;
        final_data = log10(final_data);
        
        // compute frequencies with lowest magnitudes
        max_bins = spectrum::max_bins(dft_data, freq/2);

        // smooth with moving average
        window = freq/400;
        smooth_data = spectrum::average(final_data, window);
        
        // find cutoff
        cutoff = spectrum::find_cutoff(smooth_data, window, 0.5, 1.1);
        cutoff -=window;
        std::cout << "cutoff at " << cutoff * 2 << " Hz";
    }
    // Setup window
    if(!gfx::init(glfw_error_callback)) 
        return 1;

    {
        gfx::window& window_obj = gfx::window::instance(1280, 720, "True bitrate");
        

        static univector<float> xs(final_data.size());
        static univector<float> xs_ma(final_data.size());
        std::iota(xs.begin(), xs.end(), .0);
        std::copy(xs.begin(), xs.end(), xs_ma.begin());
        smooth_data = smooth_data.slice(window, smooth_data.size()-window);

        static univector<float> xs_secs(seconds);
        std::iota(xs_secs.begin(), xs_secs.end(), 0);

        const std::string str_ma("MA " + std::to_string(window));

        window_obj.loop([&](int display_w, int display_h){
            ImGui::SetNextWindowPos(ImVec2(0,0));
            ImGui::SetNextWindowSize(ImVec2(display_w, display_h));
            ImGui::Begin("##Main", nullptr, ImGuiWindowFlags_NoCollapse |
                                            ImGuiWindowFlags_NoMove     |
                                            ImGuiWindowFlags_NoResize   |
                                            ImGuiWindowFlags_NoTitleBar);   // Pass a pointer to our bool variable (the window will have a closing button that will clear the bool when clicked)

            ImPlot::SetNextPlotLimits(-1000, final_data.size(), -5, 5);
            ImPlot::SetNextPlotLimitsY(-120, 0, ImGuiCond_Once, 1);
            ImPlot::SetNextPlotLimitsY(-5, 5, ImGuiCond_Once, 2);
            if (ImPlot::BeginPlot("DFT", "Frequencies", "Magnitude", ImVec2(-1, display_h/2.1), ImPlotFlags_Default | ImPlotFlags_YAxis2)) {
                ImPlot::PushStyleVar(ImPlotStyleVar_LineWeight, 1.0f);
                ImPlot::SetPlotYAxis(1);
                ImPlot::PlotLine("DFT", xs.data(), db_data.data(), db_data.size());
                ImPlot::PushStyleVar(ImPlotStyleVar_LineWeight, 2.0f);
                ImPlot::SetPlotYAxis(2);
                ImPlot::PlotLine(str_ma.c_str(), xs_ma.data(), smooth_data.data(), smooth_data.size());
                // draw vertical indicator
                ImVec2 p0 = ImPlot::PlotToPixels(ImPlotPoint(cutoff, -10000));
                ImVec2 p1 = ImPlot::PlotToPixels(ImPlotPoint(cutoff, +10000));
                ImPlot::PushPlotClipRect();
                ImDrawList* drawList = ImGui::GetWindowDrawList();
                drawList->AddLine(p0, p1, IM_COL32(255, 0, 0, 255));
                ImPlot::PopPlotClipRect();
                ImPlot::EndPlot();
            }
            ImPlot::SetNextPlotLimits(0, seconds, 0, 44100);
            if(ImPlot::BeginPlot("Frequency graph", "Seconds", "Frequency", ImVec2(-1, display_h/2.1))) {
                ImPlot::PlotLine("max_bins", xs_secs.data(), max_bins.data(), max_bins.size());
                ImPlot::EndPlot();
            }
            ImGui::End();
        });
    }
    
    gfx::destroy();

    return 0;
}
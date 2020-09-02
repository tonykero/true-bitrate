#include <string>
#include <iostream>
#include <memory>

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
    univector<float> final_data, smooth_data;
    uint32_t cutoff;
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
        
        uint32_t seconds    = min<uint32_t, uint32_t>(30, duration);
        uint32_t freq       = sample_rate;
        
        // raw data
        univector<float> audio_data = reader_ptr->read(freq*seconds);
        // perform dft
       final_data = spectrum::process_data(audio_data, freq);
        
        // smooth
        uint32_t window = freq/100;
       smooth_data = spectrum::average(final_data, window);
       smooth_data = smooth_data.slice(0, smooth_data.size()/2);
        
        // find cutoff
        cutoff = spectrum::find_cutoff(smooth_data, window, 1.25, 1.1);
        std::cout << "cutoff at " << cutoff * 2 << " Hz";
    }
    // Setup window
    if(!gfx::init(glfw_error_callback)) 
        return 1;

    {
        gfx::window& window_obj = gfx::window::instance(1280, 720, "True bitrate");
        
        ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
        static float* xs = (float*)malloc(final_data.size() * sizeof(float));
        for(int i = 0; i < final_data.size(); i++) {
            xs[i] = i;
        }

        window_obj.loop([&](int display_w, int display_h){
            ImGui::SetNextWindowPos(ImVec2(0,0));
            ImGui::SetNextWindowSize(ImVec2(display_w, display_h));
            ImGui::Begin("##Main", nullptr, ImGuiWindowFlags_NoCollapse |
                                            ImGuiWindowFlags_NoMove     |
                                            ImGuiWindowFlags_NoResize   |
                                            ImGuiWindowFlags_NoTitleBar);   // Pass a pointer to our bool variable (the window will have a closing button that will clear the bool when clicked)

            ImPlot::SetNextPlotLimits(0, final_data.size(), -5, 5);
            if (ImPlot::BeginPlot("Line Plot", "Frequencies", "Magnitude", ImVec2(-1, -1))) {
                ImPlot::PushStyleVar(ImPlotStyleVar_LineWeight, 1.0f);
                ImPlot::PlotLine("DFT", xs, final_data.data(), final_data.size());
                ImPlot::PushStyleVar(ImPlotStyleVar_LineWeight, 2.0f);
                ImPlot::PlotLine("smooth", xs, smooth_data.data(), smooth_data.size());
                
                // draw vertical indicator
                ImVec2 p0 = ImPlot::PlotToPixels(ImPlotPoint(cutoff, -10000));
                ImVec2 p1 = ImPlot::PlotToPixels(ImPlotPoint(cutoff, +10000));
                ImPlot::PushPlotClipRect();
                ImDrawList* drawList = ImGui::GetWindowDrawList();
                drawList->AddLine(p0, p1, IM_COL32(255, 0, 0, 255));
                ImPlot::PopPlotClipRect();
                ImPlot::EndPlot();
            }
            ImGui::End();
        });
        free(xs);
    }
    
    gfx::destroy();

    return 0;
}
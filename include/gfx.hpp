#pragma once

#include <functional>
#include <iostream>
#include <string>
#include <memory>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

namespace gfx {
    int init(GLFWerrorfun _errcb) {
        glfwSetErrorCallback(_errcb);

        if(!glfwInit())
            return 0;
        
        return 1;
    }

    void destroy() {
        // ensures glfw is initialized
        if(glfwInit())
            glfwTerminate();
    }

    class window {
        window(int _width, int _height, const std::string& _title)
            : m_window(glfwCreateWindow(_width, _height, _title.c_str(), NULL, NULL)) {
                if(!m_window) {
                    std::cout << "[GLFW] Failed to create window\n";
                }
                glfwMakeContextCurrent(m_window);
                glfwSwapInterval(1); // vsync

                if(!gladLoadGL())
                    std::cout << "[GLAD] Failed to initialize OpenGL\n";

                // Setup Dear ImGui context
                IMGUI_CHECKVERSION();
                ImGui::CreateContext();
                ImGuiIO& io = ImGui::GetIO(); (void)io;

                // Setup Dear ImGui style
                ImGui::StyleColorsDark();

                // Setup Platform/Renderer bindings
                ImGui_ImplGlfw_InitForOpenGL(m_window, true);
                ImGui_ImplOpenGL3_Init("#version 130");
            }
        public:
            window(const window&)   = delete;
            window(window&&)        = delete;
            window& operator=(const window&)= delete;
            window& operator=(window&&)     = delete;
            
            ~window() {
                // ensures glfw is initialized
                glfwInit();

                // Cleanup
                ImGui_ImplOpenGL3_Shutdown();
                ImGui_ImplGlfw_Shutdown();
                ImGui::DestroyContext();

                glfwDestroyWindow(m_window);

                gfx::destroy();
            }

            static auto& instance(int w, int h, const std::string& t) {
                static window win(w,h,t);
                return win;
            }

            void loop(std::function<void(int, int)> _frame, ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f)) {
                while(!glfwWindowShouldClose(m_window)) {
                    glfwPollEvents();

                    int display_w, display_h;
                    glfwGetFramebufferSize(m_window, &display_w, &display_h);
                    // Start the Dear ImGui frame
                    ImGui_ImplOpenGL3_NewFrame();
                    ImGui_ImplGlfw_NewFrame();
                    ImGui::NewFrame();

                    _frame(display_w, display_h);
                    
                    // Rendering
                    ImGui::Render();
                    glViewport(0, 0, display_w, display_h);
                    glClearColor(clear_color.x, clear_color.y, clear_color.z, clear_color.w);
                    glClear(GL_COLOR_BUFFER_BIT);
                    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

                    glfwSwapBuffers(m_window);
                }
            }

            GLFWwindow* get_ptr() {return m_window;}
        private:
            GLFWwindow* m_window;
    };
}
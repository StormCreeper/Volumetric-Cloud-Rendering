#define _USE_MATH_DEFINES

#include "mesh.hpp"
#include "camera.hpp"
#include "shader.hpp"
#include "object3d.hpp"
#include "framebuffer.hpp"

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include "gl_includes.hpp"

#include <cmath>
#include <cstdlib>
#include <iostream>
#include <memory>
#include <string>

const int MAX_LIGHTS = 10;

// Window parameters
GLFWwindow *g_window {};

// GPU objects
GLuint g_geometryShader {};  // A GPU program contains at least a vertex shader and a fragment shader
GLuint g_lightingShader {}; // A GPU program contains at least a vertex shader and a fragment shader

Camera g_camera {};

std::shared_ptr<FrameBuffer> g_framebuffer {};

std::vector<std::shared_ptr<Object3D>> g_objects {};

struct Light {
    int type; // 0 = ambiant, 1 = point, 2 = directional
    glm::vec3 position;
    glm::vec3 color;
    float intensity;
};



struct Scene {
    Light m_lights[MAX_LIGHTS] {};
    int m_numLights = 0;

    glm::vec3 m_domainOrigin;
    glm::vec3 m_domainSize;



    void setUniforms(GLuint lightingShader) {
        for(int i = 0; i < g_scene.m_numLights; i++) {
            setUniform(lightingShader, std::string("u_lights[" + std::to_string(i) + "].type").c_str(), m_lights[i].type);
            setUniform(lightingShader, std::string("u_lights[" + std::to_string(i) + "].position").c_str(), m_lights[i].position);
            setUniform(lightingShader, std::string("u_lights[" + std::to_string(i) + "].color").c_str(), m_lights[i].color);
            setUniform(lightingShader, std::string("u_lights[" + std::to_string(i) + "].intensity").c_str(), m_lights[i].intensity);
        }

        setUniform(lightingShader, "u_numLights", m_numLights);
    }
};

Scene g_scene {};

void setDefaults() {

}



float g_fps = 0.0f;

// Executed each time the window is resized. Adjust the aspect ratio and the rendering viewport to the current window.
void windowSizeCallback(GLFWwindow *window, int width, int height) {
    g_camera.setAspectRatio(static_cast<float>(width) / static_cast<float>(height));
    glViewport(0, 0, (GLint)width, (GLint)height);  // Dimension of the rendering region in the window
}

// Executed each time a key is entered.
void keyCallback(GLFWwindow *window, int key, int scancode, int action, int mods) {
    if(action == GLFW_PRESS) {
        if (key == GLFW_KEY_W) {
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        }
        if (key == GLFW_KEY_F) {
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        }
        if ((key == GLFW_KEY_ESCAPE || key == GLFW_KEY_Q)) {
            glfwSetWindowShouldClose(window, true);  // Closes the application if the escape key is pressed
        }

    }
}

float g_cameraDistance = 5.0f;
float g_cameraAngleX = 0.0f;

// Scroll for zooming
void scrollCallback(GLFWwindow *window, double xoffset, double yoffset) {
    g_cameraDistance -= yoffset * 0.1f;
    g_cameraDistance = std::max(g_cameraDistance, 0.1f);

    g_cameraAngleX -= xoffset * 0.04f;
}

void errorCallback(int error, const char *desc) {
    std::cout << "Error " << error << ": " << desc << std::endl;
}

void initGLFW() {
    glfwSetErrorCallback(errorCallback);

    // Initialize GLFW, the library responsible for window management
    if (!glfwInit()) {
        std::cerr << "ERROR: Failed to init GLFW" << std::endl;
        std::exit(EXIT_FAILURE);
    }

    // Before creating the window, set some option flags
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_RESIZABLE, GL_TRUE);

    // Create the window
    g_window = glfwCreateWindow(
        1024, 768,
        "Interactive 3D Applications (OpenGL) - Simple Solar System", nullptr, nullptr);
    if (!g_window) {
        std::cerr << "ERROR: Failed to open window" << std::endl;
        glfwTerminate();
        std::exit(EXIT_FAILURE);
    }

    // Load the OpenGL context in the GLFW window using GLAD OpenGL wrangler
    glfwMakeContextCurrent(g_window);
    glfwSetWindowSizeCallback(g_window, windowSizeCallback);
    glfwSetKeyCallback(g_window, keyCallback);
    glfwSetScrollCallback(g_window, scrollCallback);
}

void initImGui() {
    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls

    // Setup Platform/Renderer backends
    ImGui_ImplGlfw_InitForOpenGL(g_window, true);          // Second param install_callback=true will install GLFW callbacks and chain to existing ones.
    ImGui_ImplOpenGL3_Init();
}

void initOpenGL() {
    // Load extensions for modern OpenGL
    if (!gladLoadGL()) {
        std::cerr << "ERROR: Failed to initialize OpenGL context" << std::endl;
        glfwTerminate();
        std::exit(EXIT_FAILURE);
    }

    glEnable(GL_DEBUG_OUTPUT);
    glCullFace(GL_BACK);                   // Specifies the faces to cull (here the ones pointing away from the camera)
    glEnable(GL_CULL_FACE);                // Enables face culling (based on the orientation defined by the CW/CCW enumeration).
    glDepthFunc(GL_LESS);                  // Specify the depth test for the z-buffer
    glEnable(GL_DEPTH_TEST);               // Enable the z-buffer test in the rasterization

    g_framebuffer = std::make_shared<FrameBuffer>(1024, 768);

    // Disable v-sync
    // glfwSwapInterval(0);
}

void initGPUprogram() {
    g_geometryShader = glCreateProgram();  // Create a GPU program, i.e., two central shaders of the graphics pipeline
    loadShader(g_geometryShader, GL_VERTEX_SHADER, "../resources/geometryVertex.glsl");
    loadShader(g_geometryShader, GL_FRAGMENT_SHADER, "../resources/geometryFragment.glsl");
    glLinkProgram(g_geometryShader);  // The main GPU program is ready to be handle streams of polygons

    g_lightingShader = glCreateProgram();  // Create a GPU program, i.e., two central shaders of the graphics pipeline
    loadShader(g_lightingShader, GL_VERTEX_SHADER, "../resources/lightingVertex.glsl");
    loadShader(g_lightingShader, GL_FRAGMENT_SHADER, "../resources/lightingFragment.glsl");
    glLinkProgram(g_lightingShader);  // The main GPU program is ready to be handle streams of polygons
}


void initScene() {
    g_objects.push_back(std::make_shared<Object3D>(Mesh::genSphere(16)));
    g_objects.push_back(std::make_shared<Object3D>(Mesh::genSubdividedPlane(2)));

    g_objects[0]->setModelMatrix(glm::identity<glm::mat4>());

    g_objects[1]->setModelMatrix(glm::scale(glm::mat4(1.0f), glm::vec3(10.0f, 10.0f, 10.0f)));
    g_objects[1]->setModelMatrix(glm::translate(g_objects[1]->getModelMatrix(), glm::vec3(0.0f, -1.0f, 0.0f)));

    g_scene.m_lights[g_scene.m_numLights++] = Light{
        1,
        glm::vec3(3.0f, 3.0f, 3.0f),
        glm::vec3(1.0, 1.0, 1.0),
        1.0f
    };

    setDefaults();
}

void initCamera() {
    int width, height;
    glfwGetWindowSize(g_window, &width, &height);
    g_camera.setAspectRatio(static_cast<float>(width) / static_cast<float>(height));

    g_camera.setPosition(glm::vec3(0.0, 0.0, 3.0));
    g_camera.setNear(0.1);
    g_camera.setFar(80);

    g_camera.setFoV(90);
}

void init() {
    initGLFW();
    initOpenGL();

    initScene();
    initGPUprogram();
    initCamera();
    
    initImGui();
}

void clear() {
    glDeleteProgram(g_geometryShader);

    glfwDestroyWindow(g_window);
    glfwTerminate();

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
}

void renderUI() {
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    // Start drawing here

    ImGui::Begin("Performance", nullptr, ImGuiWindowFlags_AlwaysAutoResize);

    ImGui::Text("FPS: %.1f", g_fps);

    ImGui::End();

    ImGui::Begin("Lights", nullptr, ImGuiWindowFlags_AlwaysAutoResize);

    const char* items[] = { "Ambiant", "Point", "Directional" };

    for(int i=0; i<g_scene.m_numLights; i++) {
        Light &light = g_scene.m_lights[i];
        if(ImGui::CollapsingHeader(std::string("Light " + std::to_string(i)).c_str())) {
            //ImGui::Text("Light %d", i);

            const char* comboLabel = items[light.type];

            if (ImGui::BeginCombo(("Type" + std::to_string(i)).c_str(), comboLabel)) { // Combo box for type selection 
                for (int n = 0; n < IM_ARRAYSIZE(items); n++) {
                    const bool is_selected = (comboLabel == items[n]);
                    if (ImGui::Selectable(items[n], is_selected)) {
                        light.type = n;
                    }
                    if (is_selected) {
                        ImGui::SetItemDefaultFocus();   // You may set the initial focus when opening the combo (scrolling + for keyboard navigation support)
                    }
                }
                ImGui::EndCombo();
            }

            if(light.type != 0)
                ImGui::SliderFloat3(((light.type == 1 ? "Position" : "Direction") + std::to_string(i)).c_str(), &light.position.x, -10.0f, 10.0f);
            ImGui::ColorEdit3(("Color" + std::to_string(i)).c_str(), &light.color.x);
            ImGui::SliderFloat(("Intensity" + std::to_string(i)).c_str(), &light.intensity, 0.0f, light.type == 0 ? 1.0f : 10.0f);
        }
    }
    if(g_scene.m_numLights < MAX_LIGHTS && ImGui::Button("Add light")) {
        g_scene.m_lights[g_scene.m_numLights++] = Light{
            1,
            glm::vec3(0.0f, 0.0f, 0.0f),
            glm::vec3(1.0, 1.0, 1.0),
            1.0f
        };
        ImGui::SameLine();
    }
    if(g_scene.m_numLights > 0 && ImGui::Button("Remove light")) {
        g_scene.m_numLights--;
    }

    ImGui::End();

    // End drawing here

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void setParamsUniforms() {
    
}

void setUniforms() {
    const glm::mat4 viewMatrix = g_camera.computeViewMatrix();
    const glm::mat4 projMatrix = g_camera.computeProjectionMatrix();
    
    setUniform(g_geometryShader, "u_viewMat", viewMatrix);
    setUniform(g_geometryShader, "u_projMat", projMatrix);
    setUniform(g_geometryShader, "u_proj_viewMat", projMatrix * viewMatrix);

    setUniform(g_geometryShader, "u_modelMat", glm::mat4(1.0f));
    setUniform(g_geometryShader, "u_transposeInverseModelMat", glm::mat4(1.0f));

    setUniform(g_geometryShader, "u_cameraPosition", g_camera.getPosition());

    setUniform(g_geometryShader, "u_time", static_cast<float>(glfwGetTime()));

    setParamsUniforms();
}

// The main rendering call
void render() {
    
    // Geometry pass
    glBindFramebuffer(GL_FRAMEBUFFER, g_framebuffer->m_Buffer);

    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);  // specify the background color, used any time the framebuffer is cleared
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);  // Erase the color and z buffers.
    glUseProgram(g_geometryShader);

    setUniforms();

    // Render objects

    for(std::shared_ptr<Object3D> mesh : g_objects) {
        mesh->render(g_geometryShader);
    }

    // Post-process pass
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glUseProgram(g_lightingShader);

    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);  // specify the background color, used any time the framebuffer is cleared
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);  // Erase the color and z buffers.

    setUniform(g_lightingShader, "u_Position", 0);
    setUniform(g_lightingShader, "u_Normal", 1);
    setUniform(g_lightingShader, "u_Albedo", 2);
    setUniform(g_lightingShader, "u_Random", 3);

    const glm::mat4 viewMatrix = g_camera.computeViewMatrix();
    const glm::mat4 projMatrix = g_camera.computeProjectionMatrix();

    setUniform(g_lightingShader, "u_viewMat", viewMatrix);
    setUniform(g_lightingShader, "u_projMat", projMatrix);
    setUniform(g_lightingShader, "u_invViewMat", glm::inverse(viewMatrix));
    setUniform(g_lightingShader, "u_invProjMat", glm::inverse(projMatrix));

    setUniform(g_lightingShader, "u_time", static_cast<float>(glfwGetTime()));

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, g_framebuffer->m_position);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, g_framebuffer->m_normal);
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, g_framebuffer->m_albedo);

    glBindVertexArray(g_framebuffer->m_quad->m_vao);
    glDrawElements(GL_TRIANGLES, g_framebuffer->m_quad->m_numIndices, GL_UNSIGNED_INT, 0);

    renderUI();
}

// Update any accessible variable based on the current time
void update(const float currentTimeInSec) {

    // Update the FPS computation
    static int frameCount = 0;
    static float lastTime = glfwGetTime();
    frameCount++;

    if (currentTimeInSec - lastTime >= 1.0) {
        g_fps = static_cast<float>(frameCount) / (currentTimeInSec - lastTime);
        frameCount = 0;
        lastTime = currentTimeInSec;
    }

    // Update the camera position

    glm::vec3 targetPosition = glm::vec3(0.0f, 0.0f, 0.0f);
    g_camera.setTarget(targetPosition);

    glm::vec3 cameraOffset = glm::normalize(glm::vec3(cos(g_cameraAngleX), 0.3f, sin(g_cameraAngleX))) * (1.1f + g_cameraDistance);
    g_camera.setPosition(targetPosition + cameraOffset);
}

int main(int argc, char **argv) {
    init();
    while (!glfwWindowShouldClose(g_window)) {
        update(static_cast<float>(glfwGetTime()));
        render();
        glfwSwapBuffers(g_window);
        glfwPollEvents();
    }
    clear();
    return EXIT_SUCCESS;
}

#ifndef CLOUD_MANAGER_HPP
#define CLOUD_MANAGER_HPP

#include "gl_includes.hpp"
#include "shader.hpp"

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"


struct VolumeParams {
    int numSteps;
    int numLightSteps;

    float stepSize;
    float lightStepSize;

    float cloudAbsorption;
    float lightAbsorption;
    float densityMultiplier;

    float scatteringG;
    glm::vec4 phaseParams;
};

struct GenerationParams {
    glm::vec3 domainCenter {};
    glm::vec3 domainSize {};

    glm::vec3 worldOffset {};
    glm::vec3 worldSize {};
};

class CloudsManager {
public:
    VolumeParams m_volumeParams {};
    GenerationParams m_generationParams {};


public:
    CloudsManager() = default;
    ~CloudsManager() = default;
    
    void setUniforms(GLuint shader) {
        setUniform(shader, "MAX_STEPS", m_volumeParams.numSteps);
        setUniform(shader, "MAX_LIGHT_STEPS", m_volumeParams.numLightSteps);
        setUniform(shader, "u_stepSize", m_volumeParams.stepSize);
        setUniform(shader, "u_lightStepSize", m_volumeParams.lightStepSize);

        setUniform(shader, "u_cloudAbsorption", m_volumeParams.cloudAbsorption);
        setUniform(shader, "u_lightAbsorption", m_volumeParams.lightAbsorption);
        setUniform(shader, "u_densityMultiplier", m_volumeParams.densityMultiplier);

        setUniform(shader, "u_scatteringG", m_volumeParams.scatteringG);
        setUniform(shader, "u_phaseParams", m_volumeParams.phaseParams);

        setUniform(shader, "u_domainCenter", m_generationParams.domainCenter);
        setUniform(shader, "u_domainSize", m_generationParams.domainSize);
    }

    void setDefaults() {
        m_volumeParams.numSteps = 30;
        m_volumeParams.numLightSteps = 10;

        m_volumeParams.stepSize = 0.01f;
        m_volumeParams.lightStepSize = 0.01f;

        m_generationParams.domainCenter = glm::vec3(0, 30, 0);
        m_generationParams.domainSize = glm::vec3(150, 10, 150);

        m_volumeParams.cloudAbsorption = 1.0f;
        m_volumeParams.lightAbsorption = 0.3f;
        m_volumeParams.densityMultiplier = 1.5f;

        m_volumeParams.scatteringG = 0.5f;
        m_volumeParams.phaseParams = glm::vec4(0.74f, 0.1f, 0.1f, 1.0f);
    }

    bool renderUI() {
        bool changed = false;

        ImGui::Begin("Volume", nullptr, ImGuiWindowFlags_AlwaysAutoResize);

        ImGui::SliderInt("Num steps", &m_volumeParams.numSteps, 0, 200);
        ImGui::SliderInt("Num light steps", &m_volumeParams.numLightSteps, 0, 100);

        ImGui::SliderFloat("Step size", &m_volumeParams.stepSize, 0.01f, 0.5f);
        ImGui::SliderFloat("Light step size", &m_volumeParams.lightStepSize, 0.01f, 0.5f);

        if(ImGui::SliderFloat3("Center", &m_generationParams.domainCenter.x, -10.0f, 10.0f)) changed = true;
        if(ImGui::SliderFloat3("Size", &m_generationParams.domainSize.x, 0.0f, 10.0f)) changed = true;

        ImGui::SliderFloat("Cloud absorption", &m_volumeParams.cloudAbsorption, 0.0f, 2.0f);
        ImGui::SliderFloat("Light absorption", &m_volumeParams.lightAbsorption, 0.0f, 2.0f);
        ImGui::SliderFloat("Density multiplier", &m_volumeParams.densityMultiplier, 0.0f, 4.0f);

        ImGui::SliderFloat("Scattering G", &m_volumeParams.scatteringG, -1.0f, 1.0f);
        ImGui::SliderFloat4("Phase params", &m_volumeParams.phaseParams.x, 0.0f, 1.0f);

        ImGui::End();

        return changed;
    }
};




#endif // CLOUD_MANAGER_HPP
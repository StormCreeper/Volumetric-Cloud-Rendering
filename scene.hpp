#ifndef SCENE_HPP
#define SCENE_HPP

#include "gl_includes.hpp"
#include "shader.hpp"


const int MAX_LIGHTS = 10;

struct Light {
    int type; // 0 = ambiant, 1 = point, 2 = directional
    glm::vec3 position;
    glm::vec3 color;
    float intensity;
};

class Scene {
public:
    Light m_lights[MAX_LIGHTS] {};
    int m_numLights = 0;
    

    std::vector<std::shared_ptr<Object3D>> m_objects {};
    
    Camera m_camera {};


    void setUniforms(GLuint lightingShader) {
        for(int i = 0; i < m_numLights; i++) {
            setUniform(lightingShader, std::string("u_lights[" + std::to_string(i) + "].type").c_str(), m_lights[i].type);
            setUniform(lightingShader, std::string("u_lights[" + std::to_string(i) + "].position").c_str(), m_lights[i].position);
            setUniform(lightingShader, std::string("u_lights[" + std::to_string(i) + "].color").c_str(), m_lights[i].color);
            setUniform(lightingShader, std::string("u_lights[" + std::to_string(i) + "].intensity").c_str(), m_lights[i].intensity);
        }

        setUniform(lightingShader, "u_numLights", m_numLights);
    }

    void setGeometryUniforms(GLuint geometryShader) {
        const glm::mat4 viewMatrix = m_camera.computeViewMatrix();
        const glm::mat4 projMatrix = m_camera.computeProjectionMatrix();
        
        setUniform(geometryShader, "u_viewMat", viewMatrix);
        setUniform(geometryShader, "u_projMat", projMatrix);
        setUniform(geometryShader, "u_proj_viewMat", projMatrix * viewMatrix);

        setUniform(geometryShader, "u_modelMat", glm::mat4(1.0f));
        setUniform(geometryShader, "u_transposeInverseModelMat", glm::mat4(1.0f));

        setUniform(geometryShader, "u_cameraPosition", m_camera.getPosition());

        setUniform(geometryShader, "u_time", static_cast<float>(glfwGetTime()));
    } 

    void init(int width, int height) {
        m_objects.push_back(std::make_shared<Object3D>(Mesh::genSphere(16)));
        m_objects.push_back(std::make_shared<Object3D>(Mesh::genSubdividedPlane(2)));

        m_objects[0]->setModelMatrix(glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -5.0f, 0.0f)));

        m_objects[1]->setModelMatrix(glm::scale(glm::mat4(1.0f), glm::vec3(40.0f, 10.0f, 40.0f)));
        m_objects[1]->setModelMatrix(glm::translate(m_objects[1]->getModelMatrix(), glm::vec3(0.0f, -1.0f, 0.0f)));

        m_lights[m_numLights++] = Light{
            2,
            glm::vec3(0.5f, 1.0f, 0.5f),
            glm::vec3(1.0, 1.0, 1.0),
            1.0f
        };

        initCamera(width, height);
    }

    void initCamera(int width, int height) {
        m_camera.setAspectRatio(static_cast<float>(width) / static_cast<float>(height));

        m_camera.setPosition(glm::vec3(0.0, 0.0, 3.0));
        m_camera.setNear(0.1);
        m_camera.setFar(200);

        m_camera.setFoV(90);
    }

    void geometryPass(GLuint geometryShader) {
        setGeometryUniforms(geometryShader);

        for(std::shared_ptr<Object3D> mesh : m_objects) {
            mesh->render(geometryShader);
        }
    }
};


#endif // SCENE_HPP
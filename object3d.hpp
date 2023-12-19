/*
    object3d.hpp
    author: Telo PHILIPPE

    A class to represent a 3D object, with a mesh, a texture and a model matrix.
*/

#ifndef OBJECT_3D_H
#define OBJECT_3D_H

#include "mesh.hpp"

class Object3D {
public:
    Object3D() {}
    Object3D(std::shared_ptr<Mesh> mesh) {
        this->mesh = mesh;
    }

    void render(GLuint program) const;

    void setModelMatrix(const glm::mat4 &modelMatrix) {
        this->modelMatrix = modelMatrix;
    }
    glm::mat4 getModelMatrix() const {
        return modelMatrix;
    }

private:
    std::shared_ptr<Mesh> mesh {};
    glm::mat4 modelMatrix = glm::mat4(1.0f);
};


#endif // OBJECT_3D_H
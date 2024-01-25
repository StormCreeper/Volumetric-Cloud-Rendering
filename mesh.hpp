/*
    mesh.hpp
    author: Telo PHILIPPE

    A Mesh class, to load and render meshes, with proper deletion.
    Also a function to generate a cube sphere mesh.

*/

#ifndef MESH_H
#define MESH_H

#include "gl_includes.hpp"

#include <memory>
#include <vector>

class Mesh {
public:
    void initGPUGeometry(const std::vector<float> &vertexPositions, const std::vector<float> &vertexNormals, const std::vector<float> &vertexUVs, const std::vector<unsigned int> &triangleIndices);
    void setGPUGeometry(GLuint posVbo, GLuint normalVbo, GLuint uvVbo, GLuint ibo, GLuint vao, size_t numIndices);
    void render() const;
    static std::shared_ptr<Mesh> genSphere(const size_t resolution = 16); // Should generate a unit sphere
    static std::shared_ptr<Mesh> genPlane(); // Should generate a unit plane
    static std::shared_ptr<Mesh> genSubdividedPlane(int resolution); // Should generate a unit plane
    
    void bind() const {
        glBindVertexArray(m_vao);
    }

    ~Mesh();
    
    GLuint m_vao = 0;

    size_t m_numIndices = 0;
    
private:
    GLuint m_posVbo = 0;
    GLuint m_normalVbo = 0;
    GLuint m_uvVbo = 0;
    GLuint m_ibo = 0;

    static void genFace(std::vector<float>& vertexPositions, std::vector<float>& vertexNormals, std::vector<float> &vertexUVs, std::vector<unsigned int>& triangleIndices, const size_t resolution, const glm::vec3& dir1, const glm::vec3& dir2);
};


#endif // MESH_H
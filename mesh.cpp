/*
    mesh.cpp
    author: Telo PHILIPPE

    Implementation of the Mesh class.
*/

#include "mesh.hpp"

#include <cmath>
#include <iostream>

void Mesh::initGPUGeometry(const std::vector<float> &vertexPositions, const std::vector<float> &vertexNormals, const std::vector<float> &vertexUVs, const std::vector<unsigned int> &triangleIndices) {
    // Create a single handle, vertex array object that contains attributes,
    // vertex buffer objects (e.g., vertex's position, normal, and color)
    glGenVertexArrays(1, &m_vao);  // If your system doesn't support OpenGL 4.5, you should use this instead of glCreateVertexArrays.

    glBindVertexArray(m_vao);

    // Generate a GPU buffer to store the positions of the vertices
    size_t vertexBufferSize = sizeof(float) * vertexPositions.size();  // Gather the size of the buffer from the CPU-side vector

    glGenBuffers(1, &m_posVbo);
    glBindBuffer(GL_ARRAY_BUFFER, m_posVbo);
    glBufferData(GL_ARRAY_BUFFER, vertexBufferSize, vertexPositions.data(), GL_DYNAMIC_READ);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), 0);
    glEnableVertexAttribArray(0);

    // Same for the normal vectors
    glGenBuffers(1, &m_normalVbo);
    glBindBuffer(GL_ARRAY_BUFFER, m_normalVbo);
    glBufferData(GL_ARRAY_BUFFER, vertexBufferSize, vertexNormals.data(), GL_DYNAMIC_READ);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), 0);
    glEnableVertexAttribArray(1);

    vertexBufferSize = sizeof(float) * vertexUVs.size();

    // Same for the uv coordinates
    glGenBuffers(1, &m_uvVbo);
    glBindBuffer(GL_ARRAY_BUFFER, m_uvVbo);
    glBufferData(GL_ARRAY_BUFFER, vertexBufferSize, vertexUVs.data(), GL_DYNAMIC_READ);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(GLfloat), 0);
    glEnableVertexAttribArray(2);

    // Same for an index buffer object that stores the list of indices of the
    // triangles forming the mesh
    size_t indexBufferSize = sizeof(unsigned int) * triangleIndices.size();
    glGenBuffers(1, &m_ibo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ibo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indexBufferSize, triangleIndices.data(), GL_DYNAMIC_READ);

    glBindVertexArray(0);  // deactivate the VAO for now, will be activated again when rendering

    m_numIndices = triangleIndices.size();
}

void Mesh::setGPUGeometry(GLuint posVbo, GLuint normalVbo, GLuint uvVbo, GLuint ibo, GLuint vao, size_t numIndices) {
    m_posVbo = posVbo;
    m_normalVbo = normalVbo;
    m_uvVbo = uvVbo;
    m_ibo = ibo;
    m_vao = vao;
    m_numIndices = numIndices;
}


void Mesh::render() const {
    glBindVertexArray(m_vao);                                                    // activate the VAO storing geometry data
    glDrawElements(GL_TRIANGLES, m_numIndices, GL_UNSIGNED_INT, 0);  // Call for rendering: stream the current GPU geometry through the current GPU program
    glBindVertexArray(0);                                                        // deactivate the VAO again 
}

/**
 * Generate a unit sphere centered at the origin,
 * by making a cube of the desired resolution and then
 * projecting the vertices onto the unit sphere.
 *
 * @param resolution The number of vertices along the longitude and latitude
 * @return A shared pointer to a mesh object
 */
std::shared_ptr<Mesh> Mesh::genSphere(const size_t resolution) {
    std::vector<float> vertexPositions{};
    std::vector<float> vertexNormals{};
    std::vector<float> vertexUVs{};
    std::vector<unsigned int> triangleIndices{};

    // Generate the 6 faces of the sphere
    genFace(vertexPositions, vertexNormals, vertexUVs, triangleIndices, resolution, glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f));
    genFace(vertexPositions, vertexNormals, vertexUVs, triangleIndices, resolution, glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f));
    genFace(vertexPositions, vertexNormals, vertexUVs, triangleIndices, resolution, glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f));
    genFace(vertexPositions, vertexNormals, vertexUVs, triangleIndices, resolution, glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(-1.0f, 0.0f, 0.0f));
    genFace(vertexPositions, vertexNormals, vertexUVs, triangleIndices, resolution, glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(-1.0f, 0.0f, 0.0f));
    genFace(vertexPositions, vertexNormals, vertexUVs, triangleIndices, resolution, glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(0.0f, -1.0f, 0.0f));

    // Create a mesh object
    std::shared_ptr<Mesh> mesh = std::make_shared<Mesh>();
    mesh->initGPUGeometry(vertexPositions, vertexNormals, vertexUVs, triangleIndices);

    return mesh;
}

/**
 * Generate a face of a unit sphere, centered at the origin
 * by making a grid of the desired resolution and then
 * projecting the vertices onto the unit sphere.
 *
 * @param vertexPositions The list of vertex positions
 * @param vertexNormals The list of vertex normals
 * @param triangleIndices The list of triangle indices
 * @param resolution The number of vertices along the longitude and latitude
 * @param dir1 The first direction of the face
 * @param dir2 The second direction of the face
 */
void Mesh::genFace(std::vector<float>& vertexPositions, std::vector<float>& vertexNormals, std::vector<float> &vertexUVs, std::vector<unsigned int>& triangleIndices, const size_t resolution, const glm::vec3& dir1, const glm::vec3& dir2) {
    const int baseIndex = vertexPositions.size() / 3;

    // Compute the normal of the face
    glm::vec3 faceNormal = glm::cross(dir1, dir2);

    for (size_t i = 0; i < resolution; ++i) {
        for (size_t j = 0; j < resolution; ++j) {
            // Compute the vertex position on the grid
            glm::vec3 pos = ((float)i / (float)(resolution - 1) * 2.0f - 1.0f) * dir1
                          + ((float)j / (float)(resolution - 1) * 2.0f - 1.0f) * dir2
                          + faceNormal;

            // Normalize the position to make it lie on the unit sphere
            pos = glm::normalize(pos);

            // Compute the vertex normal
            glm::vec3 normal = pos;

            // Compute the texture uv
            float longitude = atan2(pos.z, pos.x);
            float latitude = acos(pos.y);

            // Add the vertex to the list of positions
            vertexPositions.push_back(pos.x);
            vertexPositions.push_back(pos.y);
            vertexPositions.push_back(pos.z);

            // Add the vertex to the list of normals
            vertexNormals.push_back(normal.x);
            vertexNormals.push_back(normal.y);
            vertexNormals.push_back(normal.z);

            // Add texture UV
            vertexUVs.push_back(longitude / (2 * M_PI));
            vertexUVs.push_back(latitude / M_PI);
        }
    }
    // Generate a list of triangle indices
    for (size_t i = 0; i < resolution - 1; ++i) {
        for (size_t j = 0; j < resolution - 1; ++j) {
            // Compute the indices of the four vertices of the quad
            unsigned int v0 = i * resolution + j;
            unsigned int v1 = i * resolution + j + 1;
            unsigned int v2 = (i + 1) * resolution + j;
            unsigned int v3 = (i + 1) * resolution + j + 1;

            // Add the first triangle
            triangleIndices.push_back(v0 + baseIndex);
            triangleIndices.push_back(v2 + baseIndex);
            triangleIndices.push_back(v1 + baseIndex);

            // Add the second triangle
            triangleIndices.push_back(v1 + baseIndex);
            triangleIndices.push_back(v2 + baseIndex);
            triangleIndices.push_back(v3 + baseIndex);
        }
    }
}

std::shared_ptr<Mesh> Mesh::genPlane() {
    std::vector<float> vertexPositions{
        -1.0f, -1.0f, 0.0f, // 0
        -1.0f, 1.0f, 0.0f, // 1
        1.0f, 1.0f, 0.0f, // 2
        1.0f, -1.0f, 0.0f, // 3
    };
    std::vector<float> vertexNormals{
        0.0f, 1.0f, 0.0f, // 0
        0.0f, 1.0f, 0.0f, // 1
        0.0f, 1.0f, 0.0f, // 2
        0.0f, 1.0f, 0.0f, // 3
    };
    std::vector<float> vertexUVs{
        0.0f, 0.0f, // 0
        0.0f, 1.0f, // 1
        1.0f, 1.0f, // 2
        1.0f, 0.0f, // 3
    };
    std::vector<unsigned int> triangleIndices{
        0, 2, 1,
        0, 3, 2,
    };

    // Create a mesh object
    std::shared_ptr<Mesh> mesh = std::make_shared<Mesh>();
    mesh->initGPUGeometry(vertexPositions, vertexNormals, vertexUVs, triangleIndices);

    return mesh;
}

// Create a subdivided plane from a grid of resolution x resolution vertices
std::shared_ptr<Mesh> Mesh::genSubdividedPlane(int resolution) {
    std::vector<float> vertexPositions{};
    std::vector<float> vertexNormals{};
    std::vector<float> vertexUVs{};

    // Generate the vertices
    for (int i = 0; i < resolution; ++i) {
        for (int j = 0; j < resolution; ++j) {
            // Compute the vertex position on the grid
            glm::vec3 pos = ((float)i / (float)(resolution - 1) * 2.0f - 1.0f) * glm::vec3(1.0f, 0.0f, 0.0f)
                          + ((float)j / (float)(resolution - 1) * 2.0f - 1.0f) * glm::vec3(0.0f, 0.0f, 1.0f);

            // Compute the vertex normal
            glm::vec3 normal = glm::vec3(0.0f, 1.0f, 0.0f);

            // Compute the texture uv
            glm::vec2 uv = glm::vec2((float)i / (float)(resolution - 1), (float)j / (float)(resolution - 1));
            // Add the vertex to the list of positions
            vertexPositions.push_back(pos.x);
            vertexPositions.push_back(pos.y);
            vertexPositions.push_back(pos.z);

            // Add the vertex to the list of normals
            vertexNormals.push_back(normal.x);
            vertexNormals.push_back(normal.y);
            vertexNormals.push_back(normal.z);

            // Add texture UV
            vertexUVs.push_back(uv.x);
            vertexUVs.push_back(uv.y);
        }
    }

    std::vector<unsigned int> triangleIndices{};
    // Generate a list of triangle indices
    for (int i = 0; i < resolution - 1; ++i) {
        for (int j = 0; j < resolution - 1; ++j) {
            // Compute the indices of the four vertices of the quad
            unsigned int v0 = i * resolution + j;
            unsigned int v1 = i * resolution + j + 1;
            unsigned int v2 = (i + 1) * resolution + j;
            unsigned int v3 = (i + 1) * resolution + j + 1;

            // Add the first triangle
            triangleIndices.push_back(v0);
            triangleIndices.push_back(v1);
            triangleIndices.push_back(v2);

            // Add the second triangle
            triangleIndices.push_back(v1);
            triangleIndices.push_back(v3);
            triangleIndices.push_back(v2);
        }
    }

    // Create a mesh object
    std::shared_ptr<Mesh> mesh = std::make_shared<Mesh>();
    mesh->initGPUGeometry(vertexPositions, vertexNormals, vertexUVs, triangleIndices);

    return mesh;
}

Mesh::~Mesh() {
    if(m_posVbo) glDeleteBuffers(1, &m_posVbo);
    if(m_normalVbo) glDeleteBuffers(1, &m_normalVbo);
    if(m_uvVbo) glDeleteBuffers(1, &m_uvVbo);
    if(m_ibo) glDeleteBuffers(1, &m_ibo);
    
    if(m_vao) glDeleteVertexArrays(1, &m_vao);
}
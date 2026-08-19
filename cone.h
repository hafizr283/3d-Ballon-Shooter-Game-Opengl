#ifndef cone_h
#define cone_h

#include <glad/glad.h>
#include <vector>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "shader.h"

#define PI 3.1416f

using namespace std;

class Cone
{
public:
    glm::vec3 ambient;
    glm::vec3 diffuse;
    glm::vec3 specular;
    float shininess;
    float baseRadius;
    float height;
    int sectorCount;
    int stackCount;
    unsigned int coneVAO;

    Cone(float baseRadius = 1.0f, float height = 2.0f, int sectorCount = 36, int stackCount = 18, 
         glm::vec3 amb = glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3 diff = glm::vec3(1.0f, 0.0f, 0.0f), 
         glm::vec3 spec = glm::vec3(0.5f, 0.5f, 0.5f), float shiny = 32.0f) : verticesStride(32)
    {
        this->baseRadius = baseRadius;
        this->height = height;
        this->sectorCount = sectorCount;
        this->stackCount = stackCount;
        this->ambient = amb;
        this->diffuse = diff;
        this->specular = spec;
        this->shininess = shiny;

        buildCoordinatesAndIndices();
        buildVertices();

        glGenVertexArrays(1, &coneVAO);
        glBindVertexArray(coneVAO);

        unsigned int coneVBO;
        glGenBuffers(1, &coneVBO);
        glBindBuffer(GL_ARRAY_BUFFER, coneVBO);
        glBufferData(GL_ARRAY_BUFFER, getVertexSize(), getVertices(), GL_STATIC_DRAW);

        unsigned int coneEBO;
        glGenBuffers(1, &coneEBO);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, coneEBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, getIndexSize(), getIndices(), GL_STATIC_DRAW);

        glEnableVertexAttribArray(0);
        glEnableVertexAttribArray(1);
        glEnableVertexAttribArray(2);

        int stride = getVerticesStride();
        glVertexAttribPointer(0, 3, GL_FLOAT, false, stride, (void*)0);
        glVertexAttribPointer(1, 3, GL_FLOAT, false, stride, (void*)(sizeof(float) * 3));
        glVertexAttribPointer(2, 2, GL_FLOAT, false, stride, (void*)(sizeof(float) * 6));

        glBindVertexArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    }
    ~Cone() {}

    unsigned int getVertexCount() const { return (unsigned int)coordinates.size() / 3; }
    unsigned int getVertexSize() const { return (unsigned int)vertices.size() * sizeof(float); }
    int getVerticesStride() const { return verticesStride; }
    const float* getVertices() const { return vertices.data(); }
    unsigned int getIndexSize() const { return (unsigned int)indices.size() * sizeof(unsigned int); }
    const unsigned int* getIndices() const { return indices.data(); }
    unsigned int getIndexCount() const { return (unsigned int)indices.size(); }

    void drawCone(Shader& lightingShader, glm::mat4 model) const
    {
        lightingShader.use();
        lightingShader.setVec3("material.ambient", this->ambient);
        lightingShader.setVec3("material.diffuse", this->diffuse);
        lightingShader.setVec3("material.specular", this->specular);
        lightingShader.setFloat("material.shininess", this->shininess);
        lightingShader.setMat4("model", model);

        glBindVertexArray(coneVAO);
        glDrawElements(GL_TRIANGLES, getIndexCount(), GL_UNSIGNED_INT, (void*)0);
        glBindVertexArray(0);
    }

private:
    void buildCoordinatesAndIndices()
    {
        float sectorStep = 2 * PI / sectorCount;
        float stackStep = height / stackCount;
        
        // Base normal is roughly 0,1,0 but actually points outwards. 
        // Cone side normal has y component radius / sqrt(radius*radius + height*height)
        float ny = baseRadius / sqrt(baseRadius * baseRadius + height * height);
        float nxz = height / sqrt(baseRadius * baseRadius + height * height);

        // Sides
        for (int i = 0; i <= stackCount; ++i)
        {
            float stackY = -height/2.0f + i * stackStep;
            float currentRadius = baseRadius * (1.0f - (float)i / stackCount);
            
            for (int j = 0; j <= sectorCount; ++j)
            {
                float sectorAngle = j * sectorStep;

                float x = currentRadius * cosf(sectorAngle);
                float z = currentRadius * sinf(sectorAngle);

                coordinates.push_back(x);
                coordinates.push_back(stackY);
                coordinates.push_back(z);

                normals.push_back(cosf(sectorAngle) * nxz);
                normals.push_back(ny);
                normals.push_back(sinf(sectorAngle) * nxz);

                texCoords.push_back((float)j / sectorCount);
                texCoords.push_back((float)i / stackCount);
            }
        }

        int k1, k2;
        for (int i = 0; i < stackCount; ++i)
        {
            k1 = i * (sectorCount + 1);
            k2 = k1 + sectorCount + 1;

            for (int j = 0; j < sectorCount; ++j, ++k1, ++k2)
            {
                if (i != stackCount - 1)
                {
                    indices.push_back(k1);
                    indices.push_back(k2);
                    indices.push_back(k1 + 1);

                    indices.push_back(k1 + 1);
                    indices.push_back(k2);
                    indices.push_back(k2 + 1);
                }
                else
                {
                    indices.push_back(k1);
                    indices.push_back(k2);
                    indices.push_back(k1 + 1);
                }
            }
        }
        
        // Base
        int baseCenterIndex = (int)(coordinates.size() / 3);
        coordinates.push_back(0);
        coordinates.push_back(-height/2.0f);
        coordinates.push_back(0);
        normals.push_back(0);
        normals.push_back(-1);
        normals.push_back(0);
        texCoords.push_back(0.5f);
        texCoords.push_back(0.5f);
        
        int baseStartIndex = baseCenterIndex + 1;
        for (int j = 0; j <= sectorCount; ++j)
        {
            float sectorAngle = j * sectorStep;
            coordinates.push_back(baseRadius * cosf(sectorAngle));
            coordinates.push_back(-height/2.0f);
            coordinates.push_back(baseRadius * sinf(sectorAngle));
            normals.push_back(0);
            normals.push_back(-1);
            normals.push_back(0);
            texCoords.push_back(0.5f + 0.5f * cosf(sectorAngle));
            texCoords.push_back(0.5f + 0.5f * sinf(sectorAngle));
        }

        for (int j = 0; j < sectorCount; ++j)
        {
            indices.push_back(baseCenterIndex);
            indices.push_back(baseStartIndex + j + 1);
            indices.push_back(baseStartIndex + j);
        }
    }

    void buildVertices()
    {
        size_t count = coordinates.size();
        for (size_t i = 0, j = 0; i < count; i += 3, j += 2)
        {
            vertices.push_back(coordinates[i]);
            vertices.push_back(coordinates[i + 1]);
            vertices.push_back(coordinates[i + 2]);

            vertices.push_back(normals[i]);
            vertices.push_back(normals[i + 1]);
            vertices.push_back(normals[i + 2]);

            vertices.push_back(texCoords[j]);
            vertices.push_back(texCoords[j + 1]);
        }
    }

    vector<float> vertices;
    vector<float> normals;
    vector<float> texCoords;
    vector<unsigned int> indices;
    vector<float> coordinates;
    int verticesStride;
};

#endif

#include "Model.h"
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <iostream>
#include <limits>
#include <filesystem>

namespace Metaphysics {

bool Model::LoadFromFile(const std::string& path)
{
    namespace fs = std::filesystem;

    Assimp::Importer importer;
    const aiScene* scene = importer.ReadFile(path,
        aiProcess_Triangulate |
        aiProcess_GenNormals |
        aiProcess_FlipUVs |
        aiProcess_CalcTangentSpace);

    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
        std::cerr << "ERROR::ASSIMP::" << importer.GetErrorString() << std::endl;
        return false;
    }

    m_Path = path;
    const fs::path filePath(path);
    m_Directory = filePath.has_parent_path() ? filePath.parent_path().string() : std::string();
    m_Name = filePath.stem().empty() ? "Unnamed Model" : filePath.stem().string();

    ProcessNode(scene->mRootNode, (void*)scene);
    return true;
}

void Model::ProcessNode(void* nodePtr, void* scenePtr)
{
    aiNode* node = (aiNode*)nodePtr;
    const aiScene* scene = (const aiScene*)scenePtr;

    for (unsigned int i = 0; i < node->mNumMeshes; i++) {
        aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
        m_Meshes.push_back(ProcessMesh(mesh, (void*)scene));
    }

    for (unsigned int i = 0; i < node->mNumChildren; i++) {
        ProcessNode(node->mChildren[i], scenePtr);
    }
}

std::shared_ptr<Mesh> Model::ProcessMesh(void* meshPtr, void* scenePtr)
{
    aiMesh* mesh = (aiMesh*)meshPtr;
    const aiScene* scene = (const aiScene*)scenePtr;

    auto resultMesh = std::make_shared<Mesh>();

    for (unsigned int i = 0; i < mesh->mNumVertices; i++) {
        Vertex vertex;

        vertex.position = glm::vec3(
            mesh->mVertices[i].x,
            mesh->mVertices[i].y,
            mesh->mVertices[i].z
        );

        if (mesh->HasNormals()) {
            vertex.normal = glm::vec3(
                mesh->mNormals[i].x,
                mesh->mNormals[i].y,
                mesh->mNormals[i].z
            );
        } else {
            vertex.normal = glm::vec3(0.0f, 1.0f, 0.0f);
        }

        if (mesh->mTextureCoords[0]) {
            vertex.texCoords = glm::vec2(
                mesh->mTextureCoords[0][i].x,
                mesh->mTextureCoords[0][i].y
            );
        } else {
            vertex.texCoords = glm::vec2(0.0f, 0.0f);
        }

        resultMesh->vertices.push_back(vertex);
    }

    for (unsigned int i = 0; i < mesh->mNumFaces; i++) {
        aiFace face = mesh->mFaces[i];
        for (unsigned int j = 0; j < face.mNumIndices; j++) {
            resultMesh->indices.push_back(face.mIndices[j]);
        }
    }

    resultMesh->material = std::make_shared<Material>();
    if (mesh->mMaterialIndex >= 0) {
        aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];

        aiColor3D ambient(0.0f, 0.0f, 0.0f);
        aiColor3D diffuse(0.0f, 0.0f, 0.0f);
        aiColor3D specular(0.0f, 0.0f, 0.0f);
        float shininess = 0.0f;

        bool hasAmbient  = (material->Get(AI_MATKEY_COLOR_AMBIENT,  ambient)  == AI_SUCCESS);
        bool hasDiffuse  = (material->Get(AI_MATKEY_COLOR_DIFFUSE,  diffuse)  == AI_SUCCESS);
        bool hasSpecular = (material->Get(AI_MATKEY_COLOR_SPECULAR, specular) == AI_SUCCESS);
        bool hasShininess = (material->Get(AI_MATKEY_SHININESS, shininess) == AI_SUCCESS);

        glm::vec3 amb = hasAmbient  ? glm::vec3(ambient.r,  ambient.g,  ambient.b)  : glm::vec3(0.3f);
        glm::vec3 dif = hasDiffuse  ? glm::vec3(diffuse.r,  diffuse.g,  diffuse.b)  : glm::vec3(0.8f);
        glm::vec3 spc = hasSpecular ? glm::vec3(specular.r, specular.g, specular.b) : glm::vec3(0.5f);
        float shi = hasShininess ? shininess : 32.0f;

        auto isNearBlack = [](const glm::vec3& c) {
            return (c.r + c.g + c.b) < 0.05f;
        };
        if (isNearBlack(amb)) amb = glm::vec3(0.3f, 0.3f, 0.3f);
        if (isNearBlack(dif)) dif = glm::vec3(0.7f, 0.7f, 0.7f);

        if (shi < 1.0f) shi = 32.0f;

        resultMesh->material->SetAmbient(amb);
        resultMesh->material->SetDiffuse(dif);
        resultMesh->material->SetSpecular(spc);
        resultMesh->material->SetShininess(shi);
    }

    ComputeMeshAABB(resultMesh);

    return resultMesh;
}

void Model::ComputeMeshAABB(std::shared_ptr<Mesh> mesh)
{
    if (mesh->vertices.empty()) {
        mesh->aabbMin = glm::vec3(0.0f);
        mesh->aabbMax = glm::vec3(0.0f);
        return;
    }

    glm::vec3 minAABB(std::numeric_limits<float>::max());
    glm::vec3 maxAABB(std::numeric_limits<float>::lowest());

    for (const auto& vertex : mesh->vertices) {
        minAABB = glm::min(minAABB, vertex.position);
        maxAABB = glm::max(maxAABB, vertex.position);
    }

    mesh->aabbMin = minAABB;
    mesh->aabbMax = maxAABB;
}

void Model::ComputeAABB(glm::vec3& outMin, glm::vec3& outMax) const
{
    if (m_Meshes.empty()) {
        outMin = glm::vec3(0.0f);
        outMax = glm::vec3(0.0f);
        return;
    }

    glm::vec3 minAABB(std::numeric_limits<float>::max());
    glm::vec3 maxAABB(std::numeric_limits<float>::lowest());

    for (const auto& mesh : m_Meshes) {
        minAABB = glm::min(minAABB, mesh->aabbMin);
        maxAABB = glm::max(maxAABB, mesh->aabbMax);
    }

    outMin = minAABB;
    outMax = maxAABB;
}

} // namespace Metaphysics

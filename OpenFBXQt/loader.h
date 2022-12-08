#pragma once

#include "openfbxqt.h"
#include "model.h"
#include "datastorage.h"
#include "OpenFBX/src/ofbx.h"
#include <QString>

namespace ofbxqt
{

class Loader
{
public:
    Loader();

    QVector<std::shared_ptr<Model>> open(const QString& fileName, const OpenModelConfig config = OpenModelConfig(), QList<Note>* notes = nullptr);

private:
    void addNote(const Note::Type type, const QString& text);

    std::shared_ptr<Model> loadMesh(const ofbx::Mesh* mesh, const int meshIndex, const QString& absoluteDirectoryPath);
    void loadJoints(const ofbx::Skin* skin, ModelData& data, QHash<GLuint, QVector<QPair<GLuint, GLfloat>>>& resultJointsData /*QHash<index of vertex, QVector<QPair<joint index, joint weight>>>*/);
    void loadMaterial(const ofbx::Material* rawMaterial, std::shared_ptr<Material> material, const int meshIndex, const int materialIndex, const QString& absoluteDirectoryPath);
    std::shared_ptr<TextureInfo> loadTexture(const ofbx::Texture* rawTexture, const QString& absoluteDirectoryPath, const int meshIndex, const int materialIndex, ofbx::Texture::TextureType type);

    void addVertexAttributeGLfloat(ModelData& modelData, const QString& nameForShader, const int tupleSize);
    void convertAxisDirection(ModelData::AxisDirection& value, const int axis, const int sign);

    OpenModelConfig config;
    QList<Note>* notes = nullptr;

    ModelData::AxisDirection upDirection = ModelData::DefaultUpDirection;
    ModelData::AxisDirection forwardDirection = ModelData::DefaultForwardDirection;

    static DataStorage dataStorage;
};

}

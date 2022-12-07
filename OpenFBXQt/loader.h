#pragma once

#include "openfbxqt.h"
#include "model.h"
#include "modeldatastorage.h"
#include "OpenFBX/src/ofbx.h"
#include <QString>

namespace ofbxqt
{

class Loader
{
public:
    Loader();

    QList<Model*> open(const QString& fileName, const OpenModelConfig config = OpenModelConfig(), QList<Note>* notes = nullptr);

private:
    void addNote(const Note::Type type, const QString& text);

    Model* loadMesh(const ofbx::Mesh* mesh, const int meshIndex, const QString& absoluteDirectoryPath);
    void loadJoints(const ofbx::Skin* skin, ModelData& data, QHash<GLuint, QVector<QPair<GLuint, GLfloat>>>& resultJointsData /*QHash<index of vertex, QVector<QPair<joint index, joint weight>>>*/);
    Material* loadMaterial(const ofbx::Material* rawMaterial, const int meshIndex, const int materialIndex, const QString& absoluteDirectoryPath);
    bool loadImage(QImage& image, QString& resultFileName, const ofbx::Texture* texture, const QString& absoluteDirectoryPath, const int meshIndex, const int materialIndex, ofbx::Texture::TextureType type);

    void addVertexAttributeGLfloat(ModelData& modelData, const QString& nameForShader, const int tupleSize);

    OpenModelConfig config;
    QList<Note>* notes = nullptr;

    static ModelDataStorage dataStorage;
};

}

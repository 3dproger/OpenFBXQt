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
    static QList<Model*> open(const QString& fileName, QList<Note>& notes);

private:
    Loader(){}

    static void loadJoints(const ofbx::Skin* skin, ModelData& data,
                           QHash<GLuint, QVector<QPair<GLuint, GLfloat>>>& resultJointsData /*QHash<index of vertex, QVector<QPair<joint index, joint weight>>>*/,
                           QList<Note>& notes);

    static Model* loadMesh(const ofbx::Mesh* mesh, const int meshIndex, const QString& absoluteDirectoryPath, QList<Note>& notes);
    static Material* loadMaterial(const ofbx::Material* rawMaterial, const int meshIndex, const int materialIndex, const QString& absoluteDirectoryPath, QList<Note>& notes);

    static void addVertexAttributeGLfloat(ModelData& modelData, const QString& nameForShader, const int tupleSize);

    static ModelDataStorage dataStorage;
};

}

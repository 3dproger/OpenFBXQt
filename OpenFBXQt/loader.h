#pragma once

#include "model.h"
#include "modeldatastorage.h"
#include "OpenFBX/src/ofbx.h"
#include <QString>

namespace ofbxqt
{

class Note
{
public:
    enum class Type { Info, Warning, Error };

    Note(const Type type_, const QString& text_)
        : type(type_)
        , text(text_)
    {}

    Type getType() const
    {
        return type;
    }

    QString getText() const
    {
        return text;
    }

private:
    Type type;
    QString text;
};

class Loader
{
public:
    static QList<Model*> open(const QString& fileName, QList<Note>& notes);

private:
    Loader(){}

    static void loadJoints(const ofbx::Skin* skin, ModelData& data,
                           QHash<GLuint, QVector<QPair<GLuint, GLfloat>>>& resultJointsData /*QHash<index of vertex, QVector<QPair<joint index, joint weight>>>*/,
                           QList<Note>& notes);

    static Model* loadMesh(const ofbx::Mesh* mesh, QList<Note>& notes);

    static void addVertexAttributeGLfloat(ModelData& modelData, const QString& nameForShader, const int tupleSize);

    static ModelDataStorage dataStorage;
};

}

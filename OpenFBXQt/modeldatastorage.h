#pragma once

#include "armature.h"
#include "material.h"
#include <QString>
#include <QOpenGLBuffer>
#include <QOpenGLShaderProgram>
#include <QOpenGLTexture>

namespace ofbxqt
{

class Model;
class Loader;

struct VertexAttributeInfo
{
    QString nameForShader;
    static const GLenum type = GL_FLOAT;
    int offset = 0;
    int tupleSize = 0;
};

struct ModelData
{
    std::shared_ptr<Material> material; // TODO: check

    QMatrix4x4 sourceMatrix;

    const GLenum drawElementsMode = GL_TRIANGLES;

    const GLenum indexType = GL_UNSIGNED_INT;
    const int indexStride = (1) * sizeof(GLuint);
    int indexCount = 0;
    mutable QByteArray indexData;

    QVector<VertexAttributeInfo> vertexAttributes;
    int vertexStride = 0;
    int vertexCount = 0;
    mutable QByteArray vertexData;

    mutable QOpenGLBuffer vertexBuffer = QOpenGLBuffer(QOpenGLBuffer::VertexBuffer);
    mutable QOpenGLBuffer indexBuffer = QOpenGLBuffer(QOpenGLBuffer::IndexBuffer);

    mutable QOpenGLShaderProgram shader; // TODO: move to shaders storage

    Armature armature;
};

class ModelDataStorage
{
public:
    friend class Model;
    friend class Loader;
    friend class Scene;

private:
    ModelDataStorage(){}

    inline static QVector<std::shared_ptr<ModelData>> data = QVector<std::shared_ptr<ModelData>>(); // move to scene, make non static
};

}

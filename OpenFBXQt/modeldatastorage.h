#pragma once

#include "skeleton.h"
#include <QString>
#include <QList>
#include <QOpenGLBuffer>
#include <QOpenGLShaderProgram>
#include <QOpenGLTexture>

namespace ofbxqt
{

class Model;
class Loader;

struct Material
{
    enum class Type { Color, Image };
    Type type = Type::Color;

    virtual void init() = 0;
};

struct TextureMaterial : public Material
{
    TextureMaterial(const QString& imageFileName_)
        : imageFileName(imageFileName_)
    {}

    virtual void init() override
    {
        texture = new QOpenGLTexture(QImage(imageFileName).mirrored());
    }

    const QString imageFileName;
    mutable QOpenGLTexture* texture = nullptr;
};

struct ColorMaterial : public Material
{
    virtual void init() override {}

    QColor color = QColor(191, 191, 191);
};

struct VertexAttributeInfo
{
    QString nameForShader;
    static const GLenum type = GL_FLOAT;
    int offset = 0;
    int tupleSize = 0;
};

struct ModelData
{
    Material* material = nullptr;

    QMatrix4x4 sourceMatrix;

    GLenum drawElementsMode = GL_TRIANGLES;

    GLenum indexType = GL_UNSIGNED_INT;
    int indexStride = (1) * sizeof(GLuint);
    int indexCount = 0;
    mutable QByteArray indexData;

    QVector<VertexAttributeInfo> vertexAttributes;
    int vertexStride = 0;
    int vertexCount = 0;
    mutable QByteArray vertexData;

    mutable QOpenGLBuffer vertexBuffer = QOpenGLBuffer(QOpenGLBuffer::VertexBuffer);
    mutable QOpenGLBuffer indexBuffer = QOpenGLBuffer(QOpenGLBuffer::IndexBuffer);

    mutable QOpenGLShaderProgram shader;

    Skeleton skeleton;
};

class ModelDataStorage
{
public:
    friend class Model;
    friend class Loader;

    inline static const ColorMaterial defaultMaterial = ColorMaterial();

private:
    ModelDataStorage(){}

    inline static QList<ModelData*> data = QList<ModelData*>();
};

}

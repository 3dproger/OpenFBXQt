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

    Material(const Type type_)
        : type(type_)
    {}
    virtual ~Material(){}

    Type type = Type::Color;

    virtual void init() = 0;
};

struct TextureMaterial : public Material
{
    TextureMaterial(const QString& imageFileName_)
        : Material(Type::Image)
        , imageFileName(imageFileName_)
    {}

    virtual ~TextureMaterial() override
    {
        if (texture)
        {
            delete texture;
            texture = nullptr;
        }
    }

    virtual void init() override
    {
        const QImage image(imageFileName);
        if (image.isNull())
        {
            qCritical() << Q_FUNC_INFO << "failed to open image" << imageFileName;
            return;
        }

        texture = new QOpenGLTexture(image.mirrored());
    }

    const QString imageFileName;
    mutable QOpenGLTexture* texture = nullptr;
};

struct ColorMaterial : public Material
{
    ColorMaterial(const QColor& color_)
        : Material(Type::Color)
        , color(color_)
    {}

    virtual void init() override {}

    QColor color;
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

    mutable QOpenGLShaderProgram shader;

    Skeleton skeleton;
};

class ModelDataStorage
{
public:
    friend class Model;
    friend class Loader;

private:
    ModelDataStorage(){}

    inline static QList<ModelData*> data = QList<ModelData*>();
};

}

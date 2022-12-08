#pragma once

#include "armature.h"
#include "material.h"
#include <QString>
#include <QOpenGLBuffer>
#include <QOpenGLShaderProgram>
#include <QOpenGLTexture>
#include <map>

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
    QString name;

    std::shared_ptr<Material> material;
    std::shared_ptr<Armature> armature;

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

    enum class AxisDirection { XPlus, XMinus, YPlus, YMinus, ZPlus, ZMinus };
    static QString axisDirectionToString(const AxisDirection ad)
    {
        switch (ad)
        {
        case ofbxqt::ModelData::AxisDirection::XPlus: return "+X";
        case ofbxqt::ModelData::AxisDirection::XMinus: return "-X";
        case ofbxqt::ModelData::AxisDirection::YPlus: return "+Y";
        case ofbxqt::ModelData::AxisDirection::YMinus: return "-Y";
        case ofbxqt::ModelData::AxisDirection::ZPlus: return "+Z";
        case ofbxqt::ModelData::AxisDirection::ZMinus: return "-Z";
        }

        return "<UNKNOWN>";
    }
    static const ModelData::AxisDirection DefaultUpDirection = ModelData::AxisDirection::ZPlus;
    static const ModelData::AxisDirection DefaultForwardDirection = ModelData::AxisDirection::YPlus;

    AxisDirection upDirection = AxisDirection::YPlus;
    AxisDirection forwardDirection = AxisDirection::ZMinus;
};

class DataStorage
{
public:
    friend class Model;
    friend class Loader;
    friend class Scene;

private:
    DataStorage(){}

    // TODO: all data storages
    inline static std::map<QString, std::shared_ptr<TextureInfo>> textures; // <file name, texture>
    inline static QVector<std::shared_ptr<ModelData>> data = QVector<std::shared_ptr<ModelData>>(); // move to scene, make non static
};

}

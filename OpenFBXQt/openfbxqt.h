#pragma once

#include <QString>
#include <QQuaternion>
#include <QMatrix4x4>

namespace ofbxqt
{

static const char* VersionLib = "0.1";

struct OpenModelConfig
{
    bool loadTransform = true;
    bool loadArmature = true;

    bool loadMaterial = true;

    bool loadDiffuseTexture = true;
    bool loadDiffuseColor = true;

    bool loadNormalTexture = true;
};

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

class Transform
{
public:

    const QVector3D& getScale() const
    {
        return scale;
    }

    void setScale(const QVector3D& scale_)
    {
        scale = scale_;
        updateMatrix();
    }

    const QVector3D& getTanslation() const
    {
        return translation;
    }

    void setTranslation(const QVector3D& translation_)
    {
        translation = translation_;
        updateMatrix();
    }

    const QQuaternion& getRotation() const
    {
        return rotation;
    }

    void setRotation(const QQuaternion& rotation_)
    {
        rotation = rotation_;
        eulerAngles = rotation.toEulerAngles();
        updateMatrix();
    }

    void setEulerAngles(const QVector3D& eulerAngles_)
    {
        eulerAngles = eulerAngles_;
        rotation = QQuaternion::fromEulerAngles(eulerAngles);
        updateMatrix();
    }

    const QVector3D& getEulerAngles() const
    {
        return eulerAngles;
    }

    const QMatrix4x4& getMatrix() const
    {
        return matrix;
    }

private:
    void updateMatrix()
    {
        matrix = QMatrix4x4();
        matrix.scale(scale);
        matrix.rotate(rotation);
        matrix.translate(translation);
    }

    QMatrix4x4 matrix;

    QVector3D scale = QVector3D(1, 1, 1);
    QVector3D translation;

    QVector3D eulerAngles;
    QQuaternion rotation;
};


}

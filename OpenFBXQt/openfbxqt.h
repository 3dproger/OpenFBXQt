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

    friend Transform operator*(const Transform& a, const Transform& b);

    const QMatrix4x4& getResultMatrix() const
    {
        return resultMatrix;
    }

    const QVector3D& getScale() const
    {
        return scale;
    }

    void setScale(const QVector3D& scale_)
    {
        scale = scale_;
        updateResultMatrix();
    }

    const QVector3D& getTanslation() const
    {
        return translation;
    }

    void setTranslation(const QVector3D& translation_)
    {
        translation = translation_;
        updateResultMatrix();
    }

    const QQuaternion& getRotation() const
    {
        return rotation;
    }

    void setRotation(const QQuaternion& rotation_)
    {
        rotation = rotation_;
        eulerAngles = rotation.toEulerAngles();
        updateResultMatrix();
    }

    void setEulerAngles(const QVector3D& eulerAngles_)
    {
        eulerAngles = eulerAngles_;
        rotation = QQuaternion::fromEulerAngles(eulerAngles);
        updateResultMatrix();
    }

    const QVector3D& getEulerAngles() const
    {
        return eulerAngles;
    }

    const QVector3D& getRotationPivot() const
    {
        return rotationPivot;
    }

    void setRotationPivot(const QVector3D& rotationPivot_)
    {
        rotationPivot = rotationPivot_;
        updateResultMatrix();
    }

    const QVector3D& getScalePivot() const
    {
        return scalePivot;
    }

    void setScalePivot(const QVector3D& scalePivot_)
    {
        scalePivot = scalePivot_;
        updateResultMatrix();
    }

    const QMatrix4x4& getAdditionalMatrix() const
    {
        return additionalMatrix;
    }

    void setAdditionalMatrix(const QMatrix4x4& additionalMatrix_)
    {
        additionalMatrix = additionalMatrix_;
        updateResultMatrix();
    }

private:
    void updateResultMatrix()
    {
        resultMatrix = QMatrix4x4();

        resultMatrix.translate(translation);

        resultMatrix.translate(rotationPivot);
        resultMatrix.rotate(rotation);
        resultMatrix.translate(-rotationPivot);

        resultMatrix.translate(scalePivot);
        resultMatrix.scale(scale);
        resultMatrix.translate(-scalePivot);

        resultMatrix *= additionalMatrix;
    }

    QMatrix4x4 resultMatrix;

    QVector3D translation;

    QVector3D scale = QVector3D(1, 1, 1);
    QVector3D scalePivot;

    QVector3D eulerAngles;
    QQuaternion rotation;
    QVector3D rotationPivot;

    QMatrix4x4 additionalMatrix;
};

inline Transform operator*(const Transform& a, const Transform& b)
{
    Transform c;
    c.translation = a.translation + b.translation;
    c.scale = a.scale * b.scale;
    c.setRotation(a.getRotation() * b.getRotation());
    return c;
}

}

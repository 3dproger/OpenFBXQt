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

    enum class TransformOrder { TRS, };
    enum class RotationOrder { XYZ, };

    friend Transform operator*(const Transform& a, const Transform& b);

    Transform(const TransformOrder transformOrder_ = TransformOrder::TRS, const RotationOrder rotationOrder_ = RotationOrder::XYZ)
        : transformOrder(transformOrder_)
        , rotationOrder(rotationOrder_)
    {
    }

    const QMatrix4x4& getResultMatrix() const
    {
        if (needUpdateResultMatrix)
        {
            Transform* this_ = const_cast<Transform*>(this);
            this_->updateResultMatrix();
            this_->needUpdateResultMatrix = false;
        }

        return resultMatrix;
    }

    const QVector3D& getScale() const
    {
        return scale;
    }

    void setScale(const QVector3D& scale_)
    {
        scale = scale_;
        needUpdateResultMatrix = true;
    }

    const QVector3D& getTanslation() const
    {
        return translation;
    }

    void setTranslation(const QVector3D& translation_)
    {
        translation = translation_;
        needUpdateResultMatrix = true;
    }

    const QQuaternion& getRotation() const
    {
        return rotation;
    }

    void setRotation(const QQuaternion& rotation_)
    {
        rotation = rotation_;
        eulerAngles = rotation.toEulerAngles();
        needUpdateResultMatrix = true;
    }

    void setEulerAngles(const QVector3D& eulerAngles_)
    {
        eulerAngles = eulerAngles_;
        rotation = QQuaternion::fromEulerAngles(eulerAngles);
        needUpdateResultMatrix = true;
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
        needUpdateResultMatrix = true;
    }

    const QVector3D& getScalePivot() const
    {
        return scalePivot;
    }

    void setScalePivot(const QVector3D& scalePivot_)
    {
        scalePivot = scalePivot_;
        needUpdateResultMatrix = true;
    }

    const QMatrix4x4& getAdditionalMatrix() const
    {
        return additionalMatrix;
    }

    void setAdditionalMatrix(const QMatrix4x4& additionalMatrix_)
    {
        additionalMatrix = additionalMatrix_;
        needUpdateResultMatrix = true;
    }

    TransformOrder getTransformOrder() const
    {
        return transformOrder;
    }

    void setTransformOrder(const TransformOrder transformOrder_)
    {
        transformOrder = transformOrder_;
        needUpdateResultMatrix = true;
    }

    RotationOrder getRotationOrder() const
    {
        return rotationOrder;
    }

    void setRotationOrder(const RotationOrder rotationOrder_)
    {
        rotationOrder = rotationOrder_;
        needUpdateResultMatrix = true;
    }

private:
    void updateResultMatrix()
    {
        resultMatrix = additionalMatrix;

        resultMatrix.translate(translation);

        resultMatrix.translate(rotationPivot);
        resultMatrix.rotate(rotation);
        resultMatrix.translate(-rotationPivot);

        resultMatrix.translate(scalePivot);
        resultMatrix.scale(scale);
        resultMatrix.translate(-scalePivot);
    }

    bool needUpdateResultMatrix = false;

    QMatrix4x4 resultMatrix;

    QVector3D translation;

    QVector3D scale = QVector3D(1, 1, 1);
    QVector3D scalePivot;

    QVector3D eulerAngles;
    QQuaternion rotation;
    QVector3D rotationPivot;

    QMatrix4x4 additionalMatrix;

    TransformOrder transformOrder;
    RotationOrder rotationOrder;
};

}

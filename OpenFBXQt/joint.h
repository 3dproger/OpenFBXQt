#pragma once

#include <QString>
#include <QOpenGLFunctions>
#include <QMatrix4x4>

namespace ofbxqt
{

class Armature;

class Joint
{
public:
    friend class Loader;
    friend class Model;
    friend class Armature;

    std::weak_ptr<Armature> armature;
    std::weak_ptr<Joint> parent;
    QVector<std::shared_ptr<Joint>> children;

    const QString& getName() const { return name; }

    void setRotation(const QQuaternion& rotation);
    const QQuaternion& getRotation() const { return rotation; }

private:
    Joint(const QString& name, const GLuint index, const QMatrix4x4& inverseBindMatrix);
    void addChild(std::shared_ptr<Joint> joint);

    QString name;
    GLuint index = 0;
    QMatrix4x4 inverseBindMatrix;

    QQuaternion rotation;
    QMatrix4x4 localTransformation;
};

}

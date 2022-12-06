#pragma once

#include <QString>
#include <QOpenGLFunctions>
#include <QMatrix4x4>

namespace ofbxqt
{

class Model;
class Loader;

class Joint
{
public:
    friend class Loader;
    friend class Model;
    friend class Skeleton;

    const QString& getName() const { return name; }
    void setRotation(const QQuaternion& rotation);

private:
    Joint(const QString& name, const GLuint index, const QMatrix4x4& inverseBindMatrix);
    void addChild(Joint* joint);

    QString name;
    GLuint index = 0;
    QMatrix4x4 inverseBindMatrix;

    QQuaternion rotation;
    QMatrix4x4 localTransformation;

    Joint* parent = nullptr;
    QHash<QString, Joint*> children;
};

}

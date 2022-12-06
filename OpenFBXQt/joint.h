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

    Joint(const QString& name, const GLuint index, const QMatrix4x4& inverseBindMatrix);

    const QString& getName() const { return name; }

private:
    void addChild(Joint* joint);

    QString name;
    GLuint index = 0;
    QMatrix4x4 inverseBindMatrix;

    QQuaternion rotation;
    QMatrix4x4 localTransofrmation;

    QHash<QString, Joint*> children;
};

}

#pragma once

#include <QString>
#include <QOpenGLFunctions>
#include <QMatrix4x4>

namespace ofbxqt
{

class Joint
{
public:
    friend class Loader;
    friend class Model;
    friend class Armature;

    const QString& getName() const { return name; }

    const QVector<std::shared_ptr<Joint>>& getChildren() const { return children; }

    void setRotation(const QQuaternion& rotation);

private:
    Joint(const QString& name, const GLuint index, const QMatrix4x4& inverseBindMatrix);
    void addChild(std::shared_ptr<Joint> joint);

    QString name;
    GLuint index = 0;
    QMatrix4x4 inverseBindMatrix;

    QQuaternion rotation;
    QMatrix4x4 localTransformation;

    std::weak_ptr<Joint> parent;
    QVector<std::shared_ptr<Joint>> children;
};

}

#pragma once

#include "joint.h"

namespace ofbxqt
{

class Loader;
class Model;

class Skeleton
{
public:
    friend class Loader;
    friend class Model;

    Skeleton();
    ~Skeleton();

    void update();

    const QVector<Joint*>& getJoints() const;
    Joint* getRootJoint();
    Joint* getJointByName(const QString& name);

private:
    void update(Joint* joint, const QMatrix4x4& parentMatrix = QMatrix4x4());

    Joint* rootJoint = nullptr;
    QVector<QMatrix4x4> jointsResultMatrices;
    QHash<QString, Joint*> jointsByName;
    QVector<Joint*> joints;
};

}

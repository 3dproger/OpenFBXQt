#pragma once

#include "joint.h"

namespace ofbxqt
{

class Loader;

class Skeleton
{
public:
    friend class Loader;

    Skeleton();
    ~Skeleton();

    void update();

    const QVector<QMatrix4x4>& getJointsResultMatrices() const;

private:
    void update(Joint* joint, const QMatrix4x4& parentMatrix = QMatrix4x4());

    Joint* rootJoint = nullptr;
    QVector<QMatrix4x4> jointsResultMatrices;
    QHash<QString, Joint*> jointsByName;
    QVector<Joint*> joints;
};

}

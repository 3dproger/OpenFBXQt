#include "skeleton.h"

namespace ofbxqt
{

Skeleton::Skeleton()
{

}

Skeleton::~Skeleton()
{
    for (Joint* joint : qAsConst(joints))
    {
        delete joint;
    }
    joints.clear();
}

void Skeleton::update()
{
    if (!rootJoint)
    {
        return;
    }

    update(rootJoint);
}

const QVector<Joint*> &Skeleton::getJoints() const
{
    return joints;
}

Joint *Skeleton::getRootJoint()
{
    return rootJoint;
}

Joint *Skeleton::getJointByName(const QString &name)
{
    return jointsByName.value(name);
}

void Skeleton::update(Joint *joint, const QMatrix4x4 &parentMatrix)
{
    if (!joint)
    {
        qWarning() << Q_FUNC_INFO << "joint is null";
        return;
    }

    const QMatrix4x4 matrix = parentMatrix * joint->inverseBindMatrix.inverted() * joint->localTransformation * joint->inverseBindMatrix;

    jointsResultMatrices[joint->index] = matrix;

    for (Joint* child : qAsConst(joint->children))
    {
        update(child, matrix);
    }
}

}

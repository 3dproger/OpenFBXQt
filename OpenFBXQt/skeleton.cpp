#include "skeleton.h"

namespace ofbxqt
{

void Skeleton::update()
{
    if (!rootJoint)
    {
        return;
    }

    update(rootJoint);
}

const QVector<std::shared_ptr<Joint>> &Skeleton::getJoints() const
{
    return joints;
}

std::shared_ptr<Joint> Skeleton::getRootJoint()
{
    return rootJoint;
}

std::shared_ptr<Joint> Skeleton::getJointByName(const QString &name)
{
    const int index = jointsByName.value(name, -1);
    if (index == -1)
    {
        return nullptr;
    }

    if (index >= joints.count())
    {
        qCritical() << Q_FUNC_INFO << "index out of bound";
        return nullptr;
    }

    return joints[index];
}

void Skeleton::update(std::shared_ptr<Joint> joint, const QMatrix4x4 &parentMatrix)
{
    if (!joint)
    {
        qWarning() << Q_FUNC_INFO << "joint is null";
        return;
    }

    const QMatrix4x4 matrix = parentMatrix * joint->inverseBindMatrix.inverted() * joint->localTransformation * joint->inverseBindMatrix;

    jointsResultMatrices[joint->index] = matrix;

    for (std::shared_ptr<Joint> child : qAsConst(joint->children))
    {
        update(child, matrix);
    }
}

}

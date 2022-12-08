#include "armature.h"

namespace ofbxqt
{

void Armature::update()
{
    for (const std::shared_ptr<Joint>& joint : qAsConst(topLevelJoints))
    {
         update(joint);
    }
}

std::shared_ptr<Joint> Armature::getJointByName(const QString &name)
{
    const int index = jointsByName.value(name, -1);
    if (index == -1)
    {
        return nullptr;
    }

    if (index >= allJoints.count())
    {
        qCritical() << Q_FUNC_INFO << "index out of bound";
        return nullptr;
    }

    return allJoints[index];
}

void Armature::update(std::shared_ptr<Joint> joint, const QMatrix4x4 &parentMatrix)
{
    if (!joint)
    {
        qWarning() << Q_FUNC_INFO << "joint is null";
        return;
    }

    const QMatrix4x4 matrix = parentMatrix * joint->inverseBindMatrix.inverted() * joint->localTransformation * joint->inverseBindMatrix;

    jointsResultMatrices[joint->index] = matrix;

    for (const std::shared_ptr<Joint> &child : qAsConst(joint->children))
    {
        update(child, matrix);
    }
}

}

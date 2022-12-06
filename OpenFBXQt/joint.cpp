#include "joint.h"
#include "model.h"

namespace ofbxqt
{

Joint::Joint(const QString& name_, const GLuint index_, const QMatrix4x4 &inverseBindMatrix_)
    : name(name_)
    , index(index_)
    , inverseBindMatrix(inverseBindMatrix_)
{

}

void Joint::addChild(Joint *joint)
{
    if (!joint)
    {
        qCritical() << "child joint is nullptr";
        return;
    }

    if (joint->name.isEmpty())
    {
        qWarning() << "child name is empty";
    }

    children.insert(joint->name, joint);
}

}

#include "joint.h"
#include "model.h"

namespace ofbxqt
{

void Joint::setTransform(const Transform &transform_)
{
    transform = transform_;

    localTransformation = transform.getMatrix();
}

Joint::Joint(const QString& name_, const GLuint index_, const QMatrix4x4 &inverseBindMatrix_)
    : name(name_)
    , index(index_)
    , inverseBindMatrix(inverseBindMatrix_)
{

}

void Joint::addChild(std::shared_ptr<Joint> joint)
{
    if (!joint)
    {
        qCritical() << "child joint is null";
        return;
    }

    children.append(joint);
}

}

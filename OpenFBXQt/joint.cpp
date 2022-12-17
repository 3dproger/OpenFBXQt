#include "joint.h"
#include "model.h"

namespace ofbxqt
{

void Joint::setTransform(const Transform &transform_)
{
    transform = transform_;
}

Joint::Joint(const QString& name_, const GLuint index_, const QMatrix4x4 &sourceMatrix_)
    : name(name_)
    , index(index_)
    , sourceMatrix(sourceMatrix_)
{

}

}

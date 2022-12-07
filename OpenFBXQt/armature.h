#pragma once

#include "joint.h"

namespace ofbxqt
{

class Model;

class Armature
{
public:
    friend class Loader;
    friend class Model;

    std::weak_ptr<Model> model;

    void update();

    const QVector<std::shared_ptr<Joint>>& getJoints() const;
    std::shared_ptr<Joint> getRootJoint();
    std::shared_ptr<Joint> getJointByName(const QString& name);

private:
    void update(std::shared_ptr<Joint>, const QMatrix4x4& parentMatrix = QMatrix4x4());

    std::shared_ptr<Joint> rootJoint;
    QVector<QMatrix4x4> jointsResultMatrices;
    QHash<QString, int> jointsByName; // <name, index>
    QVector<std::shared_ptr<Joint>> joints;
};

}

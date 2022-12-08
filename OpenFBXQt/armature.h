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

    const QVector<std::shared_ptr<Joint>>& getTopLevelJoints() const { return topLevelJoints; }
    const QVector<std::shared_ptr<Joint>>& getAllJoints() const { return allJoints; }
    std::shared_ptr<Joint> getJointByName(const QString& name);

private:
    void update(std::shared_ptr<Joint>, const QMatrix4x4& parentMatrix = QMatrix4x4());

    QVector<QMatrix4x4> jointsResultMatrices;
    QHash<QString, int> jointsByName; // <name, index>
    QVector<std::shared_ptr<Joint>> topLevelJoints;
    QVector<std::shared_ptr<Joint>> allJoints;
};

}

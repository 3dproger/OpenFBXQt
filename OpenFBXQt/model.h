#pragma once

#include "openfbxqt.h"
#include "armature.h"
#include "datastorage.h"
#include <QColor>
#include <QOpenGLFunctions>

namespace ofbxqt
{

class Model : protected QOpenGLFunctions
{
public:
    std::shared_ptr<Armature> armature;
    std::shared_ptr<Material> material;

    std::weak_ptr<Model> parent;
    QVector<std::shared_ptr<Model>> children;

    friend class Loader;

    Model(std::shared_ptr<ModelData> data);

    void initializeGL();
    void paintGL(const QMatrix4x4& projection);

    QString getName() const;
    void setTransform(const Transform& transform);
    const Transform& getTransform() const;

private:
    void updateChildrenTransform(const Transform& transform);

    bool initializedGL = false;

    Transform parentTransform;
    Transform transform;
    std::shared_ptr<ModelData> data;
};

}

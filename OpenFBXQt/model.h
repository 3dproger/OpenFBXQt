#pragma once

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
    std::vector<std::shared_ptr<Model>> children;

    friend class Loader;

    Model(std::shared_ptr<ModelData> data);

    void initializeGL();
    void paintGL(const QMatrix4x4& projection);

    QString getName() const;

private:
    bool initializedGL = false;

    std::shared_ptr<ModelData> data;
    QMatrix4x4 matrix;
};

}

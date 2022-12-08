#pragma once

#include "armature.h"
#include "modeldatastorage.h"
#include <QColor>
#include <QOpenGLFunctions>

namespace ofbxqt
{

class Model : protected QOpenGLFunctions
{
public:
    std::shared_ptr<Armature> armature;
    std::shared_ptr<Material> material;

    friend class Loader;

    Model(std::shared_ptr<ModelData> data);

    void initializeGL();
    void paintGL(const QMatrix4x4& projection);

private:
    bool initializedGL = false;

    std::shared_ptr<ModelData> data;
    QMatrix4x4 matrix;

    bool needUpdateArmature = false;
};

}

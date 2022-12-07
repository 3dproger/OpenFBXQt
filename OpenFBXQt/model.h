#pragma once

#include "armature.h"
#include "modeldatastorage.h"
#include <QColor>
#include <QOpenGLFunctions>

namespace ofbxqt
{

class Loader;

class Model : protected QOpenGLFunctions
{
public:
    Armature armature;

    friend class Loader;

    Model(std::shared_ptr<ModelData> data);

    void initializeGL();
    void paintGL(const QMatrix4x4& projection);

private:
    bool initializedGL = false;

    std::shared_ptr<Material> material;

    std::shared_ptr<ModelData> data;
    QMatrix4x4 matrix;

    bool needUpdateArmature = false;
};

}

#pragma once

#include "skeleton.h"
#include "modeldatastorage.h"
#include <QColor>
#include <QOpenGLFunctions>

namespace ofbxqt
{

class Loader;

class Model : protected QOpenGLFunctions
{
public:
    friend class Loader;

    Model(ModelData& data);
    ~Model();

    void initializeGL();
    void paintGL(const QMatrix4x4& projectionMatrix);

private:
    bool initializedGL = false;

    const Material& getMaterial();

    const ModelData& data;
    QMatrix4x4 matrix;

    bool needUpdateSkeleton = false;
    Skeleton skeleton;
};

}

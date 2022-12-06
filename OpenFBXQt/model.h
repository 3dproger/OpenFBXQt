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
    Skeleton skeleton;

    friend class Loader;

    Model(ModelData& data);
    ~Model();

    void initializeGL();
    void paintGL(const QMatrix4x4& projection);

    void setMaterial(Material* material);

private:
    TextureMaterial* findTexture(const QString& fileName);

    bool initializedGL = false;

    Material* material = nullptr;

    const ModelData& data;
    QMatrix4x4 matrix;

    bool needUpdateSkeleton = false;
};

}

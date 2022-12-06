#pragma once

#include "model.h"
#include <QOpenGLFunctions>
#include <QColor>

namespace ofbxqt
{

class Scene : protected QOpenGLFunctions
{
public:
    Scene();
    ~Scene();

    void initializeGL();
    void resizeGL(int width, int height);

    void setMaxFps(qreal maxFps);
    qreal getMaxFps() const;

    void setProjection(const QMatrix4x4& matrix);
    QMatrix4x4 getProjection() const;

    void addModel(Model* model);

    void paintGL();

private:

    bool initializedGL = false;

    qreal maxFps = 60;

    QColor backgroundColor = QColor(64, 64, 64);
    QList<Model*> models;
    QMatrix4x4 perspective;
    QMatrix4x4 projection;
};

}

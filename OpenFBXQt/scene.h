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

    const qreal nearDistance = 0.1;
    const qreal farDistance = 10000.0;
    const qreal viewingAngle = 60.0;

    qreal maxFps = 60;

    QColor backgroundColor = QColor(64, 64, 64);
    QList<Model*> models;
    QMatrix4x4 perspective;
    QMatrix4x4 projection;
};

}

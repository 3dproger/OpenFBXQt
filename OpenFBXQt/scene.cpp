#include "scene.h"
#include <QtMath>

namespace ofbxqt
{

Scene::Scene()
{

}

Scene::~Scene()
{
    for (Model* model : qAsConst(models))
    {
        delete model;
    }
    models.clear();
}

void Scene::initializeGL()
{
    if (initializedGL)
    {
        return;
    }

    initializedGL = true;

    initializeOpenGLFunctions();

    for (Model* model : qAsConst(models))
    {
        model->initializeGL();
    }

    glClearColor(backgroundColor.redF(), backgroundColor.greenF(), backgroundColor.blueF(), 1.0);
}

void Scene::paintGL()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    for (Model* model : qAsConst(models))
    {
        model->paintGL(perspective * projection);
    }
}

void Scene::resizeGL(int width, int height)
{
    const qreal zNear = 0.1;
    const qreal zFar = 10000.0;
    const qreal viewingAngle = 60.0;

    const qreal aspect = qreal(width) / qreal(height ? height : 1);

    perspective = QMatrix4x4();
    perspective.setToIdentity();
    perspective.perspective(viewingAngle, aspect, zNear, zFar);
}

void Scene::setProjection(const QMatrix4x4 &matrix)
{
    projection = matrix;
}

QMatrix4x4 Scene::getProjection() const
{
    return projection;
}

void Scene::addModel(Model *model)
{
    if (!model)
    {
        qCritical() << Q_FUNC_INFO << "model is null";
        return;
    }

    if (initializedGL)
    {
        model->initializeGL();
    }

    models.append(model);
}

void ofbxqt::Scene::setMaxFps(qreal maxFps_)
{
    maxFps = maxFps_;

    int interval = qFloor(1000.0 / maxFps);
    if (interval <= 0)
    {
        interval = 1;
    }
}

qreal Scene::getMaxFps() const
{
    return maxFps;
}

}

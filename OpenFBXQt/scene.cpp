#include "scene.h"
#include <QtMath>
#include <algorithm>
#include <random>

namespace ofbxqt
{

Scene::Scene(std::function<void()> onNeedUpdateCallback_)
    : onNeedUpdateCallback(onNeedUpdateCallback_)
{
    resizeGL(100, 100);
}

Scene::~Scene()
{
    clear();
}

void Scene::initializeGL()
{
    if (initializedGL)
    {
        return;
    }

    initializedGL = true;

    initializeOpenGLFunctions();
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);

    for (std::shared_ptr<Model> model : qAsConst(models))
    {
        model->initializeGL();
    }

    glClearColor(backgroundColor.redF(), backgroundColor.greenF(), backgroundColor.blueF(), 1.0);
}

void Scene::paintGL()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    for (std::shared_ptr<Model> model : qAsConst(models))
    {
        model->paintGL(perspective * projection);
    }
}

void Scene::resizeGL(int width, int height)
{
    const qreal aspect = qreal(width) / qreal(height ? height : 1);

    perspective = QMatrix4x4();
    perspective.setToIdentity();
    perspective.perspective(viewingAngle, aspect, nearDistance, farDistance);

    if (onNeedUpdateCallback)
    {
        onNeedUpdateCallback();
    }
}

void Scene::setProjection(const QMatrix4x4 &matrix)
{
    projection = matrix;

    if (onNeedUpdateCallback)
    {
        onNeedUpdateCallback();
    }
}

QMatrix4x4 Scene::getProjection() const
{
    return projection;
}

void Scene::addModel(std::shared_ptr<Model> model)
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

QVector<std::shared_ptr<Model>> Scene::open(const QString &fileName, const OpenModelConfig config, QList<ofbxqt::Note>* notes)
{
    QVector<std::shared_ptr<Model>> models = Loader().open(fileName, config, notes);

    for (std::shared_ptr<Model> model : qAsConst(models))
    {
        addModel(model);
    }

    if (onNeedUpdateCallback)
    {
        onNeedUpdateCallback();
    }

    return models;
}

void Scene::clear()
{
    models.clear();
    DataStorage::data.clear();

    if (onNeedUpdateCallback)
    {
        onNeedUpdateCallback();
    }
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

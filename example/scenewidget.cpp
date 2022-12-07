#include "scenewidget.h"
#include <QMouseEvent>

SceneWidget::SceneWidget(QWidget *parent)
    : BaseSceneWidget{parent}
{
    updateProjection();
}

void SceneWidget::resetCamera()
{
    camera = Camera();
    updateProjection();
}

void SceneWidget::mousePressEvent(QMouseEvent *event)
{
    if (!event)
    {
        qCritical() << Q_FUNC_INFO << "event is null";
        return;
    }

    if (!event->buttons().testFlag(Qt::LeftButton))
    {
        return;
    }

    setCursor(Qt::ClosedHandCursor);

    prevMousePos = event->globalPos();
}

void SceneWidget::mouseReleaseEvent(QMouseEvent *event)
{
    if (!event)
    {
        qCritical() << Q_FUNC_INFO << "event is null";
        return;
    }

    setCursor(Qt::ArrowCursor);
}

void SceneWidget::mouseMoveEvent(QMouseEvent *event)
{
    if (!event)
    {
        qCritical() << Q_FUNC_INFO << "event is null";
        return;
    }

    if (event->buttons().testFlag(Qt::MouseButton::LeftButton))
    {
        const float deltaX = (event->globalPos().x() - prevMousePos.x()) * mouseRotationSensitivity.x();
        camera.yaw += deltaX;

        const float deltaY = (event->globalPos().y() - prevMousePos.y()) * mouseRotationSensitivity.y();
        camera.tilt += deltaY;

        if (camera.tilt > 0)
        {
            camera.tilt = 0;
        }

        if (camera.tilt < -180)
        {
            camera.tilt = -180;
        }

        prevMousePos = event->globalPos();

        updateProjection();
    }
}

void SceneWidget::wheelEvent(QWheelEvent *event)
{
    if (!event)
    {
        qCritical() << Q_FUNC_INFO << "event is null";
        return;
    }

    camera.zoom += (camera.zoom * event->angleDelta().y() / 1000.0) * mouseWheelZoomSensitivity;

    if (camera.zoom < camera.minZoom)
    {
        camera.zoom = camera.minZoom;
    }

    if (camera.zoom > camera.maxZoom)
    {
        camera.zoom = camera.maxZoom;
    }

    updateProjection();
}

void SceneWidget::updateProjection()
{
    QMatrix4x4 matrix;
    matrix.translate(0.0, 0.0, -22);
    matrix.rotate(camera.tilt, QVector3D(1, 0, 0));
    matrix.rotate(camera.yaw, QVector3D(0, 0, 1));
    matrix.scale(camera.zoom);

    scene.setProjection(matrix);
}

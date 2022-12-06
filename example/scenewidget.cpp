#include "scenewidget.h"
#include <QMouseEvent>

SceneWidget::SceneWidget(QWidget *parent)
    : BaseSceneWidget{parent}
{
    updateProjection();
}

void SceneWidget::mousePressEvent(QMouseEvent *event)
{
    if (!event)
    {
        qCritical() << Q_FUNC_INFO << "event is null";
        return;
    }

    prevMousePos = event->globalPos();
}

void SceneWidget::mouseReleaseEvent(QMouseEvent *event)
{
    if (!event)
    {
        qCritical() << Q_FUNC_INFO << "event is null";
        return;
    }
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
        cameraYaw += deltaX;

        const float deltaY = (event->globalPos().y() - prevMousePos.y()) * mouseRotationSensitivity.y();
        cameraTilt += deltaY;

        if (cameraTilt > 0)
        {
            cameraTilt = 0;
        }

        if (cameraTilt < -180)
        {
            cameraTilt = -180;
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

    zoom += (zoom * event->angleDelta().y() / 1000.0) * mouseWheelZoomSensitivity;

    if (zoom < minZoom)
    {
        zoom = minZoom;
    }

    if (zoom > maxZoom)
    {
        zoom = maxZoom;
    }

    updateProjection();
}

void SceneWidget::updateProjection()
{
    QMatrix4x4 matrix;
    matrix.translate(0.0, 0.0, -22);
    matrix.rotate(cameraTilt, QVector3D(1, 0, 0));
    matrix.rotate(cameraYaw, QVector3D(0, 0, 1));
    matrix.scale(zoom);

    scene.setProjection(matrix);

    update();
}

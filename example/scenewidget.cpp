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

    if (!event->buttons().testFlag(Qt::LeftButton) && !event->buttons().testFlag(Qt::RightButton))
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

        if (camera.tilt > 90)
        {
            camera.tilt = 90;
        }

        if (camera.tilt < -90)
        {
            camera.tilt = -90;
        }

        updateProjection();
    }
    else if (event->buttons().testFlag(Qt::MouseButton::RightButton))
    {
        const QVector2D mouseDelta(QVector2D(event->globalPos() - prevMousePos) * mouseTranslationSensitivity);
        const QVector3D delta(-mouseDelta.x(), 0, mouseDelta.y());

        camera.translation += delta / camera.zoom;

        updateProjection();
    }

    prevMousePos = event->globalPos();
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
    QVector3D eye = QVector3D(0, -1000 / camera.zoom, 0);
    QVector3D center = QVector3D(0, 0, 0);

    QMatrix4x4 rotationMatrix;
    rotationMatrix.rotate(camera.tilt, QVector3D(1, 0, 0));
    rotationMatrix.rotate(camera.yaw, QVector3D(0, 0, 1));
    eye = eye * rotationMatrix;

    QVector3D translation = camera.translation;
    translation = translation * rotationMatrix;

    eye += translation;
    center += translation;

    QVector3D up(0, 0, 1);

    if (camera.tilt == 90)
    {
        up = QVector3D(0, 1, 0);
        QMatrix4x4 matrix;
        matrix.rotate(camera.yaw, QVector3D(0, 0, 1));
        up = up * matrix;
    }

    if (camera.tilt == -90)
    {
        up = QVector3D(0, -1, 0);
        QMatrix4x4 matrix;
        matrix.rotate(camera.yaw, QVector3D(0, 0, 1));
        up = up * matrix;
    }

    QMatrix4x4 matrix;
    matrix.lookAt(eye, center, up);

    scene.setProjection(matrix);
}

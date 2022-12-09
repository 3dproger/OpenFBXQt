#pragma once

#include "basescenewidget.h"

class SceneWidget : public ofbxqt::BaseSceneWidget
{
    Q_OBJECT
public:
    explicit SceneWidget(QWidget *parent = nullptr);
    void resetCamera();

signals:

protected:
    void mousePressEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void wheelEvent(QWheelEvent *event) override;

private:
    void updateProjection();

    struct Camera
    {
        qreal yaw = 0;
        qreal tilt = -60;
        QVector3D translation = QVector3D(0.0, 0.0, -22);

        static constexpr qreal minZoom = 0.01;
        static constexpr qreal maxZoom = 100.0;
        qreal zoom = 1.0;
    };

    Camera camera;

    QPoint prevMousePos;

    QVector2D mouseTranslationSensitivity = QVector2D(0.05, 0.05);
    QVector2D mouseRotationSensitivity = QVector2D(0.25, 0.25);
    qreal mouseWheelZoomSensitivity = 1.0;
};

#pragma once

#include "basescenewidget.h"

class SceneWidget : public ofbxqt::BaseSceneWidget
{
    Q_OBJECT
public:
    explicit SceneWidget(QWidget *parent = nullptr);

signals:

protected:
    void mousePressEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void wheelEvent(QWheelEvent *event) override;

private:
    void updateProjection();

    QPoint prevMousePos;
    QVector2D mouseRotationSensitivity = QVector2D(0.5, 0.5);

    qreal cameraYaw = 0;
    qreal cameraTilt = -60;

    const qreal minZoom = 0.01;
    const qreal maxZoom = 100.0;
    qreal mouseWheelZoomSensitivity = 1.0;
    qreal zoom = 1.0;
};

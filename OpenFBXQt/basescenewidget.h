#pragma once

#include "scene.h"
#include <QOpenGLWidget>
#include <QColor>

namespace ofbxqt
{

class BaseSceneWidget : public QOpenGLWidget
{
    Q_OBJECT
public:
    Scene scene;

    explicit BaseSceneWidget(QWidget *parent = nullptr);

signals:

protected:
    void initializeGL() override;
    void resizeGL(int width, int height) override;
    void paintGL() override;

};

}

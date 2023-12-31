#include "basescenewidget.h"

namespace ofbxqt
{

BaseSceneWidget::BaseSceneWidget(QWidget *parent)
    : QOpenGLWidget{parent}
    , scene([this]()
    {
        update();
    })
{

}

void BaseSceneWidget::initializeGL()
{
    scene.initializeGL();
}

void BaseSceneWidget::resizeGL(int width, int height)
{
    scene.resizeGL(width, height);
}

void BaseSceneWidget::paintGL()
{
    scene.paintGL();
}

}

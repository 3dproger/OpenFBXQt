#pragma once

#include "model.h"
#include "loader.h"
#include <QOpenGLFunctions>
#include <QColor>

namespace ofbxqt
{

class Scene : protected QOpenGLFunctions
{
public:
    Scene(std::function<void()> onNeedUpdateCallback);
    ~Scene();

    void initializeGL();
    void resizeGL(int width, int height);

    void setMaxFps(qreal maxFps);
    qreal getMaxFps() const;

    void setProjection(const QMatrix4x4& matrix);
    QMatrix4x4 getProjection() const;

    QVector<std::shared_ptr<Model>> open(const QString& fileName, const OpenModelConfig config = OpenModelConfig(), QList<ofbxqt::Note>* notes = nullptr);
    void clear();

    const QVector<std::shared_ptr<Model>>& getTopLevelModels() const { return topLevelModels; }

    void paintGL();

private:
    void addModel(std::shared_ptr<Model> model);

    bool initializedGL = false;

    std::function<void()> onNeedUpdateCallback = nullptr;

    const qreal nearDistance = 0.1;
    const qreal farDistance = 10000.0;
    const qreal viewingAngle = 60.0;

    qreal maxFps = 60;

    QColor backgroundColor = QColor(64, 64, 64);
    QVector<std::shared_ptr<Model>> topLevelModels;
    QMatrix4x4 perspective;
    QMatrix4x4 projection;
};

}

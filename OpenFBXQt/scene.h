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

    void setProjection(const QMatrix4x4& matrix);
    QMatrix4x4 getProjection() const;

    FileInfo open(const QString& fileName, const OpenModelConfig config = OpenModelConfig());
    void clear();

    const QVector<std::shared_ptr<Model>>& getTopLevelModels() const { return topLevelModels; }
    const QVector<std::shared_ptr<ofbxqt::FileInfo>>& getFiles() { return files; }

    void paintGL();

private:
    void addModel(std::shared_ptr<Model> model);

    bool initializedGL = false;

    std::function<void()> onNeedUpdateCallback = nullptr;

    const qreal nearDistance = 0.1;
    const qreal farDistance = 100000.0;
    const qreal viewingAngle = 60.0;

    qreal maxFps = 60;

    QColor backgroundColor = QColor(64, 64, 64);
    QVector<std::shared_ptr<ofbxqt::FileInfo>> files;
    QVector<std::shared_ptr<Model>> topLevelModels;
    QMatrix4x4 perspective;
    QMatrix4x4 projection;
};

}

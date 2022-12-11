#pragma once

#include <QString>
#include <QQuaternion>

namespace ofbxqt
{

static const char* VersionLib = "0.1";

struct OpenModelConfig
{
    bool loadTransform = true;
    bool loadArmature = true;

    bool loadMaterial = true;

    bool loadDiffuseTexture = true;
    bool loadDiffuseColor = true;

    bool loadNormalTexture = true;
};

class Note
{
public:
    enum class Type { Info, Warning, Error };

    Note(const Type type_, const QString& text_)
        : type(type_)
        , text(text_)
    {}

    Type getType() const
    {
        return type;
    }

    QString getText() const
    {
        return text;
    }

private:
    Type type;
    QString text;
};

struct Transform
{
    QVector3D scale = QVector3D(1, 1, 1);
    QQuaternion rotation;
    QVector3D translation;
};


}

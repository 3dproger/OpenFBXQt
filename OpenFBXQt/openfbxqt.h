#pragma once

#include <QString>

namespace ofbxqt
{

struct OpenModelConfig
{
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

}

#pragma once

#include <QString>

namespace ofbxqt
{

struct OpenModelConfig
{
    bool openMaterial = true;
    bool openMaterialTexture = true;
    bool openMaterialColor = true;
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

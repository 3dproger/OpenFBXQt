#pragma once

#include <QImage>
#include <QOpenGLTexture>
#include <memory>

namespace ofbxqt
{

class TextureInfo
{
public:
    friend class Model;
    TextureInfo(const QImage& image, const QString& fileName);

    void initializeGL();

private:
    const QImage image;
    const QString fileName;

    std::shared_ptr<QOpenGLTexture> texture;
};

class Material
{
public:
    friend class Loader;
    friend class Model;

    void initializeGL();

private:
    std::unique_ptr<QColor> diffuseColor;
    std::shared_ptr<TextureInfo> diffuseTexture;

    std::shared_ptr<TextureInfo> normalTexture;
};

}

#include "material.h"

namespace ofbxqt
{

TextureInfo::TextureInfo(const QImage &image_, const QString &fileName_)
    : image(image_)
    , fileName(fileName_)
{

}

void TextureInfo::initializeGL()
{
    if (texture)
    {
        return;
    }

    texture = std::shared_ptr<QOpenGLTexture>(new QOpenGLTexture(image.mirrored()));
    image = QImage();
}

void Material::initializeGL()
{
    if (diffuseTexture)
    {
        diffuseTexture->initializeGL();
    }

    if (normalTexture)
    {
        normalTexture->initializeGL();
    }
}

}

#include "model.h"

namespace ofbxqt
{

Model::Model(ModelData& data_)
    : skeleton(data_.skeleton)
    , data(data_)
{

}

Model::~Model()
{

}

void Model::initializeGL()
{
    if (initializedGL)
    {
        return;
    }

    initializedGL = true;

    initializeOpenGLFunctions();

    if (!data.vertexBuffer.isCreated())
    {
        if (!data.vertexBuffer.create())
        {
            qCritical() << Q_FUNC_INFO << "failed to create vertex buffer";
        }

        if (!data.vertexBuffer.bind())
        {
            qCritical() << Q_FUNC_INFO << "failed to bind vertex buffer";
        }

        if (data.vertexData.isEmpty())
        {
            qCritical() << Q_FUNC_INFO << "vertex data is empty";
        }

        if (data.vertexCount <= 0)
        {
            qCritical() << Q_FUNC_INFO << "vertex count is" << data.vertexCount;
        }

        if (data.vertexStride <= 0)
        {
            qCritical() << Q_FUNC_INFO << "vertex stride is" << data.vertexStride;
        }

        data.vertexBuffer.allocate(data.vertexData, data.vertexCount * data.vertexStride);

        if (data.vertexBuffer.size() <= 0)
        {
            qWarning() << Q_FUNC_INFO << "vertex buffer is empty";
        }

        data.vertexBuffer.release();

        data.vertexData.clear();
    }

    if (!data.indexBuffer.isCreated())
    {
        if (!data.indexBuffer.create())
        {
            qCritical() << Q_FUNC_INFO << "failed to create index buffer";
        }

        if (!data.indexBuffer.bind())
        {
            qCritical() << Q_FUNC_INFO << "failed to bind index buffer";
        }

        if (data.indexData.isEmpty())
        {
            qCritical() << Q_FUNC_INFO << "index data is empty";
        }

        if (data.indexCount <= 0)
        {
            qCritical() << Q_FUNC_INFO << "index count is" << data.indexCount;
        }

        if (data.indexStride <= 0)
        {
            qCritical() << Q_FUNC_INFO << "index stride is" << data.indexStride;
        }

        data.indexBuffer.allocate(data.indexData, data.indexCount * data.indexStride);

        if (data.indexBuffer.size() <= 0)
        {
            qWarning() << Q_FUNC_INFO << "index buffer is empty";
        }

        data.indexBuffer.release();

        data.indexData.clear();
    }

    if (!data.shader.isLinked())
    {
        QString vshaderFileName;
        if (data.skeleton.getJoints().count() > 0)
        {
            vshaderFileName = ":/OpenFBXQt-shaders/vshader-with-joins.glsl";
        }
        else
        {
            vshaderFileName = ":/OpenFBXQt-shaders/vshader-no-joints.glsl";
        }

        QString fshaderFileName;
        switch (getMaterial().type)
        {
        case Material::Type::Image:
            fshaderFileName = ":/OpenFBXQt-shaders/fshader-textured.glsl";
            break;
        case Material::Type::Color:
            fshaderFileName = ":/OpenFBXQt-shaders/fshader-colored.glsl";
            break;
        }

        if (!data.shader.addShaderFromSourceFile(QOpenGLShader::Vertex, vshaderFileName))
        {
            qWarning() << Q_FUNC_INFO << "failed to compile vertex shader";
        }

        if (!data.shader.addShaderFromSourceFile(QOpenGLShader::Fragment, fshaderFileName))
        {
            qWarning() << Q_FUNC_INFO << "failed to compile fragment shader";
        }

        if (!data.shader.link())
        {
            qWarning() << Q_FUNC_INFO << "failed to link shader";
        }
    }
}

void Model::paintGL(const QMatrix4x4 &projection)
{
    if (!data.shader.isLinked())
    {
#ifdef QT_DEBUG
        qCritical() << Q_FUNC_INFO << "shader not linked";
#endif
        return;
    }

    const Material& material = getMaterial();

    const TextureMaterial* textureMaterial = nullptr;
    const ColorMaterial* colorMaterial = nullptr;

    switch(material.type)
    {
    case ofbxqt::Material::Type::Image:
        textureMaterial = static_cast<const TextureMaterial*>(&material);
        if (textureMaterial->texture)
        {
            textureMaterial->texture->bind();
        }
        else
        {
#ifdef QT_DEBUG
            qCritical() << Q_FUNC_INFO << "texture is null";
#endif
        }
        break;
    case ofbxqt::Material::Type::Color:
        colorMaterial = static_cast<const ColorMaterial*>(&material);
        break;
    }

    if (!data.shader.bind())
    {
#ifdef QT_DEBUG
        qCritical() << Q_FUNC_INFO << "failed to bind shader";
#endif
    }

    QVector3D v(0, 0, 0);
    v = v.unproject(matrix * data.sourceMatrix, projection, QRect(0, 0, 1, 1));

    data.shader.setUniformValue("projection_pos", v);
    data.shader.setUniformValue("model_projection_matrix", projection * matrix * data.sourceMatrix);
    if (material.type == Material::Type::Image)
    {
        data.shader.setUniformValue("texture", 0);
    }
    else if (material.type == Material::Type::Color)
    {
        if (colorMaterial)
        {
            data.shader.setUniformValue("u_color", colorMaterial->color);
        }
        else
        {
#ifdef QT_DEBUG
            qCritical() << Q_FUNC_INFO << "color material is null";
#endif
        }
    }

    if (needUpdateSkeleton)
    {
        skeleton.update();
        needUpdateSkeleton = false;
    }

    const QVector<QMatrix4x4>& matrices = skeleton.jointsResultMatrices;
    if (matrices.count() > 0)
    {
        data.shader.setUniformValueArray("joints", matrices.data(), matrices.count());
    }

    data.vertexBuffer.bind();
    data.indexBuffer.bind();

    for (const VertexAttributeInfo& attribute : qAsConst(data.vertexAttributes))
    {
        const int location = data.shader.attributeLocation(attribute.nameForShader);
        if (location == -1)
        {
#ifdef QT_DEBUG
            qWarning() << Q_FUNC_INFO << "name" << attribute.nameForShader << "not valid for this shader";
#endif
            continue;
        }

        data.shader.enableAttributeArray(location);
        data.shader.setAttributeBuffer(location, attribute.type, attribute.offset, attribute.tupleSize, data.vertexStride);
    }

    glDrawElements(data.drawElementsMode, data.indexCount, data.indexType, nullptr);

    data.shader.release();

    if (material.type == Material::Type::Image && textureMaterial && textureMaterial->texture)
    {
        textureMaterial->texture->release();
    }

    data.vertexBuffer.release();
    data.indexBuffer.release();
}

const Material &Model::getMaterial()
{
    if (data.material)
    {
        return *data.material;
    }

    return ModelDataStorage::defaultMaterial;
}

}

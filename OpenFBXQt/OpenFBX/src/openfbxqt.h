#ifndef OPENFBXQT_H
#define OPENFBXQT_H

#include <QString>
#include <QVector>
#include <QHash>
#include <QMatrix4x4>
#include <QOpenGLFunctions>

class OpenFBXQt
{
public:
    struct VertexAttributeInfo
    {
        QString nameForShader;
        static const GLenum type = GL_FLOAT;
        int offset = 0;
        int tupleSize = 0;
    };

    struct Model;

    struct Joint
    {
    public:
        Joint(const QString name, const GLuint index, const QMatrix4x4& inverseBindMatrix, Model* model)
            : _name(name)
            , _index(index)
            , _inverseBindMatrix(inverseBindMatrix)
            , _model(model)
        {
            if (_name.isEmpty())
            {
                qWarning() << "joint name is empty";
            }

            if (!model)
            {
                qWarning() << "model is nullptr";
            }
        }

        QString name() const { return _name; }
        GLuint index() const { return _index; }
        QMatrix4x4 inverseBindMatrix() const { return _inverseBindMatrix; }
        QMatrix4x4 localTransofrmation() const { return _localTransofrmation; }

        void setRotation(const QQuaternion& rotation)
        {
            _localTransofrmation.rotate(_rotation.inverted());
            _rotation = rotation;
            _localTransofrmation.rotate(_rotation);

            if (_model)
            {
                _model->needRecalculateJoints = true;
            }
            else
            {
                qWarning() << "model is nullptr";
                return;
            }
        }

        void addChild(Joint* joint)
        {
            if (!joint)
            {
                qCritical() << "child joint is nullptr";
                return;
            }

            if (joint->name().isEmpty())
            {
                qWarning() << "child name is empty";
            }

            _children.insert(joint->name(), joint);
        }

        const QHash<QString, Joint*>& children()
        {
            return _children;
        }

    private:
        QString _name;
        GLuint _index = 0;
        QMatrix4x4 _inverseBindMatrix;

        Model* _model = nullptr;

        QQuaternion _rotation;
        QMatrix4x4 _localTransofrmation;

        QHash<QString, Joint*> _children;
    };

    struct Model
    {
        ~Model();

        QVector<VertexAttributeInfo> vertexAttributes;

        int vertexStride = 0;
        int vertexCount = 0;
        QByteArray vertexData;

        static const GLenum mode = GL_TRIANGLES;

        static const GLenum indexType = GL_UNSIGNED_INT;
        static const int indexStride = (1) * sizeof(GLuint);
        int indexCount = 0;
        QByteArray indexData;

        Joint* rootJoint = nullptr;
        QVector<QMatrix4x4> jointsResultMatrices;
        QHash<QString, Joint*> jointsByName;
        QVector<Joint*> joints;//ToDo: добавить удаление
        void updateJointsResultMatrices(Joint* joint, const QMatrix4x4& parentMatrix = QMatrix4x4());

        template<typename ValueType> void addVertexAttribute(const QString& nameForShader, const int tupleSize);

        bool needRecalculateJoints = false;
    };

    static Model* load(const QString& fileName);

private:
    OpenFBXQt(){}
};

#endif // OPENFBXQT_H

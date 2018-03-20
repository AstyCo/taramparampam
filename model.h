#ifndef MODEL_H
#define MODEL_H

/**
\file 3DS.h
\brief The 3ds model loader

All models are loaded and renderd using this class
*/

#include "lib3ds_qt_global.h"

#include <lib3ds/file.h>
#include <lib3ds/node.h>
#include <lib3ds/mesh.h>
#include <lib3ds/vector.h>
#include <lib3ds/matrix.h>
#include <lib3ds/material.h>
#include <lib3ds/light.h>

#include <string>
#include <vector>
#include <GL/gl.h>

#include <QVector>
#include <QMap>
#include <QString>
#include <QVector3D>

#include <QPoint>

using namespace std;

namespace lib3ds_qt {

struct Mesh
{
    int _textureID;
    QVector<GLfloat> _vertices;
    QVector<GLushort> _indices;
    QVector<GLfloat> _normals;
    QVector<GLfloat> _textureVertices;

    Mesh() : _textureID(-1) {}
};

struct LightSource
{
    GLuint lightID;
    QVector3D position;

    LightSource(GLuint id = -1, const QVector3D &pos = QVector3D())
        : lightID(id), position(pos) {}
};

/// This class can load a model, and then apply a texture on it to finally render it
class LIB3DS_QTSHARED_EXPORT Model
{
public:
    Model();
    ~Model(); /// RAII -> free file, free textures

    /// It loads the file 'name', sets the current frame to 0 and if the model has textures, it will be applied to the model
    void loadFile(const QString &name, const QString &pathToFile = QString());

    void prepareNodes();
    void prepareNode(Lib3dsNode *node);
    void renderModel();
    void renderMesh(const Mesh &mesh);
    /// It applies a texture to mesh ,according to the data that mesh contains
    void ApplyTexture(Lib3dsMesh *mesh, const QString &extraPath = QString());
    Lib3dsFile * get3DSPointer();
    const QString &getFilename() const;

    bool isValid() const;
    bool isValidRadius() const;
    double meshRadius() const;

    QVector3D getMin() const;
    QVector3D getMax() const;
    void centerModel();

    void enableLightSources();
    void disableLightSources();

    void loadStaticMaterials();

    void updateLightSource(GLuint lightID, const QVector3D &newPosition);
private:
    Lib3dsFile *_file3ds; /**< file holds the data of the model */
    QString _fileName; /**< It's the filename of the model */
    QMap<QString, GLuint> _textureFilenamesIndexes;
    typedef QMap<QString, GLuint>::iterator MapIterator;

    QList<Mesh> _meshes;
    QList<Lib3dsNode*> _nodes;
    QList<LightSource> _lightSources;
    double _meshRadius;
    bool _isValid;
};

} // namespace lib3ds_qt

#endif // MODEL_H

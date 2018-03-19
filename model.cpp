/*
Program name:Combat of Death
Description source file: this class loads 3ds models
    Copyright (C) 2005  Hylke Donker

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA
*/

/**
\file 3DS.cpp
\brief The cpp file of 3DS.h

This is the place were all the functions are 'defined'
*/

#include "model.h"

#include <QImage>
#include <QGLWidget>
#include <qmath.h>

#include <QDebug>

using namespace lib3ds_qt;

static void do_light_adjust(QImage *image, int factor)
{
    if (image == NULL || image->isNull() || factor == 0)
        return;

    int bytes_per_pixel = image->bytesPerLine() / image->width();
    uchar *pixel = NULL;
    QRgb *rgba;

    if (factor > 0) { // lighter
        factor += 100;
        for (int h = 0; h < image->height(); h++) {
            pixel = image->scanLine(h);
            for (int w = 0; w < image->width(); w++) {
                rgba = (QRgb *)pixel;
                if (qAlpha(*rgba) != 0 && (qRed(*rgba) != 0 || qGreen(*rgba) != 0 || qBlue(*rgba) != 0))
                    *rgba = QColor::fromRgba(*rgba).lighter(factor).rgba();
                pixel += bytes_per_pixel;
            }
        }
    } else { // darker
        factor = -factor;
        factor += 100;
        for (int h = 0; h < image->height(); h++) {
            uchar *pixel = image->scanLine(h);
            for (int w = 0; w < image->width(); w++) {
                rgba = (QRgb *)pixel;
                if (qAlpha(*rgba) != 0 && (qRed(*rgba) != 0 || qGreen(*rgba) != 0 || qBlue(*rgba) != 0))
                    *rgba = QColor::fromRgba(*rgba).darker(factor).rgba();
                pixel += bytes_per_pixel;
            }
        }
    }
}

static GLuint generateTextureStatic(const QString &name, int lightFactor)
{
    glEnable(GL_TEXTURE_2D);
    QImage tempImage(name);
    if (lightFactor != 0)
        do_light_adjust(&tempImage, lightFactor);
    if (tempImage.isNull()) {
        Q_ASSERT(false);
        return GLuint(-1);
    }
    QImage image = QGLWidget::convertToGLFormat(tempImage);

    GLuint result;
    GL_CHECK(glGenTextures(1, &result));

    // "Bind" the newly created texture : all future texture functions will modify this texture
    GL_CHECK(glBindTexture(GL_TEXTURE_2D, result));

    // Give the image to OpenGL
    GL_CHECK(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, (GLsizei)image.width(), (GLsizei)image.height(),
                          0, GL_RGBA, GL_UNSIGNED_BYTE, image.bits()));

    GL_CHECK( glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT));
    GL_CHECK( glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT));
    GL_CHECK( glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
    GL_CHECK( glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
    GL_CHECK( glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE));

    glDisable(GL_TEXTURE_2D);
    return result;
}


// constructor, enables and set properties of texture coordinate generation and set the current frame
Model::Model()
{
    _isValid = false;
    _meshRadius = -1;
}

// destructor, free up memory and disable texture generation
Model::~Model()
{
    if(_file3ds) // if the file isn't freed yet
        lib3ds_file_free(_file3ds); //free up memory
     //disable texture generation
    MapIterator it(_textureFilenamesIndexes.begin());
    while (it != _textureFilenamesIndexes.end()) {
        glDeleteTextures(1, &it.value());
    }
}

// load the model, and if the texture has textures, then apply them on the geometric primitives
void Model::loadFile(const char *name)
{
    _fileName = name;
    // load file
    _file3ds = lib3ds_file_load(_fileName.toLatin1().constData());
    if(!_file3ds) // if we were not able to load the file
    {
        // give some errors
        QString event = "Error loading: ";
        QString online = "On line 61 in file ";
        online.append(__FILE__);
        event.append(_fileName);
        qDebug() << event << endl;
        qDebug() << online << endl;
        Q_ASSERT(false);
    }
    lib3ds_file_eval(_file3ds, 0); // set current frame to 0
    // apply texture to all meshes that have texels
    Lib3dsMesh *mesh;
    for(mesh = _file3ds->meshes;mesh != 0;mesh = mesh->next)
    {
        if(mesh->texels) //if there's texels for the mesh
            ApplyTexture(mesh); //then apply texture to it
    }
}

void Model::prepareNodes()
{
    _nodes.clear();
    _meshes.clear();

    for(Lib3dsNode *node = _file3ds->nodes; node != 0; node = node->next) // Render all nodes
        prepareNode(node);

    centerModel();
    _isValid = true;
}

void Model::prepareNode(Lib3dsNode *node)
{
    if (!node)
        return;
    for(Lib3dsNode *childNode = node->childs; childNode != 0; childNode = childNode->next)
        prepareNode(childNode); //prepare all child nodes of this note

    Lib3dsMesh *mesh = lib3ds_file_mesh_by_name(_file3ds, node->name); //get all the meshes of the current node
    if(! mesh)
        return;

    _meshes.push_back(Mesh());
    _nodes << node;
    Mesh &meshData = _meshes.last();

    meshData._vertices.reserve(3 * mesh->points); // optimization
    meshData._normals.reserve(3 * mesh->points); // optimization


    Lib3dsVector normals[3 *mesh->faces]; //alocate memory for our normals
    lib3ds_mesh_calculate_normals(mesh, normals); // calculate the normals of the mesh
    for (unsigned i = 0; i < mesh->points; ++i) {
        const Lib3dsVector &triangleVertice =mesh->pointL[i].pos;
        const Lib3dsVector &normal = normals[i];


        meshData._vertices << triangleVertice[0];
        meshData._vertices << triangleVertice[1];
        meshData._vertices << triangleVertice[2];

        meshData._normals << normal[0];
        meshData._normals << normal[1];
        meshData._normals << normal[2];

        _meshRadius = qMax(_meshRadius, length3dsVector(triangleVertice));
    }
//    free(normals); // free up memory

    for(unsigned p = 0;p < mesh->faces;p++)
    {
        Lib3dsFace *f = &mesh->faceL[p];
        Q_ASSERT(f);
        Lib3dsMaterial *mat = lib3ds_file_material_by_name(_file3ds, f->material);
        bool isTextureValid = mat && mesh->texels;
        QString textureName;
        if (isTextureValid)
            textureName= mat->texture1_map.name;

        if(isTextureValid)
        {
            Q_ASSERT(_textureFilenamesIndexes.contains(textureName));
            meshData._textureID = _textureFilenamesIndexes[textureName];
        }

        for(int i = 0;i < 3;i++)
        {
            if(isTextureValid) {
                meshData._textureVertices << mesh->texelL[f->points[i]][0];
                meshData._textureVertices << mesh->texelL[f->points[i]][1];
            }
            meshData._indices << f->points[i];
        }
    }
    glEnd();
    glEndList(); // end of list

}

void Model::initGL()
{
    static GLfloat diff[4] = { 0.75, 0.75, 0.75, 1.0 }; // color: white/grey
    static GLfloat amb[4] = { 0.25, 0.25, 0.25, 1.0 }; //color: black/dark gray
    static GLfloat spec[4] = { 0.0, 0.0, 0.0, 1.0 }; //color: completly black
    glMaterialfv(GL_FRONT, GL_DIFFUSE, diff);
    glMaterialfv(GL_FRONT, GL_AMBIENT, amb);
    glMaterialfv(GL_FRONT, GL_AMBIENT, spec);
}

// what is basicly does is, set the properties of the texture for our mesh
void Model::ApplyTexture(Lib3dsMesh *mesh)
{
    for(unsigned int i = 0;i < mesh->faces;i++)
    {
        Lib3dsFace *f = &mesh->faceL[i];
        if(! f->material[0])
            continue;
        QImage img;
        Lib3dsMaterial *mat = lib3ds_file_material_by_name(_file3ds, f->material);
        Q_ASSERT(mat);
        QString textureName = mat->texture1_map.name;
        if (!_textureFilenamesIndexes.contains(textureName))
        {
            qDebug() << "texture name" << textureName;
            GLuint tmpIndex = generateTextureStatic(textureName, 0); // temporary index to old the place of our texture
            _textureFilenamesIndexes.insert(mat->texture1_map.name, tmpIndex);
        }
    }
}

// this function actually renders the model at place (x, y, z) and then rotated around the y axis by 'angle' degrees
void Model::renderModel()
{
    Q_ASSERT(_file3ds);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glDisable(GL_LIGHTING);

    glEnable(GL_TEXTURE_2D);
    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);
    glEnableClientState(GL_NORMAL_ARRAY);

    foreach (const Mesh &mesh, _meshes)
        renderMesh(mesh);

    glDisable(GL_TEXTURE_2D);
    glDisableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_TEXTURE_COORD_ARRAY);
    glDisableClientState(GL_NORMAL_ARRAY);
}

void Model::renderMesh(const Mesh &mesh)
{
    GL_CHECK( glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE));
    GL_CHECK( glBindTexture(GL_TEXTURE_2D, mesh._textureID));
    GL_CHECK( glVertexPointer(3, GL_FLOAT, 0, mesh._vertices.data()));
    GL_CHECK( glTexCoordPointer(2, GL_FLOAT, 0, mesh._textureVertices.data()));
    GL_CHECK( glNormalPointer(GL_FLOAT, 0, mesh._normals.data()));

    GL_CHECK( glDrawElements(GL_TRIANGLES, mesh._indices.size(), GL_UNSIGNED_SHORT, mesh._indices.data()));
}

Lib3dsFile * Model::get3DSPointer()
{
    return _file3ds;
}

const QString &Model::getFilename() const
{
    return _fileName;
}

bool Model::isValid() const
{
    return _isValid;
}

bool Model::isValidRadius() const
{
    return _meshRadius > 0;
}

double Model::meshRadius() const
{
    return _meshRadius;
}


QVector3D Model::getMin() const
{
    QVector3D minValue(+100000, +100000, +100000);
    foreach (const Mesh &mesh, _meshes)
    {
        const QVector<GLfloat> &vertices = mesh._vertices;
        Q_ASSERT(vertices.size() % 3 == 0);
        for (int i = 0; i < vertices.size(); i += 3)
        {
            QVector3D vertice(vertices[i], vertices[i+1], vertices[i+2]);

            if (minValue.x() > vertice.x())
                minValue.setX(vertice.x());
            if (minValue.y() > vertice.y())
                minValue.setY(vertice.y());
            if (minValue.z() > vertice.z())
                minValue.setZ(vertice.z());
        }
    }
    return minValue;
}

QVector3D Model::getMax() const
{
    QVector3D maxValue(-100000, -100000, -100000);
    foreach (const Mesh &mesh, _meshes)
    {
        const QVector<GLfloat> &vertices = mesh._vertices;
        Q_ASSERT(vertices.size() % 3 == 0);
        for (int i = 0; i < vertices.size(); i += 3)
        {
            QVector3D vertice(vertices[i], vertices[i+1], vertices[i+2]);

            if (maxValue.x() < vertice.x())
                maxValue.setX(vertice.x());
            if (maxValue.y() < vertice.y())
                maxValue.setY(vertice.y());
            if (maxValue.z() < vertice.z())
                maxValue.setZ(vertice.z());
        }
    }
    return maxValue;
}

void Model::centerModel()
{
    QVector3D bottomLeft = getMin();
    QVector3D topRight = getMax();

    QVector3D center = (bottomLeft + topRight) / 2;

    for (int i = 0; i < _meshes.size(); ++i)
    {
        Mesh &mesh = _meshes[i];
        QVector<GLfloat> &vertices = mesh._vertices;
        Q_ASSERT(vertices.size() % 3 == 0);
        for (int i = 0; i < vertices.size(); i += 3)
        {
            vertices[i+0] -= center.x();
            vertices[i+1] -= center.y();
            vertices[i+2] -= center.z();
        }
    }
}

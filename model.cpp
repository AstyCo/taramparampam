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

// constructor, enables and set properties of texture coordinate generation and set the current frame
Model::Model()
{
    _base = -1;
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
    int meshCount = 0;
    Lib3dsMesh *mesh;
    for(mesh = _file3ds->meshes;mesh != 0;mesh = mesh->next)
    {
        ++meshCount;
        if(mesh->texels) //if there's texels for the mesh
            ApplyTexture(mesh); //then apply texture to it
    }
    qDebug() << "meshCount" << meshCount;
}

void Model::prepareNodes()
{
    if (_base != -1) {
        Q_ASSERT(!glIsList(_base));
        _glListIndexes.clear();
    }
    _size = countOfNodesToPrepare();
    _base = glGenLists(_size);
    _nodeIndex = 0;

    _glListIndexes.reserve(_size); // optimization
    for (int i = 0; i < _size; ++i)
        _glListIndexes.append(i);

    for(Lib3dsNode *node = _file3ds->nodes; node != 0; node = node->next) // Render all nodes
        prepareNode(node);
}

void Model::prepareNode(Lib3dsNode *node)
{
    if (!node)
        return;
    Q_ASSERT(_base != -1);
    for(Lib3dsNode *childNode = node->childs; childNode != 0; childNode = childNode->next)
        prepareNode(childNode); //prepare all child nodes of this note
    ///
    static int k = 0;
    qDebug() << "prepareNode" << _nodeIndex << ++k;
    ///
    Lib3dsMesh *mesh = lib3ds_file_mesh_by_name(_file3ds, node->name); //get all the meshes of the current node
    if(! mesh)
        return;

//    Lib3dsVector *normals = static_cast<float(*)[3]> (std::malloc (3*sizeof(Lib3dsVector)*mesh->faces)); //alocate memory for our normals
//    lib3ds_mesh_calculate_normals(mesh, normals); // calculate the normals of the mesh
    static GLfloat diff[4] = { 0.75, 0.75, 0.75, 1.0 }; // color: white/grey
    static GLfloat amb[4] = { 0.25, 0.25, 0.25, 1.0 }; //color: black/dark gray
    static GLfloat spec[4] = { 0.0, 0.0, 0.0, 1.0 }; //color: completly black

    GL_CHECK( glNewList(_base + _nodeIndex++, GL_COMPILE)); //here we create our list

    glMaterialfv(GL_FRONT, GL_DIFFUSE, diff);
    glMaterialfv(GL_FRONT, GL_AMBIENT, amb);
    glMaterialfv(GL_FRONT, GL_AMBIENT, spec);

    glBegin(GL_TRIANGLES);
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
            glBindTexture(GL_TEXTURE_2D, _textureFilenamesIndexes[textureName]);
        }

        for(int i = 0;i < 3;i++)
        {
//            glNormal3fv(normals[3*p+i]); // set normal vector of that point
            if(isTextureValid)
                glTexCoord2f(mesh->texelL[f->points[i]][0], mesh->texelL[f->points[i]][1]);
            const Lib3dsVector &triangleVertice = mesh->pointL[f->points[i]].pos;
            glVertex3fv(triangleVertice);
            _meshRadius = qMax(_meshRadius, length3dsVector(triangleVertice));
        }
    }
    glEnd();
    glEndList(); // end of list

//    free(normals); // free up memory
}

int Model::countOfNodesToPrepare() const
{
    int result = 0;
    for(Lib3dsNode *node = _file3ds->nodes; node != 0; node = node->next)
        result += countOfNodesToPrepare(node);
    return result;
}

int Model::countOfNodesToPrepare(Lib3dsNode *node) const
{
    int result = 0;
    if (!node)
        return result;
    for(Lib3dsNode *childNode = node->childs; childNode != 0; childNode = childNode->next)
        result += countOfNodesToPrepare(childNode);

    Lib3dsMesh *mesh = lib3ds_file_mesh_by_name(_file3ds, node->name); //get all the meshes of the current node
    if(!mesh)
        return result;
    else
        return result + 1;
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
            qDebug() << "texture name" << mat->texture1_map.name;
            if(!img.load(mat->texture1_map.name))
            {
                Q_ASSERT_X(false,"Model::ApplyTexture" , "Error loading 3ds texture, perhaps file format not supported?");
                return;
            }
            img = QGLWidget::convertToGLFormat(img);
            /// TODO my load texture
            GLuint tmpIndex; // temporary index to old the place of our texture
            glGenTextures(1, &tmpIndex); // allocate memory for one texture
            _textureFilenamesIndexes.insert(mat->texture1_map.name, tmpIndex);
            glBindTexture(GL_TEXTURE_2D, tmpIndex); // use our newest texture
//            gluBuild2DMipmaps(GL_TEXTURE_2D, GL_RGB, img.width() , img.height(), GL_RGBA, GL_UNSIGNED_BYTE, img.bits()); // genereate MipMap levels for our texture
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR); // give the best result for texture magnification
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR); //give the best result for texture minification
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP); // don't repeat texture
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP); // don't repeat texture
        }
    }
}

// this function actually renders the model at place (x, y, z) and then rotated around the y axis by 'angle' degrees
void Model::RenderModel()
{
    Q_ASSERT(_file3ds);
    Q_ASSERT(_base != -1);
    glEnable(GL_CULL_FACE);
    glDisable(GL_LIGHTING);
    glCullFace(GL_BACK);
    glShadeModel(GL_SMOOTH);

    GL_CHECK( glPushAttrib(GL_LIST_BIT));
    GL_CHECK( glListBase(_base));
//    for (int i = 0; i < _glListIndexes.size(); ++i) {
//        Q_ASSERT(glIsList(_base + _glListIndexes[i]));
//        GL_CHECK ( glCallList(_base + _glListIndexes[i]));
//    }
    GL_CHECK( glCallLists(_size, GL_UNSIGNED_BYTE, _glListIndexes.data()));
    GL_CHECK( glPopAttrib());
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
    return _base != -1;
}

bool Model::isValidRadius() const
{
    return _meshRadius > 0;
}

double Model::meshRadius() const
{
    return _meshRadius;
}

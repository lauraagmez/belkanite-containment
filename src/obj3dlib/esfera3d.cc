#include "obj3dlib/esfera3d.h"
#include <GL/glut.h>


void EsferaObj3D::drawSolido(){
  glColor3f(colores.x,colores.y,colores.z);
  GLfloat color[4] = {colores.x, colores.y, colores.z, 1.0f};
  glMaterialfv(GL_FRONT, GL_AMBIENT, color);
  glMaterialfv(GL_FRONT, GL_DIFFUSE, color);
  glutSolidSphere(1.0, 25, 25);
}

void EsferaObj3D::drawSuave(){
  glShadeModel(GL_SMOOTH);
  glMaterialfv(GL_FRONT,GL_AMBIENT,(GLfloat *) &matAmb);
  glMaterialfv(GL_FRONT,GL_DIFFUSE,(GLfloat *) &matDiff);
  glMaterialfv(GL_FRONT,GL_SPECULAR,(GLfloat *) &matSpec);
  glMaterialf(GL_FRONT,GL_SHININESS,10);

  glMatrixMode(GL_MODELVIEW);
  glColor3f(colores.x,colores.y,colores.z);
  glutSolidSphere(1.0, 100, 100);
}

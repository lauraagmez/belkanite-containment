#include "modelos/arbol/arbol.hpp"
#include <GL/glut.h>

Arbol3D::Arbol3D() : Objeto3D(){
  glTranslatef(posicion.x, posicion.y, posicion.z);

  // Root en el suelo (Y=0) para evitar cálculos complejos con la escala global
  setTranformacion(_vertex4<float>(Translacion_, 0.0, 0.0, 0.0));

  // Tronco (Cilindro) - Altura 1.0 en local (serán ~11.0 en mundo)
  setHijo(new RevolucionObj3D("ply/cilindro.ply", 50));
  hijos[0] -> setTranformacion(_vertex4<float>(Escalado_, 0.4, 1.0, 0.4));
  hijos[0] -> setTranformacion(_vertex4<float>(Translacion_, 0.0, 0.0, 0.0));
  hijos[0] -> setColor(_vertex3<float>(45.0/256.0, 30.0/256.0, 10.0/256.0));

  // Copa (Cono) - Comienza donde termina el tronco (Y=1.0)
  setHijo(new ConoObj3D());
  // Para que empiece en 1.0 con escala 2.0, la traslación local debe ser 0.5 (0.5 * 2.0 = 1.0)
  hijos[1] -> setTranformacion(_vertex4<float>(Escalado_, 1.2, 2.0, 1.2));
  hijos[1] -> setTranformacion(_vertex4<float>(Translacion_, 0.0, 0.5, 0.0));
  hijos[1] -> setTranformacion(_vertex4<float>(Rotacion_, 0.0, 270.0, 0.0)); // 270 en X-axis
  hijos[1] -> setColor(_vertex3<float>(10.0/256.0, 60.0/256.0, 25.0/256.0));

  // Wireframe
  setHijo(new ConoObj3DWire());
  hijos[2] -> setTranformacion(_vertex4<float>(Escalado_, 1.21, 2.01, 1.21));
  hijos[2] -> setTranformacion(_vertex4<float>(Translacion_, 0.0, 0.5, 0.0));
  hijos[2] -> setTranformacion(_vertex4<float>(Rotacion_, 0.0, 270.0, 0.0));
  hijos[2] -> setColor(_vertex3<float>(0.0, 0.0, 0.0));
}

Arbol3D::~Arbol3D(){
  delete hijos[0];
  delete hijos[1];
  delete hijos[2];
}

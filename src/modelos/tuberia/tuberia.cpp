#include "modelos/tuberia/tuberia.hpp"
#include <GL/glut.h>

Tuberia3D::Tuberia3D() : Objeto3D() {
  glTranslatef(posicion.x, posicion.y, posicion.z);

  // Root at ground level (Y=0)
  setTranformacion(_vertex4<float>(Translacion_, 0.0, 0.0, 0.0));

  // Main pipe body (Cylinder) - Height 2.0 units
  setHijo(new RevolucionObj3D("ply/cilindro.ply", 50));
  hijos[0] -> setTranformacion(_vertex4<float>(Escalado_, 1.2, 2.0, 1.2));
  hijos[0] -> setTranformacion(_vertex4<float>(Translacion_, 0.0, 0.0, 0.0));
  hijos[0] -> setColor(_vertex3<float>(0.7, 0.0, 0.0)); // Dark red

  // Top rim (Wider cylinder) - Starts at Y= 2.0
  setHijo(new RevolucionObj3D("ply/cilindro.ply", 50));
  hijos[1] -> setTranformacion(_vertex4<float>(Translacion_, 0.0, 2.0, 0.0)); // Translate first
  hijos[1] -> setTranformacion(_vertex4<float>(Escalado_, 1.5, 0.8, 1.5));     // Then scale
  hijos[1] -> setColor(_vertex3<float>(1.0, 0.2, 0.2)); // Lighter red rim

  // Inner black hole (Disk/Cylinder) - Recessed slightly
  setHijo(new RevolucionObj3D("ply/cilindro.ply", 50));
  hijos[2] -> setTranformacion(_vertex4<float>(Translacion_, 0.0, 2.75, 0.0)); // Translate first
  hijos[2] -> setTranformacion(_vertex4<float>(Escalado_, 1.0, 0.1, 1.0));   // Then scale (1.35)
  hijos[2] -> setColor(_vertex3<float>(0.0, 0.0, 0.0)); // Pitch black
}


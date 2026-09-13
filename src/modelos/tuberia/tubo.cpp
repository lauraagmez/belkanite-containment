#include "modelos/tuberia/tubo.hpp"
#include <GL/gl.h>
#include <cmath>

Tubo3D::Tubo3D() : TuboBase3D() {
  // 1. Main horizontal body (Center at 0.8 height, spans -2.5 to 2.5 on Z)
  addSegment(5.0, 0, 1, 0.0, 0.8, -2.5);

  // 2. Mario Rims at both ends for "segment" look
  addFlange(-2.5);
  addFlange(2.5);

  // 3. Simple supports at the ends
  addSupport(0.0, -2.0);
  addSupport(0.0, 2.0);
}


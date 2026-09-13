#include "modelos/tuberia/tubocodo.hpp"
#include <GL/gl.h>
#include <cmath>

TuboCodo3D::TuboCodo3D() : TuboBase3D() {
  // 1. North Segment (along -Z axis)
  addSegment(2.5, 0, 1, 0.0, 0.8, -2.5);
  
  // 2. West Segment (along -X axis)
  addSegment(2.5, 2, 1, 0.0, 0.8, 0.0); // Axis 2 (Z), rotation 90 (Y -> -X)

  // 3. Central Elbow Junction (Thick Mario Joint)
  addJoint(0.0, 0.8, 0.0, 0.6);

  // 4. Simple supports
  addSupport(0.0, -1.8); // Support for North segment
  addSupport(-1.8, 0.0); // Support for West segment
}


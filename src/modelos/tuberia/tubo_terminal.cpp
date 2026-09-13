#include "modelos/tuberia/tubo_terminal.hpp"
#include <GL/gl.h>

TuboTerminal3D::TuboTerminal3D() : TuboBase3D() {
  // 1. Pipe Body (spans from edge -2.5 to center 0.0 on Z)
  addSegment(2.5, 0, 1, 0.0, 0.8, -2.5);

  // 2. Mario Rim at the terminal end (where it "opens" or connects to nothing)
  addFlange(-2.5);

  // 3. Thicker Joint at the center to hide intersections
  addJoint(0.0, 0.8, 0.0, 0.6);

  // 4. Single support
  addSupport(0.0, -1.5);
}

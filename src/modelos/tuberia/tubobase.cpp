#include "modelos/tuberia/tubobase.hpp"

TuboBase3D::TuboBase3D() : Objeto3D() {
    pipeColor = _vertex3<float>(0.7, 0.0, 0.0); // Mario Dark Red
    supportColor = _vertex3<float>(0.5, 0.0, 0.0); // Slightly darker for supports
}

void TuboBase3D::addSegment(float length, int axis, int rotationCode, float tx, float ty, float tz) {
    setHijo(new RevolucionObj3D("ply/cilindro.ply", 50));
    hijos.back()->setColor(pipeColor);
    hijos.back()->setTranformacion(_vertex4<float>(Translacion_, tx, ty, tz));
    hijos.back()->setTranformacion(_vertex4<float>(Rotacion_, axis, 90 * rotationCode, 0));
    hijos.back()->setTranformacion(_vertex4<float>(Escalado_, 1.2, length, 1.2)); // Thicker (radius 0.6)
}

void TuboBase3D::addSupport(float tx, float tz) {
    setHijo(new RevolucionObj3D("ply/cilindro.ply", 20));
    hijos.back()->setColor(supportColor);
    hijos.back()->setAmb(_vertex4<float>(supportColor.x*0.4, supportColor.y*0.4, supportColor.z*0.4, 1.0));
    hijos.back()->setDiff(_vertex4<float>(supportColor.x, supportColor.y, supportColor.z, 1.0));
    hijos.back()->setTranformacion(_vertex4<float>(Translacion_, tx, 0.0, tz));
    hijos.back()->setTranformacion(_vertex4<float>(Escalado_, 0.4, 0.8, 0.4));
}

void TuboBase3D::addJoint(float tx, float ty, float tz, float scale) {
    setHijo(new EsferaObj3D()); // Rounded joints look more like Mario
    hijos.back()->setColor(_vertex3<float>(1.0, 0.2, 0.2)); // Mario Light Red
    hijos.back()->setTranformacion(_vertex4<float>(Translacion_, tx, ty, tz));
    hijos.back()->setTranformacion(_vertex4<float>(Escalado_, scale, scale, scale));
}

void TuboBase3D::addFlange(float tz) {
    // Mario Rim
    _vertex3<float> rimColor(1.0, 0.2, 0.2); // Mario Light Red
    setHijo(new RevolucionObj3D("ply/cilindro.ply", 50));
    hijos.back()->setColor(rimColor);
    hijos.back()->setTranformacion(_vertex4<float>(Translacion_, 0.0, 0.8, tz));
    hijos.back()->setTranformacion(_vertex4<float>(Rotacion_, 0, 90, 0));
    hijos.back()->setTranformacion(_vertex4<float>(Translacion_, 0.0, -0.4, 0.0)); // Center the rim
    hijos.back()->setTranformacion(_vertex4<float>(Escalado_, 1.5, 0.8, 1.5));
}

void TuboBase3D::addValve(float tz) {
    // NO-OP for Mario pipes
}

void TuboBase3D::addVerticalConnector(float tz, float h_diff) {
    // 1. Vertical segment (rising/falling from the end of the pipe)
    setHijo(new RevolucionObj3D("ply/cilindro.ply", 50));
    hijos.back()->setColor(pipeColor);
    // Position at the end of the horizontal segment (tz), at pipe height (0.8)
    float ty = 0.8;
    float length = fabs(h_diff);
    float start_y = (h_diff > 0) ? ty : ty + h_diff;
    
    hijos.back()->setTranformacion(_vertex4<float>(Translacion_, 0.0, start_y, tz));
    hijos.back()->setTranformacion(_vertex4<float>(Escalado_, 1.2, length, 1.2)); // Same thickness
    
    // 2. Corner joint (at the new height)
    addJoint(0.0, 0.8 + h_diff, tz, 0.6);
}

VerticalConnector3D::VerticalConnector3D() : TuboBase3D() {}

void VerticalConnector3D::setConnector(float h_diff) {
    // Clear previous dynamic children
    for (auto h : hijos) delete h;
    hijos.clear();
    
    if (h_diff == 0) return;
    
    // 1. Vertical segment
    // We want a cylinder that spans from height 0.8 (current) to 0.8 + h_diff (destination)
    setHijo(new RevolucionObj3D("ply/cilindro.ply", 50));
    hijos.back()->setColor(pipeColor);
    
    float ty = 0.8;
    float length = fabs(h_diff);
    // If h_diff > 0, we start at 0.8 and go UP.
    // If h_diff < 0, we start at 0.8 + h_diff and go UP to 0.8.
    float start_y = (h_diff > 0) ? ty : ty + h_diff;
    
    hijos.back()->setTranformacion(_vertex4<float>(Translacion_, 0.0, start_y, 0.0));
    hijos.back()->setTranformacion(_vertex4<float>(Escalado_, 1.2, length, 1.2));
    
    // 2. Joints at both ends of the vertical segment to ensure smooth connections
    addJoint(0.0, ty, 0.0, 0.6); // Current height joint
    addJoint(0.0, ty + h_diff, 0.0, 0.6); // Destination height joint
}

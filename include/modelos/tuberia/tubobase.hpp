#ifndef TUBOBASE_H
#define TUBOBASE_H

#include "obj3dlib.hpp"

class TuboBase3D : public Objeto3D {
protected:
    _vertex3<float> pipeColor;
    _vertex3<float> supportColor;

    // Helpers for common industrial pipe components
    void addSegment(float length, int axis, int rotationCode, float tx = 0, float ty = 0.8, float tz = 0);
    void addSupport(float tx, float tz);
    void addJoint(float tx, float ty, float tz, float scale = 0.45);
    void addFlange(float tz);
    void addValve(float tz);
    void addVerticalConnector(float tz, float h_diff);

public:
    TuboBase3D();
    virtual ~TuboBase3D() {} // No longer needs manual cleanup
};

class VerticalConnector3D : public TuboBase3D {
public:
    VerticalConnector3D();
    void setConnector(float h_diff);
};

#endif

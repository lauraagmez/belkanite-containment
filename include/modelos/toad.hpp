#ifndef TOAD3D_HPP
#define TOAD3D_HPP

#include "obj3dlib.hpp"
#include <string>

class Toad3D : public Objeto3D {
public:
    Toad3D(const std::string & filename);
    virtual ~Toad3D();
};

#endif

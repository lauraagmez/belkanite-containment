#ifndef MARIO_LUIGI_HPP
#define MARIO_LUIGI_HPP

#include "obj3dlib.hpp"
#include <string>

class Mario3D : public Objeto3D {
public:
    Mario3D(const std::string & filename);
    virtual ~Mario3D();
};

#endif

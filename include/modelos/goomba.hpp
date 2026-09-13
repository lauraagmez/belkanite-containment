#ifndef GOOMBA3D_HPP
#define GOOMBA3D_HPP

#include "obj3dlib.hpp"
#include <string>

class Goomba3D : public Objeto3D {
public:
    Goomba3D(const std::string & filename);
    virtual ~Goomba3D();
};

#endif

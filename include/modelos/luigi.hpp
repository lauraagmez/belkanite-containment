#ifndef LUIGI3D_HPP
#define LUIGI3D_HPP

#include "obj3dlib.hpp"
#include <string>

class Luigi3D : public Objeto3D {
public:
    Luigi3D(const std::string & filename);
    virtual ~Luigi3D();
};

#endif

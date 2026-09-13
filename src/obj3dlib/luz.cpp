#include"obj3dlib/luz.hpp"
#include <cmath>

void Luz::draw(){
    if(encendida==true){
        // Changed to Directional Light (w=0) for uniform sunlight everywhere
        GLfloat posL[4] = {posicion.x, posicion.y, posicion.z, 0.0};
        
        // Enhanced lighting values for more realistic illumination
        GLfloat ambi[4] = {0.35, 0.35, 0.38, 1.0};  // Balanced ambient
        GLfloat diff[4] = {0.85, 0.85, 0.82, 1.0};  // Stronger diffuse
        GLfloat spec[4] = {0.0, 0.0, 0.0, 1.0};   // Respect user request for no shine
        
        glLightModeli(GL_LIGHT_MODEL_LOCAL_VIEWER, GL_TRUE);
        glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, GL_TRUE);
        
        // Set global ambient light
        GLfloat globalAmbi[4] = {0.4, 0.4, 0.45, 1.0}; // Slightly brighter global ambient to reduce shadows
        glLightModelfv(GL_LIGHT_MODEL_AMBIENT, globalAmbi);
        
        glLightfv(nLuz, GL_POSITION, posL);
        glLightfv(nLuz, GL_AMBIENT, ambi);
        glLightfv(nLuz, GL_DIFFUSE, diff);
        glLightfv(nLuz, GL_SPECULAR, spec);
        glEnable(nLuz);
    }
    else{
        glDisable(nLuz);
    }
}

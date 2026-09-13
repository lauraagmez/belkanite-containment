#include "ingeniero.hpp"
#include "motorlib/util.h"
#include <iostream>
#include <queue>
#include <set>
#include <string>

using namespace std;

// =========================================================================
// ÁREA DE IMPLEMENTACIÓN DEL ESTUDIANTE
// =========================================================================

Action ComportamientoIngeniero::think(Sensores sensores)
{
  Action accion = IDLE;

  // Decisión del agente según el nivel
  switch (sensores.nivel)
  {
  case 0:
    accion = ComportamientoIngenieroNivel_0(sensores);
    break;
  case 1:
    accion = ComportamientoIngenieroNivel_1(sensores);
    break;
  case 2:
    accion = ComportamientoIngenieroNivel_2(sensores);
    break;
  case 3:
    accion = ComportamientoIngenieroNivel_3(sensores);
    break;
  case 4:
    accion = ComportamientoIngenieroNivel_4(sensores);
    break;
  case 5:
    accion = ComportamientoIngenieroNivel_5(sensores);
    break;
  case 6:
    accion = ComportamientoIngenieroNivel_6(sensores);
    break;
  }

  return accion;
}


char ComportamientoIngeniero::ViablePorAltura (char casilla, int dif, bool zap){
  if (abs(dif)<=1 or (zap and abs(dif)<=2)) return casilla;
  else return 'P';
}
void ComportamientoIngeniero::EvaluarAlturas(const Sensores &sensores, char &i, char &c, char &d) {
  i = ViablePorAltura(sensores.superficie[1], sensores.cota[1] - sensores.cota[0], tiene_zapatillas);
  c = ViablePorAltura(sensores.superficie[2], sensores.cota[2] - sensores.cota[0], tiene_zapatillas);
  d = ViablePorAltura(sensores.superficie[3], sensores.cota[3] - sensores.cota[0], tiene_zapatillas);
}
void ComportamientoIngeniero::AplicarObstaculosAgentes(const Sensores &sensores, char &i, char &c, char &d) {
  // Si hay un agente cerca de mi, me tomo la casilla como un muro
  if (sensores.agentes[1] != '_') i = 'M';
  if (sensores.agentes[2] != '_') c = 'M';
  if (sensores.agentes[3] != '_') d = 'M';
  
  //Ocultamos la casilla si el obstáculo está justo detrás (distancia 2)
  if (sensores.agentes[5] != '_') i = 'M';
  if (sensores.agentes[6] != '_') c = 'M';
  if (sensores.agentes[7] != '_') d = 'M';
}

ubicacion ComportamientoIngeniero::GirarIzquierda(ubicacion p) const{
  p.brujula = (Orientacion)(((int)p.brujula + 7) % 8);
  return p;
}
ubicacion ComportamientoIngeniero::GirarDerecha(ubicacion p) const{
  p.brujula = (Orientacion)(((int)p.brujula + 1) % 8);
  return p;
}
bool ComportamientoIngeniero::EsCamino(unsigned char c) const{
  return (c == 'C' || c == 'D' || c == 'U');
}

void ComportamientoIngeniero::ActualizarMapaPasos (const Sensores &sensores){
  if (sensores.posF != ultima_pos.first || sensores.posC != ultima_pos.second) {
    mapaPasos[sensores.posF][sensores.posC]++;
    ultima_pos = {sensores.posF, sensores.posC};
  }
}
void ComportamientoIngeniero::ActualizarYDetectarBucle(const Sensores &sensores) {
  pair<int, int> pos_actual = {sensores.posF, sensores.posC};

  // 1. Guardamos la posición actual
  ultimos_pasos.push_back(pos_actual);

  // 2. Mantenemos la memoria corta (solo recordamos los últimos 20 "ticks")
  if (ultimos_pasos.size() > 20) {
    ultimos_pasos.pop_front();
  }

  // 3. Contamos cuántas veces aparece nuestra posición actual en la memoria reciente
  int repeticiones = 0;
  for (auto it = ultimos_pasos.begin(); it != ultimos_pasos.end(); ++it) {
    if (*it == pos_actual) {
      repeticiones++;
    }
  }

  // 4. Si estamos atascados en la misma zona, encendemos la alarma
  if (repeticiones > 4) {
    en_bucle = true;
  } else {
    en_bucle = false;
  }
}
int ComportamientoIngeniero::VeoZapatillasVisual(const Sensores &sensores, bool izq, bool cen, bool der) {
  // CENTRO: Prioridad absoluta para ir recto
  if (cen && (sensores.superficie[2] == 'D' || sensores.superficie[6] == 'D' || sensores.superficie[12] == 'D')) {
      return 2;
  }

  // IZQUIERDA: Escaneo de todo el sector izquierdo
  if (izq && (sensores.superficie[1] == 'D' || sensores.superficie[4] == 'D' || 
              sensores.superficie[5] == 'D' || sensores.superficie[9] == 'D' || 
              sensores.superficie[10] == 'D' || sensores.superficie[11] == 'D')) {
      return 1;
  }

  // DERECHA: Escaneo de todo el sector derecho
  if (der && (sensores.superficie[3] == 'D' || sensores.superficie[7] == 'D' || 
              sensores.superficie[8] == 'D' || sensores.superficie[13] == 'D' || 
              sensores.superficie[14] == 'D' || sensores.superficie[15] == 'D')) {
      return 3;
  }

  return 0;
}


//NIVEL 0: INGENIERO BUSCA 'U'
void ComportamientoIngeniero::DetectarMeta(const Sensores &sensores) {
  ubicacion actual = {sensores.posF, sensores.posC, sensores.rumbo};
  
  for (int i = 1; i < 16; i++) { // Para los 15 sensores
    if (sensores.superficie[i] == 'U') {
      ubicacion posU = actual; // Inicializamos

      // --- DISTANCIA 1 ---
      if (i == 1) posU = Delante(GirarIzquierda(actual));
      else if (i == 2) posU = Delante(actual);
      else if (i == 3) posU = Delante(GirarDerecha(actual));
      
      // --- DISTANCIA 2 ---
      else if (i == 4) posU = Delante(Delante(GirarIzquierda(actual))); 
      else if (i == 5) posU = Delante(GirarIzquierda(Delante(actual))); 
      else if (i == 6) posU = Delante(Delante(actual));                 
      else if (i == 7) posU = Delante(GirarDerecha(Delante(actual)));   
      else if (i == 8) posU = Delante(Delante(GirarDerecha(actual)));   
      
      // --- DISTANCIA 3 ---
      else if (i == 9)  posU = Delante(Delante(Delante(GirarIzquierda(actual)))); 
      else if (i == 10) posU = Delante(Delante(GirarIzquierda(Delante(actual)))); 
      else if (i == 11) posU = Delante(GirarIzquierda(Delante(Delante(actual)))); 
      else if (i == 12) posU = Delante(Delante(Delante(actual)));                 
      else if (i == 13) posU = Delante(GirarDerecha(Delante(Delante(actual))));   
      else if (i == 14) posU = Delante(Delante(GirarDerecha(Delante(actual))));   
      else if (i == 15) posU = Delante(Delante(Delante(GirarDerecha(actual))));

      // Memorizamos la posición calculada solo si es accesible
      pair <int, int> coordenada_candidata = {posU.f, posU.c};
    
      // Si hay un agente en esa 'U', es la meta permanentemente ocupada
      if (sensores.agentes[i] != '_') {
        meta_ocupada = coordenada_candidata;
        
        // Si estábamos persiguiendo esta meta y resulta que está ocupada, la descartamos
        if (meta_conocida && meta_encontrada == coordenada_candidata) {
            meta_conocida = false;
            meta_encontrada = {-1, -1};
        }
      } 
      // Si la 'U' está libre, y NO es la que sabemos que está ocupada
      else if (coordenada_candidata != meta_ocupada) {
        meta_encontrada = coordenada_candidata;
        meta_conocida = true;
        return; // Salimos porque ya encontramos una meta válida
      }
    }
    
  }
}
int ComportamientoIngeniero::AvanzaMetaConocida(const Sensores &sensores, char i, char c, char d) {
  int direcciones = 3;
  ubicacion actual = {sensores.posF, sensores.posC, sensores.rumbo};
  ubicacion opciones[direcciones] = {Delante(actual), Delante(GirarIzquierda(actual)), Delante(GirarDerecha(actual))};
  char terreno[direcciones] = {c, i, d};
  int movimientos[direcciones]={2, 1, 3}; //izquierda, centro, derecha
  
  int mejor_opcion = 0; 
  int min_coste = 999999;

  // Significa que pisar una casilla visitada "duele" como si te alejaras X pasos de la meta.
  const int PENALIZACION = 5;
  for (int j = 0; j < direcciones; j++) { //Para las tres posibles direcciones
    if (EsCamino(terreno[j])) {
      //1. Calculamos la distancia Manhattan
      int dist = abs(opciones[j].f - meta_encontrada.first) + abs(opciones[j].c - meta_encontrada.second);
      //2. Consultamos el coste de pasos
      int visitas = mapaPasos[opciones[j].f][opciones[j].c];
      //3. Evaluamos el coste
      int coste = dist + (visitas*PENALIZACION);
      if (coste < min_coste) {
        min_coste = coste;
        mejor_opcion = movimientos[j];
      }
    }
  }
  return mejor_opcion;
}
int ComportamientoIngeniero::VeoCasillaInteresante(const Sensores &sensores, char i, char c, char d, bool zap) {
  
  // 1. Asegurarnos de que el paso inmediato es legal en el Nivel 0 ("UDC")
  bool izq_valida = (permitidas_nivel0.find(i) != string::npos);
  bool cen_valida = (permitidas_nivel0.find(c) != string::npos);
  bool der_valida = (permitidas_nivel0.find(d) != string::npos);

  if (en_bucle) {
    return ElegirMenosVisitada_N0(sensores, i, c, d, permitidas_nivel0);
  }

  // PRIORIDAD 1: Ir a la meta ('U')
  // Con DetectarMeta(), si la 'U' está en cualquier punto de nuestra visión (0 a 15)
  // o la vimos antes, meta_conocida será true. Delegamos el movimiento a AvanzaMetaConocida
  if (meta_conocida) {
    int pos = AvanzaMetaConocida(sensores, i, c, d);
    // Comprobamos que la dirección que sugiere AvanzaMetaConocida no es un muro/precipicio
    if (pos == 2 && cen_valida) return 2;
    if (pos == 1 && izq_valida) return 1;
    if (pos == 3 && der_valida) return 3;
   
   
  }

  // PRIORIDAD 2: Radar de Zapatillas
  if (!zap) {
      int dir_zap = VeoZapatillasVisual(sensores, izq_valida, cen_valida, der_valida);
      if (dir_zap != 0) return dir_zap;
  }

  // PRIORIDAD 3: Supervivencia y avance normal (buscamos la menos pisada)
  return ElegirMenosVisitada_N0(sensores, i, c, d, permitidas_nivel0);
}
Action ComportamientoIngeniero::RompeBucles_N0(Action accion_propuesta, char c, const string &transitables) {

  // 1.  Guardamos la accion que ejecutaríamos
  vector<Action> historial_futuro = historial_acciones;
  historial_futuro.push_back(accion_propuesta);

  if (historial_futuro.size() > 4)
    historial_futuro.erase(historial_futuro.begin());

  Action accion_final = accion_propuesta;

  // 2. Detectamos bucle típico de giro: L-R-L-R o R-L-R-L
  if (historial_futuro.size() == 4) {
    bool bucle_L_R = (historial_futuro[0] == TURN_SL &&
                      historial_futuro[1] == TURN_SR &&
                      historial_futuro[2] == TURN_SL &&
                      historial_futuro[3] == TURN_SR);

    bool bucle_R_L = (historial_futuro[0] == TURN_SR &&
                      historial_futuro[1] == TURN_SL &&
                      historial_futuro[2] == TURN_SR &&
                      historial_futuro[3] == TURN_SL);

    if (bucle_L_R || bucle_R_L) {

      // 3. Intentamos romper el bucle de forma inteligente
      if (transitables.find(c) != string::npos) {
        // Si podemos avanzar, avanzamos
        accion_final = WALK;
      } else {
        // Si no podemos avanzar, cambiamos el sentido
        accion_final = (accion_propuesta == TURN_SL ? TURN_SR : TURN_SL);
      }
    }
  }

  // 4. Guardamos la acción ejecutada
  historial_acciones.push_back(accion_final);
  if (historial_acciones.size() > 4)
    historial_acciones.erase(historial_acciones.begin());

  return accion_final;
}
int ComportamientoIngeniero::ElegirMenosVisitada_N0(const Sensores &sensores, char i, char c, char d, string permitidas){
  ubicacion actual;
  actual.f = sensores.posF;
  actual.c = sensores.posC;
  actual.brujula = sensores.rumbo;

  ubicacion izq = Delante(GirarIzquierda(actual));
  ubicacion cen = Delante(actual);
  ubicacion der = Delante(GirarDerecha(actual));

  int mejor = 0;
  int minPasos = 999999;
  int mejorCosto = 999999; // Guardará el índice de la letra en el string (menor = más barata)

  // 1. CENTRO (Damos mayor prioridad a ir de frente)
  int posC = permitidas.find(c);
  if (posC != string::npos) {
    
    if (cen.f >= 0 && cen.f < mapaPasos.size() && cen.c >= 0 && cen.c < mapaPasos[0].size()) {
      // Si tiene menos visitas o tiene las mismas pero es más barata
      if (mapaPasos[cen.f][cen.c] < minPasos || (mapaPasos[cen.f][cen.c] == minPasos && posC < mejorCosto)){
        minPasos = mapaPasos[cen.f][cen.c];
        mejorCosto = posC; // Guardamos el coste de esta casilla
        mejor = 2;
      }
    }
    
  }
  //2. IZQUIERDA
  int posI = permitidas.find(i);
  if (posI != string::npos) {
    if (izq.f >= 0 && izq.f < mapaPasos.size() && izq.c >= 0 && izq.c < mapaPasos[0].size()) {
      if (mapaPasos[izq.f][izq.c] < minPasos || (mapaPasos[izq.f][izq.c] == minPasos && posI < mejorCosto)) {
        minPasos = mapaPasos[izq.f][izq.c];
        mejorCosto = posI;
        mejor = 1;
      }
    }
  }

  // 3. DERECHA
  int posD = permitidas.find(d);
  if (posD != string::npos) {
    if (der.f >= 0 && der.f < mapaPasos.size() && der.c >= 0 && der.c < mapaPasos[0].size()) {
      if (mapaPasos[der.f][der.c] < minPasos || (mapaPasos[der.f][der.c] == minPasos && posD < mejorCosto)) {
        minPasos = mapaPasos[der.f][der.c];
        mejorCosto = posD;
        mejor = 3;
      }
    }
  }

  return mejor;
}

//NIVEL 1: INGENIERO EXPLORA MAPA
Action ComportamientoIngeniero::RompeBucles_N1(const Sensores &sensores, Action accion_propuesta, char c, const string &transitables) {
  // 1. Aumentamos el historial para detectar patrones más largos
  historial_acciones.push_back(accion_propuesta);
  if (historial_acciones.size() > 8) { // Memoria de 8 acciones
    historial_acciones.erase(historial_acciones.begin());
  }

  Action accion_final = accion_propuesta;

  // 2. Contamos cuántos giros hemos hecho recientemente
  int conteo_giros = 0;
  for (Action a : historial_acciones) {
    if (a == TURN_SL || a == TURN_SR) conteo_giros++;
  }

  // 3. Si más del 75% de nuestras últimas acciones son giros, estamos en un bucle
  if (historial_acciones.size() >= 6 && conteo_giros >= (historial_acciones.size() * 0.75)) {
    
    // Intentamos romper el bucle forzando un avance
    bool centro_transitable = (transitables.find(c) != string::npos);
    
    if (centro_transitable) {
      // Si el centro es seguro, evaluamos si podemos hacer JUMP
      if (sensores.agentes[2] == '_' && sensores.agentes[6] == '_' && 
          transitables.find(ViablePorAltura(sensores.superficie[6], sensores.cota[6]-sensores.cota[0], tiene_zapatillas)) != string::npos) {
        accion_final = JUMP;
      } else {
        accion_final = WALK;
      }
    } else {
      // Si no podemos avanzar de frente, forzamos que el próximo giro sea mantenido
      accion_final = (sensores.energia % 2 == 0) ? TURN_SL : TURN_SR; 
    }
    
    // Limpiamos parte del historial para no re-activar el rompebucles inmediatamente
    historial_acciones.clear();
  }

  return accion_final;
}
int ComportamientoIngeniero::ElegirMenosVisitada_N1(const Sensores &sensores, char i, char c, char d, string permitidas){
  ubicacion actual;
  actual.f = sensores.posF;
  actual.c = sensores.posC;
  actual.brujula = sensores.rumbo;

  // Calculamos ubicaciones posibles
  ubicacion izq = Delante(GirarIzquierda(actual));
  ubicacion cen = Delante(actual);
  ubicacion der = Delante(GirarDerecha(actual));
  ubicacion jump = Delante(cen); // Ubicación a 2 casillas de distancia

  int mejor = 0;
  int minPasos = 999999;
  int mejorCosto = 999999;

  //JUMP
  // Evaluamos si el salto es físicamente posible (terreno y altura)
  char terrenoJump = ViablePorAltura(sensores.superficie[6], sensores.cota[6] - sensores.cota[0], tiene_zapatillas);
  int posJ = permitidas.find(terrenoJump);
  char terrenoIntermedio = ViablePorAltura(sensores.superficie[2], sensores.cota[2] - sensores.cota[0], tiene_zapatillas);
  bool caminoIntermedioSeguro = (permitidas.find(terrenoIntermedio) != string::npos);

  bool jumpPosible = (posJ != string::npos && 
                    caminoIntermedioSeguro &&
                    sensores.agentes[2] == '_' && 
                    sensores.agentes[6] == '_');
                    
  if (jumpPosible) {
    // Comprobamos los límites para la casilla del salto
    if (jump.f >= 0 && jump.f < mapaPasos.size() && jump.c >= 0 && jump.c < mapaPasos[0].size()) {
      minPasos = mapaPasos[jump.f][jump.c];
      mejorCosto = posJ;
      mejor = 4; // Empezamos asumiendo que saltar es buena opción
    }
  }

  //1. CENTRO (Prioridad sobre JUMP si tienen mismas visitas)
  int posC = permitidas.find(c);
  if (posC != string::npos) {
    if (cen.f >= 0 && cen.f < mapaPasos.size() && cen.c >= 0 && cen.c < mapaPasos[0].size()) {
      int pasosC = mapaPasos[cen.f][cen.c];
      // DESEMPATE ALEATORIO: Si pasos son iguales, usamos energía % 2 para decidir si cambiar
      if (pasosC < minPasos || (pasosC == minPasos && (posC < mejorCosto || sensores.energia % 2 == 0))) {
        minPasos = pasosC;
        mejorCosto = posC;
        mejor = 2;
      }
    }
  }

  //2. IZQUIERDA
  int posI = permitidas.find(i);
  if (posI != string::npos) {
    if (izq.f >= 0 && izq.f < mapaPasos.size() && izq.c >= 0 && izq.c < mapaPasos[0].size()) {
      int pasosI = mapaPasos[izq.f][izq.c];
      // Usamos el sensor de vida o energía como semilla de azar para desempatar
      if (pasosI < minPasos || (pasosI == minPasos && (posI < mejorCosto || sensores.vida % 2 == 0))) {
        minPasos = pasosI;
        mejorCosto = posI;
        mejor = 1;
      }
    }
  }

  //3. DERECHA
  int posD = permitidas.find(d);
  if (posD != string::npos) {
    if (der.f >= 0 && der.f < mapaPasos.size() && der.c >= 0 && der.c < mapaPasos[0].size()) {
      int pasosD = mapaPasos[der.f][der.c];
      if (pasosD < minPasos || (pasosD == minPasos && (posD < mejorCosto || (sensores.energia + sensores.vida) % 2 == 0))) {
        minPasos = pasosD;
        mejorCosto = posD;
        mejor = 3;
      }
    }
  }

  return mejor;
}
int ComportamientoIngeniero::VeoCasillaExplorable (const Sensores &sensores, char i, char c, char d, bool zap) {
  char c_salto = ViablePorAltura(sensores.superficie[6], sensores.cota[6] - sensores.cota[0], zap);
  bool salto_viable = (sensores.agentes[2] == '_' && sensores.agentes[6] == '_' && 
                       c_salto != 'M' && c_salto != 'B' && c_salto != 'P' && c_salto != '?');

  if (en_bucle) {
    return ElegirMenosVisitada_N1(sensores, i, c, d, permitidas_nivel1);
  }
  if (sensores.energia < sensores.energia/2) { 
      if (salto_viable && sensores.superficie[6] == 'X') return 4;
      if (c == 'X') return 2;
      if (i == 'X') return 1;
      if (d == 'X') return 3;
  }
  // 1. PRIORIDAD: Zapatillas 
  if (!zap) {
    // Si veo zapatillas justo en la casilla de aterrizaje y el salto es seguro, ¡salto!
    if (salto_viable && sensores.superficie[6] == 'D') return 4; 
    if (c == 'D') return 2;
    if (i == 'D') return 1;
    if (d == 'D') return 3;
  }
  

  // 2. IR HACIA LO DESCONOCIDO ('?')
  if (c == '?') return 2; 
  if (i == '?') return 1;
  if (d == '?') return 3;

  // 3. PRIORIDAD: Caminos (C)
  if (salto_viable && c == 'C' && sensores.superficie[6] == 'C') return 4;
  if (c == 'C') return 2; 
  if (i == 'C') return 1;
  if (d == 'C') return 3;

  // 4. PRIORIDAD: Senderos (S)
  if (salto_viable && c == 'S' && sensores.superficie[6] == 'S') return 4;
  if (c == 'S') return 2;
  if (i == 'S') return 1;
  if (d == 'S') return 3;

  // 4. PRIORIDAD: Las menos exploradas
  return ElegirMenosVisitada_N1(sensores, i, c, d, permitidas_nivel1);
  
}



// =========================================================================
// MOTOR DE BÚSQUEDA (NIVELES 2, 3)
// N2: MOTOR DE BÚSQUEDA BELKANITA
// N3: INGENIERO EVITA INTERPONERSE EN EL CAMINO DEL TÉNICO
// =========================================================================
  
EstadoI ComportamientoIngeniero::NextCasillaIngeniero (const EstadoI &st, Action accion){
  EstadoI siguiente = st;
  // Si la acción es JUMP, avanzamos 2 pasos. Si es WALK, 1 paso.
  int pasos = (accion == JUMP) ? 2 : 1; 

  for (int i = 0; i < pasos; i++) {
    switch (siguiente.site.brujula) {
      case norte:     siguiente.site.f--; break;
      case noreste:   siguiente.site.f--; siguiente.site.c++; break;
      case este:      siguiente.site.c++; break;
      case sureste:   siguiente.site.f++; siguiente.site.c++; break;
      case sur:       siguiente.site.f++; break;
      case suroeste:  siguiente.site.f++; siguiente.site.c--; break;
      case oeste:     siguiente.site.c--; break;
      case noroeste:  siguiente.site.f--; siguiente.site.c--; break;
    }
  }
  return siguiente;
}
bool ComportamientoIngeniero::CasillaAccesibleIngeniero(const EstadoI &st, const vector<vector<unsigned char>> &terreno, const vector<vector<unsigned char>> &altura, Action accion){
  EstadoI siguiente = NextCasillaIngeniero(st, accion); //queremos saber que el proximo paso de nuestra secuencia es accesible 
  // 0. Comprobar que no nos salimos del mapa
  if (siguiente.site.f < 0 || siguiente.site.f >= terreno.size() || siguiente.site.c < 0 || siguiente.site.c >= terreno[0].size()) {
    return false;
  }
  
  // 2. Obtener el terreno original y calcular la diferencia de altura
  char terrDest = terreno[siguiente.site.f][siguiente.site.c];
  int dif = altura[siguiente.site.f][siguiente.site.c] - altura[st.site.f][st.site.c];

  // 3. Filtrar por altura 
  terrDest = ViablePorAltura(terrDest, dif, st.zapatillas);

  // 4. Comprobar si el terreno resultante es transitable para el Ingeniero
  if (terrDest == 'M' || terrDest == 'P' || terrDest == 'B') return false; 

  // 5. Regla especial para JUMP: la casilla intermedia también debe ser transitable
  if (accion == JUMP) {
    EstadoI intermedia = NextCasillaIngeniero(st, WALK);
    char terrInt = terreno[intermedia.site.f][intermedia.site.c];
    
    // 5.1 Debe ser transitable (no ser Muro, Precipicio ni Bosque)
    if (terrInt == 'M' || terrInt == 'P' || terrInt == 'B') return false;
  }
  
  return true;
}
EstadoI ComportamientoIngeniero::applyI(Action accion, const EstadoI &st, const vector<vector<unsigned char>> &terreno, const vector<vector<unsigned char>> &altura){
  EstadoI siguiente = st;
  switch(accion){
    case WALK:
    case JUMP:
      if (CasillaAccesibleIngeniero(st,terreno,altura, accion)){
        siguiente = NextCasillaIngeniero(st, accion);
      }
      break;
    case TURN_SR:
      siguiente.site.brujula = (Orientacion) ((siguiente.site.brujula+1) % 8); //la siguiente
      break;
    case TURN_SL:
      siguiente.site.brujula = (Orientacion) ((siguiente.site.brujula+7) % 8); //la anterior
      break;
    default:
      break;
  }
  return siguiente;
}
list<Action> ComportamientoIngeniero::calcularRutaIngeniero(const EstadoI &inicio, const EstadoI &final, const vector<vector<unsigned char>> &terreno, const vector<vector<unsigned char>> &altura){
  NodoI current_node;       //Almacena el estado actual y que inicialmente toma el valor del parametro inicio
  list<NodoI> frontier;     //Mantiene los nodos pendientes de explorar, inicialmente vacía
  set <NodoI> explored;     //Mantiene los nodos ya explorados, inicialmente vacia
  list <Action> path;       //Devuelve la secuencia de acciones encontrada como soluccion

  //0. Se introduce el nodo actual en la lista pendiente de explorar
  current_node.estado = inicio;
  frontier.push_back(current_node);

  //1.Comprobamos que estemos justo en la solucion
  bool SolutionFound = (current_node.estado.site.f == final.site.f && current_node.estado.site.c == final.site.c);

  //2. Hasta que no se encuentre la solucion, o se hayan explorado todos los caminos 
  while (!SolutionFound && !frontier.empty()){
    
    //2.1.Eliminar el estado actual de la lista de pendientes y meterlo en la lista de explorados
    frontier.pop_front();
    explored.insert(current_node);

    //Recoge las zapatillas
    if (terreno[current_node.estado.site.f][current_node.estado.site.c] == 'D') {
        current_node.estado.zapatillas = true;
    }
    
    //2.2.Generamos el hijo resultante de aplicar la acción WALK
    NodoI child_Walk = current_node;
    child_Walk.estado = applyI(WALK, current_node.estado, terreno, altura);
    
    //Evaluamos si es solucion, sino lo metemos en la lista a explorar (siempre que no haya sido explorado y no esté ya en la lista)
    if (child_Walk.estado.site.f == final.site.f && child_Walk.estado.site.c == final.site.c){
      child_Walk.secuencia.push_back(WALK);
      current_node = child_Walk;
      SolutionFound = true;

    }else if (explored.find(child_Walk)==explored.end()){
      child_Walk.secuencia.push_back(WALK);
      frontier.push_back(child_Walk);
    }
    //2.3.Generamos el hijo resultante de aplicar la accion JUMP
    if (!SolutionFound){
      NodoI child_Jump = current_node;
      child_Jump.estado = applyI(JUMP, current_node.estado, terreno, altura);
      
      if (child_Jump.estado.site.f == final.site.f && child_Jump.estado.site.c == final.site.c){
        child_Jump.secuencia.push_back(JUMP);
        current_node = child_Jump;
        SolutionFound = true;
      }else if (explored.find(child_Jump)==explored.end()){
        child_Jump.secuencia.push_back(JUMP);
        frontier.push_back(child_Jump);
      }
    }
    //2.4.Generamos los hijos izquierda y derecha y los añadimos a la lista por explorar (si no estan ya y no han sido explorados)
    if (!SolutionFound){
      NodoI child_TurnSR =current_node;
      child_TurnSR.estado = applyI(TURN_SR, current_node.estado, terreno, altura);
      if (explored.find(child_TurnSR)==explored.end()){
        child_TurnSR.secuencia.push_back(TURN_SR);
        frontier.push_back(child_TurnSR);
      }
      NodoI child_TurnSL =current_node;
      child_TurnSL.estado = applyI(TURN_SL, current_node.estado, terreno, altura);
      if (explored.find(child_TurnSL)==explored.end()){
        child_TurnSL.secuencia.push_back(TURN_SL);
        frontier.push_back(child_TurnSL);
      }
    }
    //2.5. Tomamos nodos de la lista frontier hasta que unno no haya sido explorado
    if (!SolutionFound && !frontier.empty()){
      current_node=frontier.front();
      //Descartamos los nodos explorados
      while (explored.find(current_node) != explored.end() && !frontier.empty()) {
        frontier.pop_front();
        if (!frontier.empty()){
          current_node = frontier.front();
        }
        SolutionFound = (current_node.estado.site.f == final.site.f && current_node.estado.site.c == final.site.c);
      }
    }
  }
  
  //4. Si salimos del while porque ya se ha encontrado la solucion, ya tenemos el camino
  if (SolutionFound){
    path = current_node.secuencia;
  }
  return path;

}

// =========================================================================
// NIVEL 4: INGENIERO PLANIFICA LA RED DE TUBERÍAS
// =========================================================================
  
double ComportamientoIngeniero::HeuristicaTuberia(int f,int c, const vector<pair<int,int>>& plantasU)
{
    int mejor = 999999;

    for (const auto& planta : plantasU) {
      int dist = abs(f - planta.first) + abs(c - planta.second);
      if (dist < mejor) mejor = dist;
    }
    return mejor;

}
int ComportamientoIngeniero::CosteEnergeticoInstall(unsigned char terreno){
  switch (terreno) {
    case 'A': return 60;
    case 'H': return 45;
    case 'S': return 25;
    case 'C': case 'U': return 15;
    default:  return 30;
  }
}
int ComportamientoIngeniero::CosteEcoBaseInstall(unsigned char terreno) {
    switch (terreno) {
        case 'A': return 50;
        case 'H': return 45;
        case 'S': return 25;
        case 'C': case 'U': return 15;
        default:  return 30;
    }
}
int ComportamientoIngeniero::CosteEcologico(unsigned char terreno, int op)
{
    // Impacto adicional por DIG o RAISE (solo se cobra una vez por casilla)
    int eco_op = 0;
    if (op == 1) {
        switch (terreno) {
            case 'H': eco_op = 55; break;
            case 'S': eco_op = 30; break;
            case 'C': case 'U': eco_op = 10; break;
            default:  eco_op = 40; break;
        }
    } else if (op == -1) {
        switch (terreno) {
            case 'H': eco_op = 65; break;
            case 'S': eco_op = 40; break;
            case 'C': case 'U': eco_op = 25; break;
            default:  eco_op = 50; break;
        }
    }
    return eco_op; 
}
list<Paso> ComportamientoIngeniero::PlanificarRedTuberias(int belF, int belC, const vector<vector<unsigned char>>& terreno, const vector<vector<unsigned char>>& altura, int energiaMax, int ecoMax)
{
  int filas = terreno.size();
  int cols  = terreno[0].size();

  // 1. Convertir matriz de alturas a enteros para facilitar cálculos
  vector<vector<int>> alturaInt(filas, vector<int>(cols, 0));
  vector<pair<int, int>> plantasU; // Para la heurística

  for (int i = 0; i < filas; i++) {
    for (int j = 0; j < cols; j++) {
      alturaInt[i][j] = (int)altura[i][j];
      // Recopilar las posiciones de las Plantas de Tratamiento ('U')
      if (terreno[i][j] == 'U') {
          plantasU.push_back({i, j});
      }
    }
  }

  const int df[] = {-1,  1,  0,  0};
  const int dc[] = { 0,  0,  1, -1};

  // Usamos un mapa para recordar el mejor impacto ecológico con el que llegamos a cada estado
  map<tuple<int,int,int>, int> mejorEcoEnEstado;
  priority_queue<NodoTuberia> frontier;

  // 2. Generar estados iniciales (Evaluar op = 1, 0, -1 para la casilla origen)
  unsigned char tipoInicio = terreno[belF][belC];
  
  for (int op : {1, 0, -1}) {
    // No se puede excavar ni elevar en agua
    if (op != 0 && tipoInicio == 'A') continue;

    int altEfectiva = alturaInt[belF][belC] + op;
    if (altEfectiva < 0 || altEfectiva > 9) continue;

    NodoTuberia inicio;
    inicio.f            = belF;
    inicio.c            = belC;
    inicio.altura       = altEfectiva;
    inicio.g            = 0.0;
    inicio.h            = HeuristicaTuberia(belF, belC, plantasU);
    inicio.coste        = inicio.g + inicio.h;
    inicio.energiaUsada = CosteEnergeticoInstall(tipoInicio);
    inicio.ecoUsado     = CosteEcologico(tipoInicio, op); 
    
    if (inicio.energiaUsada <= energiaMax && inicio.ecoUsado <= ecoMax) {
      inicio.secuencia.push_back({belF, belC, op});
      frontier.push(inicio);
    }
  }

  // 3. Bucle principal A*
  while (!frontier.empty()) {
    NodoTuberia actual = frontier.top();
    frontier.pop();

    auto clave = make_tuple(actual.f, actual.c, actual.altura);
    
    // Poda al extraer: Si ya pasamos por aquí con un impacto mejor o igual, lo podamos
    if (mejorEcoEnEstado.count(clave) > 0 && mejorEcoEnEstado[clave] <= actual.ecoUsado) {
      continue;
    }
    // Registramos el nuevo récord ecológico para este estado
    mejorEcoEnEstado[clave] = actual.ecoUsado;

    // ¿Es Meta?
    if (terreno[actual.f][actual.c] == 'U') {
      return actual.secuencia;
    }

    // Expandir vecinos
    for (int d = 0; d < 4; d++) {
      int nf = actual.f + df[d];
      int nc = actual.c + dc[d];

      bool dentroDelMapa = (nf >= 0 && nf < filas && nc >= 0 && nc < cols);

      if (dentroDelMapa) {
        unsigned char tipo = terreno[nf][nc];
        bool casillaTubeable = (tipo != 'P' && tipo != 'M' && tipo != '?' && tipo != 'B');

        if (casillaTubeable) {
          int alturaOriginal = alturaInt[nf][nc];
          
          for (int op : {1, 0, -1}) {
            if (op != 0 && tipo == 'A') continue;

            int alturaEfectiva = alturaOriginal + op;
            if (alturaEfectiva < 0 || alturaEfectiva > 9) continue;

            bool flujoValido = (alturaEfectiva == actual.altura || alturaEfectiva == actual.altura - 1);

            if (flujoValido) {
              int nuevaEnergia = actual.energiaUsada + CosteEnergeticoInstall(tipo);
              
              unsigned char tipoAnterior = terreno[actual.f][actual.c];
              int costeTramoInstall = CosteEcoBaseInstall(tipoAnterior) + CosteEcoBaseInstall(tipo);
              int costeAlteracion = CosteEcologico(tipo, op);
              
              int nuevoEco = actual.ecoUsado + costeTramoInstall + costeAlteracion;

              bool energiaOk = (nuevaEnergia <= energiaMax);
              bool ecoOk     = (nuevoEco     <= ecoMax);

              if (energiaOk && ecoOk) {
                  auto claveHijo = make_tuple(nf, nc, alturaEfectiva);
                
                // Poda temprana: Si ya sabemos que existe un camino hacia este hijo 
                // con mejor o igual impacto, ni lo metemos en la cola
                if (mejorEcoEnEstado.count(claveHijo) == 0 || mejorEcoEnEstado[claveHijo] > nuevoEco) {
                  NodoTuberia hijo;
                  hijo.f            = nf;
                  hijo.c            = nc;
                  hijo.altura       = alturaEfectiva;
                  hijo.g            = actual.g + 1.0;
                  hijo.h            = HeuristicaTuberia(nf, nc, plantasU);
                  hijo.coste        = hijo.g + hijo.h; 
                  hijo.energiaUsada = nuevaEnergia;
                  hijo.ecoUsado     = nuevoEco;
                  hijo.secuencia    = actual.secuencia;
                  hijo.secuencia.push_back({nf, nc, op});
                  frontier.push(hijo);
                }
                
              }
            }
          }
        }
      }
    }
  }

  return list<Paso>();
}


// =========================================================================
// NIVEL 6: INGENIERO Y TÉCNICO CONSTRUYEN RED DE TUBERÍAS
// =========================================================================
  

void ComportamientoIngeniero::ActualizarPlantasDescubiertas() {
  
  // Recorremos todo el mapa que tenemos guardado en memoria
  for (int i = 0; i < mapaResultado.size(); i++) {
    for (int j = 0; j < mapaResultado[0].size(); j++) {
      // Si la casilla está descubierta y es una Planta de Tratamiento
      if (mapaResultado[i][j] == 'U') {
        plantasDescubiertas.push_back({i, j});
      }
    }
  }

}
void ComportamientoIngeniero::ActualizarBelkanita(const Sensores &sensores)
{
  if (sensores.BelPosF >= 0 && sensores.BelPosC >= 0 &&
      sensores.BelPosF < mapaResultado.size() &&
      sensores.BelPosC < mapaResultado[0].size()) {

    if (mapaResultado[sensores.BelPosF][sensores.BelPosC] == '?') {
      mapaResultado[sensores.BelPosF][sensores.BelPosC] = 'C'; 
    }
  }
}


// Niveles iniciales (Comportamientos reactivos simples)
Action ComportamientoIngeniero::ComportamientoIngenieroNivel_0(Sensores sensores)
{
  //INGENIERO BUSCA META 'U'

  Action accion = IDLE;

  ActualizarMapa(sensores);
  if (sensores.superficie[0] == 'D') tiene_zapatillas = true;
  if (sensores.superficie[0]=='U') return IDLE;
  

  ActualizarMapaPasos(sensores);
  ActualizarYDetectarBucle(sensores);
  DetectarMeta(sensores);

  

  //Se pone 'P' si las adyacentes no son accesibles por altura 
  //Se pone 'M' si las de distancia 1 y 2 estan ocupadas (por si hago jump)
  char i, c, d;
  EvaluarAlturas(sensores, i, c, d); 
  AplicarObstaculosAgentes(sensores, i, c, d);
  
  //4. Decision
  int pos = VeoCasillaInteresante(sensores, i,c, d, tiene_zapatillas);

  //5.Accion
  switch (pos) {
    case 2: 
      accion = WALK; break;
    case 1: 
      accion = TURN_SL; break;
    case 3: 
      accion = TURN_SR; break;
    default: 
      // Por si hay algún error o está rodeado de muros
      accion = TURN_SL; break;
  }

  //6.Rompe Bucles
  accion = RompeBucles_N0(accion, c, permitidas_nivel0);
  last_action=accion;

  return accion;
}

Action ComportamientoIngeniero::ComportamientoIngenieroNivel_1(Sensores sensores)
{
  // INGENIERO EXPLORA MAPA

  Action accion = IDLE;

  //0.Memoria del mapa que tengo delante
  ActualizarMapa(sensores); 
  ActualizarMapaPasos(sensores);
  ActualizarYDetectarBucle(sensores);

  //1. Memoria. Si la casilla que piso ahora mismo(0) tiene zapatillas, lo memorizo
  if (sensores.superficie[0] == 'D') tiene_zapatillas = true;

  //2. Vision. Pasamos la visión y si tenemos o no zapatillas
  char i, c, d;
  EvaluarAlturas(sensores, i, c, d);

  AplicarObstaculosAgentes(sensores, i, c, d);

  //4. Decision en base a los criterios del nivel 1
  int pos = VeoCasillaExplorable(sensores, i,c, d, tiene_zapatillas);

  //5.Accion
  switch (pos) {
    case 2: 
      accion = WALK; break;
    case 1: 
      accion = TURN_SL; break;
    case 3: 
      accion = TURN_SR; break;
    case 4:
      accion = JUMP; break;
    default: 
      // Por si hay algún error o está rodeado de muros
      accion = TURN_SL; break;
  }
  
  //6.Rompe Bucles
  accion = RompeBucles_N1(sensores, accion, c, permitidas_nivel1);

  last_action = accion;
  return accion;
}

// Niveles avanzados (Uso de búsqueda)
Action ComportamientoIngeniero::ComportamientoIngenieroNivel_2(Sensores sensores)
{
  Action accion = IDLE;

  if (!hayPlan){
    EstadoI inicio;
    inicio.site.f = sensores.posF;
    inicio.site.c = sensores.posC;
    inicio.site.brujula = sensores.rumbo;
    inicio.zapatillas= tiene_zapatillas;

    if (sensores.superficie[0] == 'D') tiene_zapatillas = true;

    EstadoI fin;
    fin.site.f = sensores.BelPosF;
    fin.site.c = sensores.BelPosC;
    
    planMovimiento = calcularRutaIngeniero (inicio, fin, mapaResultado, mapaCotas);

    if (!planMovimiento.empty()){
      VisualizaPlan (inicio.site, planMovimiento); 
      hayPlan = true;
    } 
  }

  if (hayPlan && !planMovimiento.empty()){
    accion = planMovimiento.front(); 
    planMovimiento.pop_front();
  }

  if (sensores.choque) {
    planMovimiento.clear();
    hayPlan = false;
  }

  if (planMovimiento.empty()) {
      hayPlan = false;
  }

  return accion;
}

Action ComportamientoIngeniero::ComportamientoIngenieroNivel_3(Sensores sensores)
{
  //INGENIERO EVITA INTERPONERSE EN EL CAMINO DEL TÉNICO
  Action accion = IDLE;

  // 1. Terminar la maniobra de evasion
  if (!planEscape.empty()) {
    accion = planEscape.front();
    planEscape.pop_front();
    return accion;
  }

  // 2. Radar de peligro: Buscar al Técnico en el cono de visión (1 a 15)
  bool tecnico_a_la_vista = false;
  for (int k = 1; k < 16; k++) {
    if (sensores.agentes[k] == 't') { 
      tecnico_a_la_vista = true;
      break;
    }
  }

  char i, c, d;
  EvaluarAlturas(sensores, i, c, d);
  AplicarObstaculosAgentes(sensores, i, c, d);

  // 3. Modo evasión: Si el Técnico se cruza, huimos
  if (tecnico_a_la_vista) {
    planMovimiento.clear(); 
    hayPlan = false; 

    if (i != 'M' && i != 'P' && i != 'B') { 
      planEscape.push_back(WALK);
      return TURN_SL;
    } 
    else if (d != 'M' && d != 'P' && d != 'B') { 
      planEscape.push_back(WALK);
      return TURN_SR;
    } 
    else if (c != 'M' && c != 'P' && c != 'B') {
      return WALK; 
    }
    else {
      planEscape.push_back(WALK);
      return TURN_SL;
    }
  }

  // 4. Control de Energía
  if (sensores.energia < 100) {
    planMovimiento.clear();
    hayPlan = true; // Lo mantenemos a true para no gastar CPU recalculando
    return IDLE; 
  }

  // 5. Destino temerario para el tecnico (Lejos y con Agua)
  if (!hayPlan) {
    int mejor_puntuacion = -1;
    int temerarioF = sensores.posF;
    int temerarioC = sensores.posC;
    
    int filas = mapaResultado.size();
    int cols = mapaResultado[0].size();

    for (int f = 0; f < filas; f++) {
      for (int col = 0; col < cols; col++) {
        char terr = mapaResultado[f][col];
        
        if (terr != 'M' && terr != 'P' && terr != 'B' && terr != '?') {
          int dist = abs(f - sensores.BelPosF) + abs(col - sensores.BelPosC);
          int puntuacion = dist;

          if (terr == 'A') {
            puntuacion += 1000; 
          }

          if (puntuacion > mejor_puntuacion) {
            mejor_puntuacion = puntuacion;
            temerarioF = f;
            temerarioC = col;
          }
        }
      }
    }

    if (sensores.posF == temerarioF && sensores.posC == temerarioC) {
        hayPlan = true;
    } else {
        EstadoI inicio {ubicacion{sensores.posF, sensores.posC, sensores.rumbo}, this->tiene_zapatillas};
        EstadoI fin    {ubicacion{temerarioF, temerarioC, norte}, this->tiene_zapatillas};
        
        planMovimiento = calcularRutaIngeniero(inicio, fin, mapaResultado, mapaCotas);
        
        hayPlan = true; 
        
        if (planMovimiento.empty()) {
            return TURN_SL; 
        }
    }
  }

  // 6. Ejecutamos el plan
  if (hayPlan && !planMovimiento.empty()) {
    accion = planMovimiento.front();
    planMovimiento.pop_front();
  }

  // 7. Llegamos y nos quedamos girando
  if (hayPlan && planMovimiento.empty()) {
     accion = TURN_SL;
  }

  return accion;
}

Action ComportamientoIngeniero::ComportamientoIngenieroNivel_4(Sensores sensores)
{
  //INGENIERO PLANIFICA LA RED DE TUBERÍAS

  Action accion = IDLE;
  if (!hayPlan) {
    planTuberias = PlanificarRedTuberias(sensores.BelPosF, sensores.BelPosC,mapaResultado, mapaCotas,sensores.energia, sensores.max_ecologico);
    VisualizaRedTuberias(planTuberias);
    hayPlan = true;
  }
  return accion; 
}

Action ComportamientoIngeniero::ComportamientoIngenieroNivel_5(Sensores sensores)
{
  //INGENIERO CONSTRUYE CON TÉCNICO RED DE TUBERÍAS

  Action accion = IDLE;
  ActualizarMapa(sensores);

  switch (estadoObra) {

    case PLANIFICANDO:
      if (planTuberias.empty()) {
        planTuberias = PlanificarRedTuberias(sensores.BelPosF, sensores.BelPosC, mapaResultado, mapaCotas,sensores.energia, sensores.max_ecologico);
      }
      estadoObra = CAMINO_PRIMERA_CASILLA;
      break;

    //Se ejecuta solo una vez
    case CAMINO_PRIMERA_CASILLA: {
      if (planTuberias.empty()) { estadoObra = FINALIZADO; break; }
      Paso origen = *planTuberias.begin();

      if (planMovimiento.empty()) {
        if (sensores.posF == origen.fil && sensores.posC == origen.col) {
          estadoObra = ADECUANDO_PRIMERA_CASILLA;
          break;
        }
        EstadoI estadoInicial {ubicacion{sensores.posF, sensores.posC, sensores.rumbo}, this->tiene_zapatillas};
        EstadoI estadoFinal   {ubicacion{origen.fil, origen.col, sur}, this->tiene_zapatillas};
        planMovimiento = calcularRutaIngeniero(estadoInicial, estadoFinal, mapaResultado, mapaCotas);
      }

      if (!planMovimiento.empty()) {
        accion = planMovimiento.front();
        planMovimiento.pop_front();
      } else {
        estadoObra = ADECUANDO_PRIMERA_CASILLA;
      }
      break;
    }

    case ADECUANDO_PRIMERA_CASILLA: {
      Paso origen = *planTuberias.begin();
      if (origen.op == 1)       accion = RAISE;
      else if (origen.op == -1) accion = DIG;
      estadoObra = LLAMANDO_TECNICO;
      break;
    }

    // BUCLE PRINCIPAL DE INSTALACIÓN
    case LLAMANDO_TECNICO:
      accion = COME; // Llama al Técnico a la casilla actual
      estadoObra = CAMINO_PASO_PROPIO;
      break;

    case CAMINO_PASO_PROPIO: {
      if (planTuberias.size() < 2) { estadoObra = FINALIZADO; break; }
      Paso destino = *(next(planTuberias.begin())); // Segunda posición de la lista

      if (planMovimiento.empty()) {
        if (sensores.posF == destino.fil && sensores.posC == destino.col) {
          estadoObra = ADECUANDO_PASO_PROPIO;
          break;
        }
        EstadoI estadoInicial {ubicacion{sensores.posF, sensores.posC, sensores.rumbo}, this->tiene_zapatillas};
        EstadoI estadoFinal   {ubicacion{destino.fil, destino.col, sur}, this->tiene_zapatillas};
        planMovimiento = calcularRutaIngeniero(estadoInicial, estadoFinal, mapaResultado, mapaCotas);
      }

      if (!planMovimiento.empty()) {
        accion = planMovimiento.front();
        planMovimiento.pop_front();
      } else {
        estadoObra = ADECUANDO_PASO_PROPIO;
      }
      break;
    }

    case ADECUANDO_PASO_PROPIO: {
      Paso destino = *(next(planTuberias.begin()));
      if (destino.op == 1)       accion = RAISE;
      else if (destino.op == -1) accion = DIG;
      estadoObra = ALINEANDO_CON_TECNICO;
      break;
    }

    case ALINEANDO_CON_TECNICO: {
      if (sensores.enfrente) {
        accion = INSTALL;
        planTuberias.pop_front();
        planMovimiento.clear();
        estadoObra = (planTuberias.size() < 2) ? FINALIZADO : LLAMANDO_TECNICO;
        break;
      }
      
      Paso origen = *planTuberias.begin(); // Donde está el Técnico
      int rumboDeseado;
      
      if      (origen.fil < sensores.posF) rumboDeseado = 0; // Norte
      else if (origen.fil > sensores.posF) rumboDeseado = 4; // Sur
      else if (origen.col > sensores.posC) rumboDeseado = 2; // Este
      else                                 rumboDeseado = 6; // Oeste

      if (sensores.rumbo == rumboDeseado) {
        accion = COME; 
        estadoObra = INSTALANDO;
      } else {
        // Elegimos el giro más corto
        int diff = (rumboDeseado - sensores.rumbo + 8) % 8;
        accion = (diff <= 4) ? TURN_SR : TURN_SL;
      }
      break;
    }

    case INSTALANDO: {
      if (planTuberias.size() < 2) { estadoObra = FINALIZADO; break; }
      
      if (sensores.enfrente) {
        accion = INSTALL;
        planTuberias.pop_front();
        planMovimiento.clear();
        estadoObra = (planTuberias.size() < 2) ? FINALIZADO : LLAMANDO_TECNICO;
      } else {
        accion = IDLE;
      }
      break;
    }

    case FINALIZADO:
      accion = IDLE;
      break;
  }

  return accion;
}

Action ComportamientoIngeniero::ComportamientoIngenieroNivel_6(Sensores sensores) {
  Action accion = IDLE;
  ActualizarMapa(sensores); 
  ActualizarBelkanita(sensores);
  ActualizarPlantasDescubiertas();

  switch (estadoNivel6) { 
    case FASE_EXPLORACION: {
      // 1. Comprobamos que conocemos la Belkanita (y está descubierta en el mapa)
      bool belkanita_conocida = (sensores.BelPosF != -1 && sensores.BelPosC != -1 && 
                                 mapaResultado[sensores.BelPosF][sensores.BelPosC] != '?');

      // 2. Usamos la vida (iteraciones restantes). 
      if (belkanita_conocida && !plantasDescubiertas.empty() && (sensores.vida % 50 == 0)) {
        estadoNivel6 = FASE_PLANIFICACION;
        break;
      }
      
      // Si no es el momento o falta información, seguimos explorando
      accion = ComportamientoIngenieroNivel_1(sensores);
      break;
    }

    case FASE_PLANIFICACION: {
      planTuberias = PlanificarRedTuberias(sensores.BelPosF, sensores.BelPosC, mapaResultado, mapaCotas, sensores.energia, sensores.max_ecologico);

      if (!planTuberias.empty()) {
        estadoNivel6 = FASE_EJECUCION;
      } else {
        // Al fallar, vuelve a explorar.
        estadoNivel6 = FASE_EXPLORACION;
      }
      break;
    }

    case FASE_EJECUCION: {
      accion = ComportamientoIngenieroNivel_5(sensores);

      if (estadoObra == FINALIZADO) {
        estadoNivel6 = FASE_FINALIZACION;
      }
      break;
    }

    case FASE_FINALIZACION:
      accion = IDLE;
      break;
  }

  return accion;
}


// =========================================================================
// FUNCIONES PROPORCIONADAS
// =========================================================================

/**
 * @brief Actualiza el mapaResultado y mapaCotas con la información de los sensores.
 * @param sensores Datos actuales de los sensores.
 */
void ComportamientoIngeniero::ActualizarMapa(Sensores sensores)
{
  mapaResultado[sensores.posF][sensores.posC] = sensores.superficie[0];
  mapaCotas[sensores.posF][sensores.posC] = sensores.cota[0];

  int pos = 1;
  switch (sensores.rumbo)
  {
  case norte:
    for (int j = 1; j < 4; j++)
      for (int i = -j; i <= j; i++)
      {
        mapaResultado[sensores.posF - j][sensores.posC + i] = sensores.superficie[pos];
        mapaCotas[sensores.posF - j][sensores.posC + i] = sensores.cota[pos++];
      }
    break;
  case noreste:
    mapaResultado[sensores.posF - 1][sensores.posC] = sensores.superficie[1];
    mapaCotas[sensores.posF - 1][sensores.posC] = sensores.cota[1];
    mapaResultado[sensores.posF - 1][sensores.posC + 1] = sensores.superficie[2];
    mapaCotas[sensores.posF - 1][sensores.posC + 1] = sensores.cota[2];
    mapaResultado[sensores.posF][sensores.posC + 1] = sensores.superficie[3];
    mapaCotas[sensores.posF][sensores.posC + 1] = sensores.cota[3];
    mapaResultado[sensores.posF - 2][sensores.posC] = sensores.superficie[4];
    mapaCotas[sensores.posF - 2][sensores.posC] = sensores.cota[4];
    mapaResultado[sensores.posF - 2][sensores.posC + 1] = sensores.superficie[5];
    mapaCotas[sensores.posF - 2][sensores.posC + 1] = sensores.cota[5];
    mapaResultado[sensores.posF - 2][sensores.posC + 2] = sensores.superficie[6];
    mapaCotas[sensores.posF - 2][sensores.posC + 2] = sensores.cota[6];
    mapaResultado[sensores.posF - 1][sensores.posC + 2] = sensores.superficie[7];
    mapaCotas[sensores.posF - 1][sensores.posC + 2] = sensores.cota[7];
    mapaResultado[sensores.posF][sensores.posC + 2] = sensores.superficie[8];
    mapaCotas[sensores.posF][sensores.posC + 2] = sensores.cota[8];
    mapaResultado[sensores.posF - 3][sensores.posC] = sensores.superficie[9];
    mapaCotas[sensores.posF - 3][sensores.posC] = sensores.cota[9];
    mapaResultado[sensores.posF - 3][sensores.posC + 1] = sensores.superficie[10];
    mapaCotas[sensores.posF - 3][sensores.posC + 1] = sensores.cota[10];
    mapaResultado[sensores.posF - 3][sensores.posC + 2] = sensores.superficie[11];
    mapaCotas[sensores.posF - 3][sensores.posC + 2] = sensores.cota[11];
    mapaResultado[sensores.posF - 3][sensores.posC + 3] = sensores.superficie[12];
    mapaCotas[sensores.posF - 3][sensores.posC + 3] = sensores.cota[12];
    mapaResultado[sensores.posF - 2][sensores.posC + 3] = sensores.superficie[13];
    mapaCotas[sensores.posF - 2][sensores.posC + 3] = sensores.cota[13];
    mapaResultado[sensores.posF - 1][sensores.posC + 3] = sensores.superficie[14];
    mapaCotas[sensores.posF - 1][sensores.posC + 3] = sensores.cota[14];
    mapaResultado[sensores.posF][sensores.posC + 3] = sensores.superficie[15];
    mapaCotas[sensores.posF][sensores.posC + 3] = sensores.cota[15];
    break;
  case este:
    for (int j = 1; j < 4; j++)
      for (int i = -j; i <= j; i++)
      {
        mapaResultado[sensores.posF + i][sensores.posC + j] = sensores.superficie[pos];
        mapaCotas[sensores.posF + i][sensores.posC + j] = sensores.cota[pos++];
      }
    break;
  case sureste:
    mapaResultado[sensores.posF][sensores.posC + 1] = sensores.superficie[1];
    mapaCotas[sensores.posF][sensores.posC + 1] = sensores.cota[1];
    mapaResultado[sensores.posF + 1][sensores.posC + 1] = sensores.superficie[2];
    mapaCotas[sensores.posF + 1][sensores.posC + 1] = sensores.cota[2];
    mapaResultado[sensores.posF + 1][sensores.posC] = sensores.superficie[3];
    mapaCotas[sensores.posF + 1][sensores.posC] = sensores.cota[3];
    mapaResultado[sensores.posF][sensores.posC + 2] = sensores.superficie[4];
    mapaCotas[sensores.posF][sensores.posC + 2] = sensores.cota[4];
    mapaResultado[sensores.posF + 1][sensores.posC + 2] = sensores.superficie[5];
    mapaCotas[sensores.posF + 1][sensores.posC + 2] = sensores.cota[5];
    mapaResultado[sensores.posF + 2][sensores.posC + 2] = sensores.superficie[6];
    mapaCotas[sensores.posF + 2][sensores.posC + 2] = sensores.cota[6];
    mapaResultado[sensores.posF + 2][sensores.posC + 1] = sensores.superficie[7];
    mapaCotas[sensores.posF + 2][sensores.posC + 1] = sensores.cota[7];
    mapaResultado[sensores.posF + 2][sensores.posC] = sensores.superficie[8];
    mapaCotas[sensores.posF + 2][sensores.posC] = sensores.cota[8];
    mapaResultado[sensores.posF][sensores.posC + 3] = sensores.superficie[9];
    mapaCotas[sensores.posF][sensores.posC + 3] = sensores.cota[9];
    mapaResultado[sensores.posF + 1][sensores.posC + 3] = sensores.superficie[10];
    mapaCotas[sensores.posF + 1][sensores.posC + 3] = sensores.cota[10];
    mapaResultado[sensores.posF + 2][sensores.posC + 3] = sensores.superficie[11];
    mapaCotas[sensores.posF + 2][sensores.posC + 3] = sensores.cota[11];
    mapaResultado[sensores.posF + 3][sensores.posC + 3] = sensores.superficie[12];
    mapaCotas[sensores.posF + 3][sensores.posC + 3] = sensores.cota[12];
    mapaResultado[sensores.posF + 3][sensores.posC + 2] = sensores.superficie[13];
    mapaCotas[sensores.posF + 3][sensores.posC + 2] = sensores.cota[13];
    mapaResultado[sensores.posF + 3][sensores.posC + 1] = sensores.superficie[14];
    mapaCotas[sensores.posF + 3][sensores.posC + 1] = sensores.cota[14];
    mapaResultado[sensores.posF + 3][sensores.posC] = sensores.superficie[15];
    mapaCotas[sensores.posF + 3][sensores.posC] = sensores.cota[15];
    break;
  case sur:
    for (int j = 1; j < 4; j++)
      for (int i = -j; i <= j; i++)
      {
        mapaResultado[sensores.posF + j][sensores.posC - i] = sensores.superficie[pos];
        mapaCotas[sensores.posF + j][sensores.posC - i] = sensores.cota[pos++];
      }
    break;
  case suroeste:
    mapaResultado[sensores.posF + 1][sensores.posC] = sensores.superficie[1];
    mapaCotas[sensores.posF + 1][sensores.posC] = sensores.cota[1];
    mapaResultado[sensores.posF + 1][sensores.posC - 1] = sensores.superficie[2];
    mapaCotas[sensores.posF + 1][sensores.posC - 1] = sensores.cota[2];
    mapaResultado[sensores.posF][sensores.posC - 1] = sensores.superficie[3];
    mapaCotas[sensores.posF][sensores.posC - 1] = sensores.cota[3];
    mapaResultado[sensores.posF + 2][sensores.posC] = sensores.superficie[4];
    mapaCotas[sensores.posF + 2][sensores.posC] = sensores.cota[4];
    mapaResultado[sensores.posF + 2][sensores.posC - 1] = sensores.superficie[5];
    mapaCotas[sensores.posF + 2][sensores.posC - 1] = sensores.cota[5];
    mapaResultado[sensores.posF + 2][sensores.posC - 2] = sensores.superficie[6];
    mapaCotas[sensores.posF + 2][sensores.posC - 2] = sensores.cota[6];
    mapaResultado[sensores.posF + 1][sensores.posC - 2] = sensores.superficie[7];
    mapaCotas[sensores.posF + 1][sensores.posC - 2] = sensores.cota[7];
    mapaResultado[sensores.posF][sensores.posC - 2] = sensores.superficie[8];
    mapaCotas[sensores.posF][sensores.posC - 2] = sensores.cota[8];
    mapaResultado[sensores.posF + 3][sensores.posC] = sensores.superficie[9];
    mapaCotas[sensores.posF + 3][sensores.posC] = sensores.cota[9];
    mapaResultado[sensores.posF + 3][sensores.posC - 1] = sensores.superficie[10];
    mapaCotas[sensores.posF + 3][sensores.posC - 1] = sensores.cota[10];
    mapaResultado[sensores.posF + 3][sensores.posC - 2] = sensores.superficie[11];
    mapaCotas[sensores.posF + 3][sensores.posC - 2] = sensores.cota[11];
    mapaResultado[sensores.posF + 3][sensores.posC - 3] = sensores.superficie[12];
    mapaCotas[sensores.posF + 3][sensores.posC - 3] = sensores.cota[12];
    mapaResultado[sensores.posF + 2][sensores.posC - 3] = sensores.superficie[13];
    mapaCotas[sensores.posF + 2][sensores.posC - 3] = sensores.cota[13];
    mapaResultado[sensores.posF + 1][sensores.posC - 3] = sensores.superficie[14];
    mapaCotas[sensores.posF + 1][sensores.posC - 3] = sensores.cota[14];
    mapaResultado[sensores.posF][sensores.posC - 3] = sensores.superficie[15];
    mapaCotas[sensores.posF][sensores.posC - 3] = sensores.cota[15];
    break;
  case oeste:
    for (int j = 1; j < 4; j++)
      for (int i = -j; i <= j; i++)
      {
        mapaResultado[sensores.posF - i][sensores.posC - j] = sensores.superficie[pos];
        mapaCotas[sensores.posF - i][sensores.posC - j] = sensores.cota[pos++];
      }
    break;
  case noroeste:
    mapaResultado[sensores.posF][sensores.posC - 1] = sensores.superficie[1];
    mapaCotas[sensores.posF][sensores.posC - 1] = sensores.cota[1];
    mapaResultado[sensores.posF - 1][sensores.posC - 1] = sensores.superficie[2];
    mapaCotas[sensores.posF - 1][sensores.posC - 1] = sensores.cota[2];
    mapaResultado[sensores.posF - 1][sensores.posC] = sensores.superficie[3];
    mapaCotas[sensores.posF - 1][sensores.posC] = sensores.cota[3];
    mapaResultado[sensores.posF][sensores.posC - 2] = sensores.superficie[4];
    mapaCotas[sensores.posF][sensores.posC - 2] = sensores.cota[4];
    mapaResultado[sensores.posF - 1][sensores.posC - 2] = sensores.superficie[5];
    mapaCotas[sensores.posF - 1][sensores.posC - 2] = sensores.cota[5];
    mapaResultado[sensores.posF - 2][sensores.posC - 2] = sensores.superficie[6];
    mapaCotas[sensores.posF - 2][sensores.posC - 2] = sensores.cota[6];
    mapaResultado[sensores.posF - 2][sensores.posC - 1] = sensores.superficie[7];
    mapaCotas[sensores.posF - 2][sensores.posC - 1] = sensores.cota[7];
    mapaResultado[sensores.posF - 2][sensores.posC] = sensores.superficie[8];
    mapaCotas[sensores.posF - 2][sensores.posC] = sensores.cota[8];
    mapaResultado[sensores.posF][sensores.posC - 3] = sensores.superficie[9];
    mapaCotas[sensores.posF][sensores.posC - 3] = sensores.cota[9];
    mapaResultado[sensores.posF - 1][sensores.posC - 3] = sensores.superficie[10];
    mapaCotas[sensores.posF - 1][sensores.posC - 3] = sensores.cota[10];
    mapaResultado[sensores.posF - 2][sensores.posC - 3] = sensores.superficie[11];
    mapaCotas[sensores.posF - 2][sensores.posC - 3] = sensores.cota[11];
    mapaResultado[sensores.posF - 3][sensores.posC - 3] = sensores.superficie[12];
    mapaCotas[sensores.posF - 3][sensores.posC - 3] = sensores.cota[12];
    mapaResultado[sensores.posF - 3][sensores.posC - 2] = sensores.superficie[13];
    mapaCotas[sensores.posF - 3][sensores.posC - 2] = sensores.cota[13];
    mapaResultado[sensores.posF - 3][sensores.posC - 1] = sensores.superficie[14];
    mapaCotas[sensores.posF - 3][sensores.posC - 1] = sensores.cota[14];
    mapaResultado[sensores.posF - 3][sensores.posC] = sensores.superficie[15];
    mapaCotas[sensores.posF - 3][sensores.posC] = sensores.cota[15];
    break;
  }
}

/**
 * @brief Determina si una casilla es transitable para el ingeniero.
 * @param f Fila de la casilla.
 * @param c Columna de la casilla.
 * @param tieneZapatillas Indica si el agente posee las zapatillas.
 * @return true si la casilla es transitable (no es muro ni precipicio).
 */
bool ComportamientoIngeniero::EsCasillaTransitableLevel0(int f, int c, bool tieneZapatillas)
{
  if (f < 0 || f >= mapaResultado.size() || c < 0 || c >= mapaResultado[0].size())
    return false;
  return EsCamino(mapaResultado[f][c]); // Solo 'C', 'D', 'U' son transitables en Nivel 0
}

/**
 * @brief Comprueba si la casilla de delante es accesible por diferencia de altura.
 * Para el ingeniero: desnivel máximo 1 sin zapatillas, 2 con zapatillas.
 * @param actual Estado actual del agente (fila, columna, orientacion, zap).
 * @return true si el desnivel con la casilla de delante es admisible.
 */
bool ComportamientoIngeniero::EsAccesiblePorAltura(const ubicacion &actual, bool zap)
{
  ubicacion del = Delante(actual);
  if (del.f < 0 || del.f >= mapaCotas.size() || del.c < 0 || del.c >= mapaCotas[0].size())
    return false;
  int desnivel = abs(mapaCotas[del.f][del.c] - mapaCotas[actual.f][actual.c]);
  if (zap && desnivel > 2)
    return false;
  if (!zap && desnivel > 1)
    return false;
  return true;
}

/**
 * @brief Devuelve la posición (fila, columna) de la casilla que hay delante del agente.
 * Calcula la casilla frontal según la orientación actual (8 direcciones).
 * @param actual Estado actual del agente (fila, columna, orientacion).
 * @return Estado con la fila y columna de la casilla de enfrente.
 */
ubicacion ComportamientoIngeniero::Delante(const ubicacion &actual) const
{
  ubicacion delante = actual;
  switch (actual.brujula)
  {
  case 0:
    delante.f--;
    break; // norte
  case 1:
    delante.f--;
    delante.c++;
    break; // noreste
  case 2:
    delante.c++;
    break; // este
  case 3:
    delante.f++;
    delante.c++;
    break; // sureste
  case 4:
    delante.f++;
    break; // sur
  case 5:
    delante.f++;
    delante.c--;
    break; // suroeste
  case 6:
    delante.c--;
    break; // oeste
  case 7:
    delante.f--;
    delante.c--;
    break; // noroeste
  }
  return delante;
}

/**
 * @brief Imprime por consola la secuencia de acciones de un plan.
 *
 * @param plan  Lista de acciones del plan.
 */
void ComportamientoIngeniero::PintaPlan(const list<Action> &plan)
{
  auto it = plan.begin();
  while (it != plan.end())
  {
    if (*it == WALK)
    {
      cout << "W ";
    }
    else if (*it == JUMP)
    {
      cout << "J ";
    }
    else if (*it == TURN_SR)
    {
      cout << "r ";
    }
    else if (*it == TURN_SL)
    {
      cout << "l ";
    }
    else if (*it == COME)
    {
      cout << "C ";
    }
    else if (*it == IDLE)
    {
      cout << "I ";
    }
    else
    {
      cout << "-_ ";
    }
    it++;
  }
  cout << "( longitud " << plan.size() << ")" << endl;
}

/**
 * @brief Imprime las coordenadas y operaciones de un plan de tubería.
 *
 * @param plan  Lista de pasos (fila, columna, operación),
 *              donde operacion = -1 (DIG), operación = 1 (RAISE).
 */
void ComportamientoIngeniero::PintaPlan(const list<Paso> &plan)
{
  auto it = plan.begin();
  while (it != plan.end())
  {
    cout << it->fil << ", " << it->col << " (" << it->op << ")\n";
    it++;
  }
  cout << "( longitud " << plan.size() << ")" << endl;
}

/**
 * @brief Convierte un plan de acciones en una lista de casillas para
 *        su visualización en el mapa 2D.
 *
 * @param st    Estado de partida.
 * @param plan  Lista de acciones del plan.
 */
void ComportamientoIngeniero::VisualizaPlan(const ubicacion &st,
                                            const list<Action> &plan)
{
  listaPlanCasillas.clear();
  ubicacion cst = st;

  listaPlanCasillas.push_back({cst.f, cst.c, WALK});
  auto it = plan.begin();
  while (it != plan.end())
  {

    switch (*it)
    {
    case JUMP:
      switch (cst.brujula)
      {
      case 0:
        cst.f--;
        break;
      case 1:
        cst.f--;
        cst.c++;
        break;
      case 2:
        cst.c++;
        break;
      case 3:
        cst.f++;
        cst.c++;
        break;
      case 4:
        cst.f++;
        break;
      case 5:
        cst.f++;
        cst.c--;
        break;
      case 6:
        cst.c--;
        break;
      case 7:
        cst.f--;
        cst.c--;
        break;
      }
      if (cst.f >= 0 && cst.f < mapaResultado.size() &&
          cst.c >= 0 && cst.c < mapaResultado[0].size())
        listaPlanCasillas.push_back({cst.f, cst.c, JUMP});
    case WALK:
      switch (cst.brujula)
      {
      case 0:
        cst.f--;
        break;
      case 1:
        cst.f--;
        cst.c++;
        break;
      case 2:
        cst.c++;
        break;
      case 3:
        cst.f++;
        cst.c++;
        break;
      case 4:
        cst.f++;
        break;
      case 5:
        cst.f++;
        cst.c--;
        break;
      case 6:
        cst.c--;
        break;
      case 7:
        cst.f--;
        cst.c--;
        break;
      }
      if (cst.f >= 0 && cst.f < mapaResultado.size() &&
          cst.c >= 0 && cst.c < mapaResultado[0].size())
        listaPlanCasillas.push_back({cst.f, cst.c, WALK});
      break;
    case TURN_SR:
      cst.brujula = (Orientacion) (( (int) cst.brujula + 1) % 8);
      break;
    case TURN_SL:
      cst.brujula = (Orientacion) (( (int) cst.brujula + 7) % 8);
      break;
    }
    it++;
  }
}

/**
 * @brief Convierte un plan de tubería en la lista de casillas usada
 *        por el sistema de visualización.
 *
 * @param st    Estado de partida (no utilizado directamente).
 * @param plan  Lista de pasos del plan de tubería.
 */
void ComportamientoIngeniero::VisualizaRedTuberias(const list<Paso> &plan)
{
  listaCanalizacionTuberias.clear();
  auto it = plan.begin();
  while (it != plan.end())
  {
    listaCanalizacionTuberias.push_back({it->fil, it->col, it->op});
    it++;
  }
}

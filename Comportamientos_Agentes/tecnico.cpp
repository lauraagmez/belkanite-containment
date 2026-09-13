#include "tecnico.hpp"
#include "motorlib/util.h"
#include <iostream>
#include <queue>
#include <set>
#include <string>
#include <map>
#include <algorithm>

using namespace std;

// =========================================================================
// ÁREA DE IMPLEMENTACIÓN DEL ESTUDIANTE
// =========================================================================



Action ComportamientoTecnico::think(Sensores sensores) {
  Action accion = IDLE;


  // Decisión del agente según el nivel
  switch (sensores.nivel) {
    case 0: accion = ComportamientoTecnicoNivel_0(sensores); break;
    case 1: accion = ComportamientoTecnicoNivel_1(sensores); break;
    case 2: accion = ComportamientoTecnicoNivel_2(sensores); break;
    case 3: accion = ComportamientoTecnicoNivel_3(sensores); break;
    case 4: accion = ComportamientoTecnicoNivel_4(sensores); break;
    case 5: accion = ComportamientoTecnicoNivel_5(sensores); break;
    case 6: accion = ComportamientoTecnicoNivel_6(sensores); break;
  }

  return accion;
}

//CONTADOR PARA LLEVAR UN REGISTRO DE LAS CASILLAS MENOS VISITADAS
void ComportamientoTecnico::ActualizarMapaPasos (const Sensores &sensores){
  if (sensores.posF != ultima_pos.first || sensores.posC != ultima_pos.second) {
    mapaPasos[sensores.posF][sensores.posC]++;
    ultima_pos = {sensores.posF, sensores.posC};
  }
}

//NIVEL 0, 1: CAMINO A LA META | EXPLORANDO MAPA

char ComportamientoTecnico::ViablePorAltura (char casilla, int dif, bool tiene_zaptillas){
  if (abs(dif)<=1) return casilla;
  else return 'P';
}

void ComportamientoTecnico::EvaluarAlturas(const Sensores &sensores, char &i, char &c, char &d) {
  i = ViablePorAltura(sensores.superficie[1], sensores.cota[1] - sensores.cota[0], tiene_zapatillas);
  c = ViablePorAltura(sensores.superficie[2], sensores.cota[2] - sensores.cota[0], tiene_zapatillas);
  d = ViablePorAltura(sensores.superficie[3], sensores.cota[3] - sensores.cota[0], tiene_zapatillas);
}

void ComportamientoTecnico::AplicarObstaculosAgentes(const Sensores &sensores, char &i, char &c, char &d) {
  // Si hay un agente cerca de mi, me tomo la casilla como un muro
  if (sensores.agentes[1] != '_') i = 'M';
  if (sensores.agentes[2] != '_') c = 'M';
  if (sensores.agentes[3] != '_') d = 'M';
}

ubicacion ComportamientoTecnico::GirarIzquierda(ubicacion p) const{
  p.brujula = (Orientacion)(((int)p.brujula + 7) % 8);
  return p;
}

ubicacion ComportamientoTecnico::GirarDerecha(ubicacion p) const{
  p.brujula = (Orientacion)(((int)p.brujula + 1) % 8);
  return p;
}

bool ComportamientoTecnico::EsCamino(unsigned char c) const {
  return (c == 'C' || c == 'D' || c == 'U');
}

Action ComportamientoTecnico::RompeBucles(Action accion_propuesta, char c, const string &transitables) {

  // 1. Guardamos la accion que ejecutaríamos
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
        // Si no podemos avanzar, cambiamos el sentido del giro
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

int ComportamientoTecnico::ElegirMenosVisitada(const Sensores &sensores, char i, char c, char d, string permitidas){
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
      if (mapaPasos[cen.f][cen.c] < minPasos || (mapaPasos[cen.f][cen.c] == minPasos && posC < mejorCosto)) {
        minPasos = mapaPasos[cen.f][cen.c];
        mejorCosto = posC; // Guardamos el coste de esta casilla
        mejor = 2;
      }
    }
  }

  // 2. IZQUIERDA
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

int ComportamientoTecnico::VeoCasillaInteresante (const Sensores &sensores, char i, char c, char d, bool tiene_zapatillas){
  // 1. Asegurarnos de que el paso inmediato es legal y viable por altura
  string permitidas = "UDC";
  bool izq_valida = (permitidas.find(i) != string::npos);
  bool cen_valida = (permitidas.find(c) != string::npos);
  bool der_valida = (permitidas.find(d) != string::npos);
  
  // PRIORIDAD 1: Meta INMEDIATA ('U') a distancia 1
  if (cen_valida && c == 'U' && sensores.agentes[2] == '_') return 2;
  if (izq_valida && i == 'U' && sensores.agentes[1] == '_') return 1;
  if (der_valida && d == 'U' && sensores.agentes[3] == '_') return 3;

  // PRIORIDAD 2: Meta a lo lejos (Distancias 2 y 3)
  if (cen_valida && (sensores.superficie[6] == 'U' || sensores.superficie[12] == 'U')) return 2;
  
  if (izq_valida && (sensores.superficie[4] == 'U' || sensores.superficie[5] == 'U' || 
                     sensores.superficie[9] == 'U' || sensores.superficie[10] == 'U' || 
                     sensores.superficie[11] == 'U')) return 1;
                     
  if (der_valida && (sensores.superficie[7] == 'U' || sensores.superficie[8] == 'U' || 
                     sensores.superficie[13] == 'U' || sensores.superficie[14] == 'U' || 
                     sensores.superficie[15] == 'U')) return 3;
  
  // PRIORIDAD 3: Si no vemos la meta por ninguna parte, avanzamos intentando no repetir casillas
  return ElegirMenosVisitada(sensores, i, c, d, permitidas);
}

int ComportamientoTecnico::VeoCasillaExplorable(const Sensores &sensores, char i, char c, char d, bool tiene_zapatillas){
  if (c == 'X') return 2;
  if (i == 'X') return 1;
  if (d == 'X') return 3;
  
  if (!tiene_zapatillas){
    if (c == 'D') return 2;
    if (i == 'D') return 1;
    if (d == 'D') return 3;
  }
  if (c == '?') return 2;
  if (i == '?') return 1;
  if (d == '?') return 3;

  if (tiene_zapatillas) {
    permitidas_nivel1 = "?BCDUXSHA"; // Ponemos la 'B' al principio (ahora tiene coste 1)
  }

  return ElegirMenosVisitada(sensores, i, c, d, permitidas_nivel1);
}


//NIVEL 3: CAMINO A BELKANITA CON MINIMO COSTE ENERGÍA

EstadoT ComportamientoTecnico::NextCasillaTecnico (const EstadoT &st, Action accion){
  EstadoT siguiente = st;
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
  return siguiente;
}
bool ComportamientoTecnico::CasillaAccesibleTecnico(const EstadoT &st, const vector<vector<unsigned char>> &terreno, const vector<vector<unsigned char>> &altura, Action accion){
  EstadoT siguiente = NextCasillaTecnico(st, accion); //queremos saber que el proximo paso de nuestra secuencia es accesible 
  
  // 0. Comprobar que no nos salimos del mapa
  if (siguiente.site.f < 0 || siguiente.site.f >= terreno.size() || siguiente.site.c < 0 || siguiente.site.c >= terreno[0].size()) {
    return false;
  }
  
  // 2. Obtener el terreno original y calcular la diferencia de altura
  char terrDest = terreno[siguiente.site.f][siguiente.site.c];
  int dif = altura[siguiente.site.f][siguiente.site.c] - altura[st.site.f][st.site.c];

  // 3. Filtrar por altura 
  if (abs(dif)>1) return false;

  // 4. Comprobar si el terreno resultante es transitable para el Técnico
  if (terrDest == 'M' || terrDest == 'P' ) return false;

  // 5. Regla especial para Hierba
  if (terrDest =='B' && !st.zapatillas) return false;
  
  return true;
}
EstadoT ComportamientoTecnico::applyT(Action accion, const EstadoT &st, const vector<vector<unsigned char>> &terreno, const vector<vector<unsigned char>> &altura) {
  EstadoT siguiente = st;
  switch(accion){
    case WALK:
      if (CasillaAccesibleTecnico(st,terreno,altura, accion)){
        siguiente = NextCasillaTecnico(st, accion);
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
double ComportamientoTecnico::CalcularCosteEnergia(const EstadoT &origen, const EstadoT &destino, Action accion, const vector<vector<unsigned char>> &terreno, const vector<vector<unsigned char>> &altura){
  double coste = 0.0;
  // 1. Identificamos el terreno desde el que partimos
  char terr_origen = terreno[origen.site.f][origen.site.c];
  
  // 2. Magia de las zapatillas: El Bosque ('B') se transforma en Camino ('C')
  if (terr_origen == 'B' && origen.zapatillas) {
    terr_origen = 'C'; 
  }
  
  switch (accion) {
    case WALK: {
      if (terr_origen == 'A') coste = 60;
      else if (terr_origen == 'H') coste = 6;
      else if (terr_origen == 'S') coste = 3;
      else coste = 1; // Resto de casillas ('C', 'X', 'D', 'U', 'B' con zapatillas...)
      
      //Si hay un cambio de altura
      if (terr_origen == 'A' || terr_origen == 'H' || terr_origen == 'S') {
        int diff_alt = altura[destino.site.f][destino.site.c] - altura[origen.site.f][origen.site.c];
        if (diff_alt > 0) coste += 5; // Cuesta arriba
        else if (diff_alt < 0) coste -= 2; // Cuesta abajo
      }//( el resto tiene +0/-0)
      break;
    }
    case TURN_SR:
    case TURN_SL: {
      if (terr_origen == 'A') coste = 5;
      else if (terr_origen == 'H') coste = 2;
      else coste = 1; // Sendero ('S') y Resto de casillas valen 1
      break;
    }

    default: //por si le pasamos otra accion diferente
      coste = 0;
      break;
  }

  return coste;
}
double ComportamientoTecnico::heuristica(const EstadoT &actual, const EstadoT &destino) {
  // Distancia de Manhattan
  return max(abs(actual.site.f - destino.site.f), abs(actual.site.c - destino.site.c)); 
}
list<Action> ComportamientoTecnico::calcularRutaTecnico(const EstadoT &inicio, const EstadoT &final, const vector<vector<unsigned char>> &terreno, const vector<vector<unsigned char>> &altura) {
  
  NodoT current_node;
  list <Action> path;
  priority_queue <NodoT> abiertos; //Nodos que vamos explorando (ordenados de menor a mayor coste)
  map <EstadoT, double> cerrados; // El Estado con el menor coste
  bool SolutionFound = false;

  //0. Se introduce el nodo actual en la lista pendiente de explorar de menor a mayor coste
  current_node.estado = inicio;
  current_node.g=0.0;
  current_node.h = heuristica(inicio, final); 
  current_node.f = current_node.g + current_node.h;
  abiertos.push(current_node);


  while (!SolutionFound && !abiertos.empty()){
    current_node = abiertos.top(); //El de menor energia 'f'
    abiertos.pop();
    
    if (terreno[current_node.estado.site.f][current_node.estado.site.c] == 'D') {
        current_node.estado.zapatillas = true;
    }

    //1. Comprobar si es solucion
    if (current_node.estado.site.f == final.site.f && current_node.estado.site.c== final.site.c){
      SolutionFound=true;

    }
    else {
      // 2. Control de Nodos Cerrados (Si no hemos estado nunca en este estado, o si hemos estado pero ahora logramos menos consumo)
      if (cerrados.find(current_node.estado) == cerrados.end() || cerrados[current_node.estado] > current_node.g) {
        //Nuevo camino con menor consumo
        cerrados[current_node.estado] = current_node.g;

        // 3.1. Generamos el hijo resultante de aplicar la acción WALK
        const int NUM_ACCIONES=3;
        Action posibles_acciones[NUM_ACCIONES] = {WALK, TURN_SR, TURN_SL};
        for (int i = 0; i < NUM_ACCIONES; i++) {
          Action accion = posibles_acciones[i];
          
          //Comprobamos que el movimiento no es ilegal (si el estado no cambia, es ilegal)
          EstadoT estado_nuevo = applyT(accion, current_node.estado, terreno, altura);
          if (!(estado_nuevo == current_node.estado)){
            NodoT child = current_node;
            child.estado = estado_nuevo;
            child.secuencia.push_back(accion);
            //recoge las zapatillas antes de calcular el coste
            if (terreno[current_node.estado.site.f][current_node.estado.site.c] == 'D') 
              current_node.estado.zapatillas = true;
            double coste_child = CalcularCosteEnergia(current_node.estado, child.estado, accion, terreno, altura);
            child.g = current_node.g + coste_child;
            child.h = heuristica(child.estado, final);
            child.f = child.g + child.h;
            abiertos.push(child);
          }
          
        }

      }
    } 
    
  }

  if (SolutionFound){
    path = current_node.secuencia;
  }
  return path;

}


//NIVEL 5:

list<Action> ComportamientoTecnico::RodearIngeniero(const Sensores &sensores, int destinoF, int destinoC) {
    // 1. Obtenemos nuestra ubicación actual
    ubicacion miUbicacion = {sensores.posF, sensores.posC, sensores.rumbo};
    
    // 2. Calculamos dónde está el ingeniero usando tu función
    ubicacion ingenieroPos = Delante(miUbicacion);

    // 3. Copiamos el mapa para no alterar el original
    vector<vector<unsigned char>> mapaModificado = mapaResultado;

    // 4. Plantamos el muro falso donde está el ingeniero
    mapaModificado[ingenieroPos.f][ingenieroPos.c] = 'M';

    // 5. Preparamos los estados para la búsqueda
    EstadoT estInicial {miUbicacion, this->tiene_zapatillas};
    EstadoT estFinal   {ubicacion{destinoF, destinoC, sur}, this->tiene_zapatillas}; 
          
    // 6. Calculamos la ruta con el mapa modificado
    return calcularRutaTecnico(estInicial, estFinal, mapaModificado, mapaCotas);
}

// Niveles del técnico

Action ComportamientoTecnico::ComportamientoTecnicoNivel_0(Sensores sensores) {
  
  Action accion = IDLE;

  ActualizarMapa(sensores);
  ActualizarMapaPasos(sensores);

  //TÉCNICO NO CONSIDERA LAS ZAPATILLAS, permanece a false

  if (sensores.superficie[0]=='U') return IDLE;

  //3. Mi vision. Pasamos la visión
  char i, c, d;
  EvaluarAlturas(sensores, i, c, d);

  AplicarObstaculosAgentes(sensores, i, c, d);

  //4. Decision. Pregunto cual es mi mejor opcion
  int pos = VeoCasillaInteresante(sensores, i,c, d, tiene_zapatillas);
  

  //5.Accion 
  switch (pos) {
    case 2: accion = WALK; break;
    case 1: accion = TURN_SL; break;  
    case 3: accion = TURN_SR; break;  
    default: accion = TURN_SL; break;
  }
  
  // 6. Romper posibles bucles
  accion = RompeBucles(accion, c, permitidas_nivel0);

  last_action = accion;
  return accion;
}


Action ComportamientoTecnico::ComportamientoTecnicoNivel_1(Sensores sensores) {

  Action accion = IDLE;

  //0.Memoria del mapa que tengo delante
  ActualizarMapa(sensores); 
  ActualizarMapaPasos(sensores);

  //1. Memoria. Si la casilla que piso ahora mismo(0) tiene zapatillas, lo memorizo
  if (sensores.superficie[0] == 'D') tiene_zapatillas = true;

  //2. Vision. Pasamos la visión y si tenemos o no zapatillas
  char i, c, d;
  EvaluarAlturas(sensores, i, c, d);

  //Si hay un agente cerca de mi, me tomo la casilla como un muro
  AplicarObstaculosAgentes(sensores, i, c, d);

  //4. Decision
  int pos = VeoCasillaExplorable(sensores,i,c, d, tiene_zapatillas);

 //5.Accion 
  switch (pos) {
    case 2: accion = WALK; break;
    case 1: accion = TURN_SL; break; 
    case 3: accion = TURN_SR; break; 
    default: accion = TURN_SL; break;
  }
  
  // 6. Rompe posible bucle

  if (tiene_zapatillas) permitidas_nivel1 ="BCDUXSHA";
  accion = RompeBucles(accion, c, permitidas_nivel1);

  last_action = accion;
  return accion;
}
Action ComportamientoTecnico::ComportamientoTecnicoNivel_2(Sensores sensores) { 
  Action accion = IDLE;

  // 1. Buscar al Ingeniero en el cono de visión (1 a 15)
  bool ingeniero_a_la_vista = false;
  for (int k = 1; k < 16; k++) {
    if (sensores.agentes[k] == 'i' || sensores.agentes[k] == 'I') {
      ingeniero_a_la_vista = true;
      break;
    }
  }

  char i, c, d;
  EvaluarAlturas(sensores, i, c, d);
  AplicarObstaculosAgentes(sensores, i, c, d);
  
  // El Técnico puede pisar Bosque ('B') solo si tiene zapatillas
  bool i_ok = (i != 'M' && i != 'P' && (i != 'B' || tiene_zapatillas));
  bool c_ok = (c != 'M' && c != 'P' && (c != 'B' || tiene_zapatillas));
  bool d_ok = (d != 'M' && d != 'P' && (d != 'B' || tiene_zapatillas));


  if (historial_acciones.empty()) {
      historial_acciones.push_back(WALK); // Lo marcamos para que no vuelva a entrar
      if (c_ok) {
          return WALK;
      }
  }

  // 3. Evasion
  static int panico = 0; 
  
  if (ingeniero_a_la_vista || sensores.choque) {
    panico = 4; 
    planMovimiento.clear();
    hayPlan = false;
  }

  if (panico > 0) {
    panico--; 

    // Prioridad 1: Huir de frente para no estorbar
    if (c_ok && sensores.agentes[2] != 'i' && sensores.agentes[2] != 'I') {
      return WALK; 
    } 
    else if (i_ok) { 
      return TURN_SL;
    } 
    else if (d_ok) { 
      return TURN_SR;
    } 
    else {
      return TURN_SL;
    }
  } 

  // 4. Planificamos un escondite
  if (!hayPlan) {
    int max_dist = -1;
    int esconditeF = sensores.posF;
    int esconditeC = sensores.posC;
    
    int filas = mapaResultado.size();
    int cols = mapaResultado[0].size();

    // Calculamos la casilla transitable más alejada de la Belkanita
    for (int f = 0; f < filas; f++) {
      for (int col = 0; col < cols; col++) {
        char terr = mapaResultado[f][col];
        
        if (terr != 'M' && terr != 'P' && terr != '?') {
          if (terr == 'B' && !tiene_zapatillas) continue;
          
          int dist = abs(f - sensores.BelPosF) + abs(col - sensores.BelPosC);
          if (dist > max_dist) {
            max_dist = dist;
            esconditeF = f;
            esconditeC = col;
          }
        }
      }
    }

    if (sensores.posF == esconditeF && sensores.posC == esconditeC) {
        hayPlan = true;
    } else {
        EstadoT inicio {ubicacion{sensores.posF, sensores.posC, sensores.rumbo}, this->tiene_zapatillas};
        EstadoT fin    {ubicacion{esconditeF, esconditeC, norte}, this->tiene_zapatillas};
        
        planMovimiento = calcularRutaTecnico(inicio, fin, mapaResultado, mapaCotas);
        hayPlan = true; 
        
        if (planMovimiento.empty()) {
            return TURN_SL; // Girar si no hay ruta posible
        }
    }
  }

  // 5. Ejecutamos el camino al escondite
  if (hayPlan && !planMovimiento.empty()) {
    accion = planMovimiento.front();
    planMovimiento.pop_front();
  }

  // 6. Modo centinela
  if (hayPlan && planMovimiento.empty()) {
     if (sensores.rumbo != norte) {
         int diff = (0 - sensores.rumbo + 8) % 8;
         accion = (diff <= 4) ? TURN_SR : TURN_SL;
     } else {
         accion = IDLE;
     }
  }

  return accion;
}
Action ComportamientoTecnico::ComportamientoTecnicoNivel_3(Sensores sensores) {
  Action accion = IDLE;
  if (sensores.superficie[0] == 'D') tiene_zapatillas=true;

  if (!hayPlan){
    EstadoT inicio;
    inicio.site.f = sensores.posF;
    inicio.site.c = sensores.posC;
    inicio.site.brujula = sensores.rumbo;
    inicio.zapatillas = tiene_zapatillas;

    EstadoT fin;
    fin.site.f = sensores.BelPosF;
    fin.site.c = sensores.BelPosC;
    
    // Invocamos el A* para minimizar energía
    planMovimiento = calcularRutaTecnico(inicio, fin, mapaResultado, mapaCotas);

    if (!planMovimiento.empty()){
      VisualizaPlan (inicio.site, planMovimiento); 
      hayPlan = true;
    }
  }
  // 2. Si tenemos un plan, lo ejecutamos
  if (hayPlan && !planMovimiento.empty()){
    accion = planMovimiento.front(); 
    planMovimiento.pop_front();
  }
  // 3. Si hemos vaciado el plan, lo desmarcamos para poder calcular otro si hiciera falta
  if (planMovimiento.empty()) {
    hayPlan = false;
  }

  
  return accion;
}
Action ComportamientoTecnico::ComportamientoTecnicoNivel_4(Sensores sensores) {
  return IDLE;
}


Action ComportamientoTecnico::ComportamientoTecnicoNivel_5(Sensores sensores)
{
  Action accion = IDLE;
  ActualizarMapa(sensores);
  
  
  if (sensores.venpaca) {
    if (estadoObra == ESPERANDO) {
      // El ingeniero nos da la casilla a la que ir
      destinoF = sensores.GotoF;
      destinoC = sensores.GotoC;
      planMovimiento.clear();
      modo_rodeo = false; 
      ingeniero_listo = false;
      
      // Derivamos al estado correspondiente
      if (!primera_casilla_alcanzada) {
        estadoObra = CAMINO_PRIMERA_CASILLA;
      } else {
        estadoObra = CAMINO_PASO_PROPIO;
      }
    } 
    else {
      // El ingeniero se ha alineado y nos avisa de que está listo
      ingeniero_listo = true; // Guardamos el aviso en memoria
      ingF = sensores.GotoF;  // Guardamos sus coordenadas
      ingC = sensores.GotoC;
    }
  }

  switch (estadoObra) {

    case ESPERANDO:
      // Ya no comprobamos venpaca aquí porque lo hace el radar global
      accion = IDLE;
      break;

    case CAMINO_PRIMERA_CASILLA: {
      if (sensores.posF == destinoF && sensores.posC == destinoC) {
        primera_casilla_alcanzada = true;
        planMovimiento.clear();
        modo_rodeo = false;
        
        if (sensores.enfrente) {
          accion = INSTALL;
          estadoObra = ESPERANDO;
        } else {
          estadoObra = ALINEANDO_INSTALANDO;
        }
        break;
      }

      if (planMovimiento.empty()) {
          EstadoT estInicial {ubicacion{sensores.posF, sensores.posC, sensores.rumbo}, this->tiene_zapatillas};
          EstadoT estFinal   {ubicacion{destinoF, destinoC, sur}, this->tiene_zapatillas};
          planMovimiento = calcularRutaTecnico(estInicial, estFinal, mapaResultado, mapaCotas);
      }

      //EJECUCIÓN REACTIVA
      if (!planMovimiento.empty()) {
        if (planMovimiento.front() == WALK) {
            
            // Evaluamos la casilla real que tenemos frente a nosotros ahora mismo
            char frente = ViablePorAltura(sensores.superficie[2], sensores.cota[2] - sensores.cota[0], tiene_zapatillas);
            
            // 1. Choques con el Ingeniero u otros agentes
            if (sensores.choque || sensores.agentes[2] != '_') {
                planMovimiento = RodearIngeniero(sensores, destinoF, destinoC);
                if (!planMovimiento.empty()) {
                    accion = planMovimiento.front();
                    planMovimiento.pop_front();
                }
            } 
            else if (frente == 'P' || frente == 'M') {
                planMovimiento.clear(); // Limpiamos para que recalcule en el próximo tick
                accion = IDLE;
            } 
            // 3. Vía libre real
            else {
                accion = planMovimiento.front();
                planMovimiento.pop_front();
            }
        } else {
            // Los giros son seguros
            accion = planMovimiento.front();
            planMovimiento.pop_front();
        }
      }
      break;
    }

    case CAMINO_PASO_PROPIO: {
      if (sensores.posF == destinoF && sensores.posC == destinoC) {
        planMovimiento.clear();
        
        if (sensores.enfrente) {
          accion = INSTALL;
          estadoObra = ESPERANDO;
        } else {
          estadoObra = ALINEANDO_INSTALANDO;
        }
        break;
      }

      if (planMovimiento.empty()) {
          EstadoT estInicial {ubicacion{sensores.posF, sensores.posC, sensores.rumbo}, this->tiene_zapatillas};
          EstadoT estFinal   {ubicacion{destinoF, destinoC, sur}, this->tiene_zapatillas};
          planMovimiento = calcularRutaTecnico(estInicial, estFinal, mapaResultado, mapaCotas);
      }

      // --- SISTEMA DE EJECUCIÓN REACTIVA ---
      if (!planMovimiento.empty()) {
        if (planMovimiento.front() == WALK) {
            
            char frente = ViablePorAltura(sensores.superficie[2], sensores.cota[2] - sensores.cota[0], tiene_zapatillas);
            
            if (sensores.choque || sensores.agentes[2] != '_') {
                accion = IDLE; // Nos esperamos a que el ingeniero termine de cavar/moverse
            } 
            else if (frente == 'P' || frente == 'M') {
                planMovimiento.clear();
                accion = IDLE;
            } 
            else {
                accion = planMovimiento.front(); 
                planMovimiento.pop_front();
            }
        } else {
            accion = planMovimiento.front();
            planMovimiento.pop_front();
        }
      }
      break;
    }

    case ALINEANDO_INSTALANDO: {
      // 1. Si ya estamos perfectamente alineados, ¡Instalamos!
      if (sensores.enfrente) {
        accion = INSTALL;
        estadoObra = ESPERANDO; // Volvemos a esperar el próximo tramo
        break;
      }

      // 2. Giro Inteligente (Usamos la memoria del booleano del Radar Global)
      if (ingeniero_listo) {
        int rumboHaciaIngeniero = -1;
        // Calculamos el punto cardinal exacto con restas
        if      (ingF < sensores.posF && ingC == sensores.posC) rumboHaciaIngeniero = 0; // Norte
        else if (ingF == sensores.posF && ingC > sensores.posC) rumboHaciaIngeniero = 2; // Este
        else if (ingF > sensores.posF && ingC == sensores.posC) rumboHaciaIngeniero = 4; // Sur
        else if (ingF == sensores.posF && ingC < sensores.posC) rumboHaciaIngeniero = 6; // Oeste
        
        if (rumboHaciaIngeniero != -1) {
          // Calculamos la diferencia para saber si es mejor girar a izquierda o derecha
          int diff = (rumboHaciaIngeniero - sensores.rumbo + 8) % 8;
          if (diff == 0) accion = IDLE; // Por seguridad
          else accion = (diff <= 4) ? TURN_SR : TURN_SL;
        } else {
          accion = TURN_SR; // Fallback por si hay algún error
        }
      } 
      // 3. Si el Ingeniero aún no ha llegado o no ha avisado, nos ahorramos la batería
      else {
        accion = IDLE;
      }
      break;
    }
  }

  return accion;
}


Action ComportamientoTecnico::ComportamientoTecnicoNivel_6(Sensores sensores) {
    Action accion = IDLE;
    ActualizarMapa(sensores);

    switch (estadoNivel6) {
      
      case FASE_EXPLORACION:
        if (sensores.venpaca) {
          estadoNivel6 = FASE_EJECUCION;
          accion = ComportamientoTecnicoNivel_5(sensores);
        } else if (sensores.energia < 2500 || sensores.vida < 1500) {
          // Nos quedamos quietos para conservar recursos hasta que el ingeniero esté listo
          accion = IDLE; 
        }else {
          accion = ComportamientoTecnicoNivel_1(sensores);
        }
        break;

      case FASE_EJECUCION:
        accion = ComportamientoTecnicoNivel_5(sensores);

        // Si hemos acabado nuestra parte y no nos llaman...
        if (estadoObra == ESPERANDO && !sensores.venpaca) {
          
          // Chequeamos las iteraciones restantes.
          // Solo nos rendimos si la inactividad coincide con un múltiplo de 100.
          if (sensores.vida % 100 == 0) {
            estadoNivel6 = FASE_EXPLORACION;
          }
        }
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
void ComportamientoTecnico::ActualizarMapa(Sensores sensores) {
  mapaResultado[sensores.posF][sensores.posC] = sensores.superficie[0];
  mapaCotas[sensores.posF][sensores.posC] = sensores.cota[0];

  int pos = 1;
  switch (sensores.rumbo) {
    case norte:
      for (int j = 1; j < 4; j++)
        for (int i = -j; i <= j; i++) {
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
        for (int i = -j; i <= j; i++) {
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
        for (int i = -j; i <= j; i++) {
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
        for (int i = -j; i <= j; i++) {
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
 * @brief Determina si una casilla es transitable para el técnico.
 * En esta práctica, si el técnico tiene zapatillas, el bosque ('B') es transitable.
 * @param f Fila de la casilla.
 * @param c Columna de la casilla.
 * @param tieneZapatillas Indica si el agente posee las zapatillas.
 * @return true si la casilla es transitable.
 */
bool ComportamientoTecnico::EsCasillaTransitableLevel0(int f, int c, bool tieneZapatillas) {
  if (f < 0 || f >= mapaResultado.size() || c < 0 || c >= mapaResultado[0].size()) return false;
  return EsCamino(mapaResultado[f][c]);  // Solo 'C', 'S', 'D', 'U' son transitables en Nivel 0
}

/**
 * @brief Comprueba si la casilla de delante es accesible por diferencia de altura.
 * Para el técnico: desnivel máximo siempre 1.
 * @param actual Estado actual del agente (fila, columna, orientacion).
 * @return true si el desnivel con la casilla de delante es admisible.
 */
bool ComportamientoTecnico::EsAccesiblePorAltura(const ubicacion &actual) {
  ubicacion del = Delante(actual);
  if (del.f < 0 || del.f >= mapaCotas.size() || del.c < 0 || del.c >= mapaCotas[0].size()) return false;
  int desnivel = abs(mapaCotas[del.f][del.c] - mapaCotas[actual.f][actual.c]);
  if (desnivel > 1) return false;
  return true;
}

/**
 * @brief Devuelve la posición (fila, columna) de la casilla que hay delante del agente.
 * Calcula la casilla frontal según la orientación actual (8 direcciones).
 * @param actual Estado actual del agente (fila, columna, orientacion).
 * @return Estado con la fila y columna de la casilla de enfrente.
 */
ubicacion ComportamientoTecnico::Delante(const ubicacion &actual) const {
  ubicacion delante = actual;
  switch (actual.brujula) {
    case 0: delante.f--; break;                        // norte
    case 1: delante.f--; delante.c++; break;     // noreste
    case 2: delante.c++; break;                     // este
    case 3: delante.f++; delante.c++; break;     // sureste
    case 4: delante.f++; break;                        // sur
    case 5: delante.f++; delante.c--; break;     // suroeste
    case 6: delante.c--; break;                     // oeste
    case 7: delante.f--; delante.c--; break;     // noroeste
  }
  return delante;
}

/**
 * @brief Imprime por consola la secuencia de acciones de un plan.
 *
 * @param plan  Lista de acciones del plan.
 */
void ComportamientoTecnico::PintaPlan(const list<Action> &plan)
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
 * @brief Convierte un plan de acciones en una lista de casillas para
 *        su visualización en el mapa 2D.
 *
 * @param st    Estado de partida.
 * @param plan  Lista de acciones del plan.
 */
void ComportamientoTecnico::VisualizaPlan(const ubicacion &st,
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



#ifndef COMPORTAMIENTOINGENIERO_H
#define COMPORTAMIENTOINGENIERO_H

#include <chrono>
#include <list>
#include <map>
#include <set>
#include <thread>
#include <time.h>
#include <string>

#include "comportamientos/comportamiento.hpp"

// =========================================================================
// ESTRUCTURAS PARA BÚSQUEDA (Niveles 2, 3, 4)
// =========================================================================

//NIVEL 2
struct EstadoI {
  ubicacion site;
  bool zapatillas;

  bool operator==(const EstadoI &st) const { //si esta en la misma posicion y tiene o no zap
    return site == st.site && zapatillas == st.zapatillas;
  }
};

struct NodoI {
  EstadoI estado;
  list<Action> secuencia;

  bool operator==(const NodoI &node) const {
    return estado == node.estado;
  }

  // Operador < necesario para usar 'set' (Búsqueda en Anchura V2)
  bool operator<(const NodoI &node) const { //nos sirve para elegir el mejor escenario
    if (estado.site.f < node.estado.site.f) return true; //compara la fila
    //si la fila es la misma, compara la columna
    else if (estado.site.f == node.estado.site.f && estado.site.c < node.estado.site.c) return true;
    //si f y c son iguales compara la brujula
    else if (estado.site.f == node.estado.site.f && estado.site.c == node.estado.site.c && estado.site.brujula < node.estado.site.brujula) return true;
    //si f,c,brujula son iguales compara i tiene zapatillas
    else if (estado.site.f == node.estado.site.f && estado.site.c == node.estado.site.c && estado.site.brujula == node.estado.site.brujula && !estado.zapatillas && node.estado.zapatillas) return true;
    else return false;
  }
};

//NIVEL 4. RED DE TUBERÍAS


struct NodoTuberia{
  int f, c;         //Coordenadas para situar la tuberia
  int altura;       //Altura de la casilla
  int ecoUsado;
  int energiaUsada;
  list <Paso> secuencia;    //Trazo de tuberias
  double g;         //Coste a minimizar (acciones INSTALL)
  double h;         //Distancia a la U mas cercana
  double coste; //g+h
  bool operator<(const NodoTuberia &otro) const {
    if (coste == otro.coste) {
        return ecoUsado > otro.ecoUsado; // Prioriza el de menor impacto ecológico
    }
    return coste > otro.coste; 
  }
};

class ComportamientoIngeniero : public Comportamiento {
public:
  // =========================================================================
  // CONSTRUCTORES
  // =========================================================================
  
  /**
   * @brief Constructor para niveles 0, 1 y 6 (sin mapa completo)
   * @param size Tamaño del mapa (si es 0, se inicializa más tarde)
   */
  ComportamientoIngeniero(unsigned int size = 0) : Comportamiento(size) {
    
    //NIVEL 0:
    this->last_action = IDLE;
    this->tiene_zapatillas = false;
    this->permitidas_nivel0 = "?UDCX";

    //NIVEL 1:
    this->permitidas_nivel1="?CDUXSHA";

    this-> ultimos_pasos.clear(); // Guarda las últimas 20 posiciones
    this-> en_bucle = false;

    this->ultima_pos = {-1, -1};
    this-> meta_ocupada= {-1, -1};
    this->meta_encontrada= {-1, -1};
    this->meta_conocida=false;
    this->mapa_pasos_inicializado=false;
    this->historial_acciones = {};
    this->estadoNivel6=FASE_EXPLORACION;   
    this->plantasDescubiertas.clear();

    // 1. Inicializar la matriz con ceros la primera vez
    if (!mapa_pasos_inicializado && mapaResultado.size() > 0 && mapaResultado[0].size() > 0) {
      mapaPasos.assign(mapaResultado.size(), vector<int>(mapaResultado[0].size(), 0));
      mapa_pasos_inicializado = true;
    }
  }

  /**
   * @brief Constructor para niveles 2, 3, 4 y 5 (con mapa completo conocido)
   * @param mapaR Mapa de terreno conocido
   * @param mapaC Mapa de cotas conocido
   */
  ComportamientoIngeniero(std::vector<std::vector<unsigned char>> mapaR, 
                         std::vector<std::vector<unsigned char>> mapaC): 
                         Comportamiento(mapaR, mapaC) {
    this->hayPlan=false;
    this->tiene_zapatillas=false;    
    this->planMovimiento.clear();
    planEscape.clear();
    this->planTuberias.clear();
    this ->estadoObra=PLANIFICANDO;  
    this->intentandoInstalar=false;   
         
  }

  ComportamientoIngeniero(const ComportamientoIngeniero &comport)
      : Comportamiento(comport) {}
  ~ComportamientoIngeniero() {}

  /**
   * @brief Bucle principal de decisión del agente.
   * Estudia los sensores y decide la siguiente acción.
   * 
   * EJEMPLO DE USO:
   * Action accion = think(sensores);
   * return accion; // El motor ejecutará esta acción
   */
  Action think(Sensores sensores);

  ComportamientoIngeniero *clone() {
    return new ComportamientoIngeniero(*this);
  }

  // =========================================================================
  // ÁREA DE IMPLEMENTACIÓN DEL ESTUDIANTE
  // =========================================================================

  // Funciones específicas para cada nivel (para ser implementadas por el alumno)
  
  /**
   * @brief Implementación del Nivel 0.
   * @param sensores Datos actuales de los sensores del agente.
   * @return Acción a realizar.
   */
  Action ComportamientoIngenieroNivel_0(Sensores sensores);
  
  /**
   * @brief Implementación del Nivel 1.
   * @param sensores Datos actuales de los sensores del agente.
   * @return Acción a realizar.
   */
  Action ComportamientoIngenieroNivel_1(Sensores sensores);
  
  /**
   * @brief Implementación del Nivel 2.
   * @param sensores Datos actuales de los sensores del agente.
   * @return Acción a realizar.
   */ 
  Action ComportamientoIngenieroNivel_2(Sensores sensores);
  
  /**
   * @brief Implementación del Nivel 3.
   * @param sensores Datos actuales de los sensores del agente.
   * @return Acción a realizar.
   */
  Action ComportamientoIngenieroNivel_3(Sensores sensores);
  
  /**
   * @brief Implementación del Nivel 4.
   * @param sensores Datos actuales de los sensores del agente.
   * @return Acción a realizar.
   */
  Action ComportamientoIngenieroNivel_4(Sensores sensores);
  
  /**
   * @brief Implementación del Nivel 5.
   * @param sensores Datos actuales de los sensores del agente.
   * @return Acción a realizar.
   */
  Action ComportamientoIngenieroNivel_5(Sensores sensores);
  
  /**
   * @brief Implementación del Nivel 6.
   * @param sensores Datos actuales de los sensores del agente.
   * @return Acción a realizar.
   */
  Action ComportamientoIngenieroNivel_6(Sensores sensores);

protected:
  // =========================================================================
  // FUNCIONES PROPORCIONADAS
  // =========================================================================

  /**
   * @brief Actualiza la información del mapa interno basándose en los sensores.
   * IMPORTANTE: Esta función ya está implementada. Actualiza mapaResultado y mapaCotas
   * con la información de los 16 sensores (casilla actual + 15 casillas alrededor).
   */
  void ActualizarMapa(Sensores sensores);

  /**
   * @brief Comprueba si una casilla es transitable.
   * @param f Fila de la casilla.
   * @param c Columna de la casilla.
   * @param tieneZapatillas Indica si el agente posee zapatillas.
   * @return true si la casilla es transitable (no es muro ni precipicio).
   */
  bool EsCasillaTransitableLevel0(int f, int c, bool tieneZapatillas);

  /**
   * @brief Comprueba si la casilla de delante es accesible por diferencia de altura.
   * REGLAS: Desnivel máximo 1 sin zapatillas, 2 con zapatillas.
   * @param actual Estado actual del agente (fila, columna, orientacion).
   * @return true si el desnivel con la casilla de delante es admisible.
   */
  bool EsAccesiblePorAltura(const ubicacion &actual, bool zap);

  /**
   * @brief Devuelve la posición (fila, columna) de la casilla que hay delante del agente.
   * @param actual Estado actual del agente (fila, columna, orientacion).
   * @return Estado con la fila y columna de la casilla de enfrente.
   */
  ubicacion Delante(const ubicacion &actual) const;

  

  /**
 * @brief Imprime por consola la secuencia de acciones de un plan para un agente.
 * @param plan  Lista de acciones del plan.
 */
  void PintaPlan(const list<Action> &plan);


/**
 * @brief Imprime las coordenadas y operaciones de un plan de tubería.
 * @param plan  Lista de pasos (fila, columna, operación).
 */
  void PintaPlan(const list<Paso> &plan);


  /**
 * @brief Convierte un plan de acciones en una lista de casillas para
 *        su visualización en el mapa gráfico.
 * @param st    Estado de partida.
 * @param plan  Lista de acciones del plan.
 */
  void VisualizaPlan(const ubicacion &st, const list<Action> &plan);

  /**
 * @brief Convierte un plan de tubería en la lista de casillas usada
 *        por el sistema de visualización.
 * @param st    Estado de partida (no utilizado directamente).
 * @param plan  Lista de pasos del plan de tubería.
 */
  void VisualizaRedTuberias(const list<Paso> &plan);

  // =========================================================================
  //  NIVEL 2: MOTOR DE BÚSQUEDA BELKANITA
  // =========================================================================
  
  /**
   * @brief Devuelve el estado tras avanzar 1 (WALK) o 2 (JUMP) casillas.
   */
  EstadoI NextCasillaIngeniero (const EstadoI &st, Action accion);

  /**
   * @brief Comprueba si el Ingeniero puede hacer WALK o JUMP hacia una casilla.
   */
  bool CasillaAccesibleIngeniero(const EstadoI &st, const vector<vector<unsigned char>> &terreno, const vector<vector<unsigned char>> &altura, Action accion);

  /**
   * @brief Aplica una acción a un estado y devuelve el nuevo estado resultante.
   */
  EstadoI applyI(Action accion, const EstadoI &st, const vector<vector<unsigned char>> &terreno, const vector<vector<unsigned char>> &altura);

  /**
   * @brief Algoritmo de Búsqueda en Anchura.
   */
  list<Action> calcularRutaIngeniero(const EstadoI &inicio, const EstadoI &final, const vector<vector<unsigned char>> &terreno, const vector<vector<unsigned char>> &altura);

  // =========================================================================
  // NIVEL 4: INGENIERO PLANIFICA RED TUBERÍAS
  // =========================================================================
  double HeuristicaTuberia(int f, int c, const vector<pair<int,int>>& plantasU);
  int CosteEnergeticoInstall(unsigned char terreno);
  int CosteEcologico(unsigned char terreno, int op);
  list<Paso> PlanificarRedTuberias(int belF, int belC,const vector<vector<unsigned char>>& terreno, const vector<vector<unsigned char>>& altura, int energiaDisponible, int ecoMaximo);
  int CosteEcoBaseInstall(unsigned char terreno);

  // =========================================================================
  // NIVEL 6: INGENIERO Y TÉCNICO CONSTRUYEN RED DE TUBERÍAS
  // =========================================================================
  void ActualizarPlantasDescubiertas();
  void ActualizarBelkanita(const Sensores& sensores);

  // =========================================================================
  // NIVELES 0 Y 1 
  // =========================================================================
  char ViablePorAltura(char casilla, int dif, bool zap);
  void EvaluarAlturas(const Sensores &sensores, char &i, char &c, char &d);
  void AplicarObstaculosAgentes(const Sensores &sensores, char &i, char &c, char &d);
  
  ubicacion GirarIzquierda(ubicacion p) const; 
  ubicacion GirarDerecha(ubicacion p) const;
  bool EsCamino(unsigned char c) const;
  
  void ActualizarYDetectarBucle(const Sensores &sensores);
  void ActualizarMapaPasos(const Sensores &sensores);
  int VeoZapatillasVisual(const Sensores &sensores, bool izq, bool cen, bool der);

  //N0: INGENIERO BUSCA Y LLEGA A 'U'
  void DetectarMeta(const Sensores &sensores);
  int AvanzaMetaConocida(const Sensores &sensores, char i, char c, char d);
  int VeoCasillaInteresante(const Sensores &sensores, char i, char c, char d, bool zap);
  int ElegirMenosVisitada_N0(const Sensores &sensores, char i, char c, char d, string permitidas);
  Action RompeBucles_N0(Action accion_propuesta, char c, const string &transitables);
  
  //NIVEL 1: INGENIERO EXPLORA EL MAPA
  Action RompeBucles_N1(const Sensores &sensores, Action accion_propuesta, char c, const string &transitables);
  int ElegirMenosVisitada_N1(const Sensores &sensores, char i, char c, char d, string permitidass);
  int VeoCasillaExplorable(const Sensores &sensores, char i, char c, char d, bool zap);



private:
  // =========================================================================
  // VARIABLES DE ESTADO (PUEDEN SER EXTENDIDAS POR EL ALUMNO)
  // =========================================================================
  
  //NIVLE 0
  Action last_action;            // Almacena la última acción realizada para detectar bucles.
  bool tiene_zapatillas;         // Indica si el agente posee las zapatillas actualmente.
  string permitidas_nivel0;
  pair<int, int> meta_ocupada;
  

  //NIVEL 1
  string permitidas_nivel1;
  
  // NIVEL 0 y 1 - Memoria anti-bucles espaciales
  list<pair<int, int>> ultimos_pasos; // Guarda las últimas 20 posiciones
  bool en_bucle;


  vector<vector<int>> mapaPasos; // Contador de visitas por cada casilla del mapa.
  bool mapa_pasos_inicializado;  // Control para inicializar la matriz de pasos una sola vez.
  pair<int, int> ultima_pos;     // Coordenadas (f, c) de la última casilla donde se registró paso.
  pair<int, int> meta_encontrada;// Coordenadas de la ubicación de la meta detectada.
  bool meta_conocida;            // Indica si el agente tiene un objetivo de meta activo.
  vector<Action> historial_acciones;   // Memoria de acciones recientes para el sistema anti-bucles.

  //NILVE 2
  bool hayPlan;                  // Indica si hay un plan establecido que ejecutar
  list <Action> planMovimiento;             // Almacena la secuencia de acciones a realizar
  //NIVLE 3
  list <Action> planEscape;

  //NIVEL 4
  list <Paso> planTuberias;       //Almacena la secuencia de tuberías a instalar
  
  //NIVEL 5
  enum EstadoObraI {PLANIFICANDO, CAMINO_PRIMERA_CASILLA, ADECUANDO_PRIMERA_CASILLA, CAMINO_PASO_PROPIO, LLAMANDO_TECNICO, ADECUANDO_PASO_PROPIO, ALINEANDO_CON_TECNICO, INSTALANDO, FINALIZADO};
  EstadoObraI estadoObra;
  Paso pasoActual;
  bool intentandoInstalar;

  //NIVEL 6
  enum EstadoNivel6{FASE_EXPLORACION, FASE_PLANIFICACION, FASE_EJECUCION, FASE_FINALIZACION};
  EstadoNivel6 estadoNivel6;
  list<pair<int, int>> plantasDescubiertas;


};

#endif

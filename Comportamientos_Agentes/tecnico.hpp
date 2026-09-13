#ifndef COMPORTAMIENTOTECNICO_H
#define COMPORTAMIENTOTECNICO_H

#include <chrono>
#include <time.h>
#include <thread>
#include <list>
#include <vector>
#include <string>


#include "comportamientos/comportamiento.hpp"
struct EstadoT {
  ubicacion site;
  bool zapatillas;

  bool operator==(const EstadoT &st) const { //si esta en la misma posicion y tiene o no zap
    return site == st.site && zapatillas == st.zapatillas;
  }
  //Operador < para la lista de Cerrados (para no repetir estados)
  bool operator<(const EstadoT &st) const {
    if (site.f != st.site.f) return site.f < st.site.f;
    if (site.c != st.site.c) return site.c < st.site.c;
    if (site.brujula != st.site.brujula) return site.brujula < st.site.brujula;
    return zapatillas < st.zapatillas;
  }
};

struct NodoT {
  EstadoT estado;
  list<Action> secuencia;
  double g; // Coste real acumulado (energía)
  double h; // Estimación heurística al destino
  double f; // f = g + h

  // Operador para la cola de prioridad (Abiertos)
  //Definimos para el priority_queue (mayor prioridad el nodo con menor coste)
  bool operator<(const NodoT &otro) const {
    return f > otro.f; 
  }
};






// =========================================================================
// DOCUMENTACIÓN PARA ESTUDIANTES
// =========================================================================
/*
 * CLASE: ComportamientoTecnico
 * 
 * DESCRIPCIÓN:
 * Esta clase implementa el comportamiento del agente Técnico en el mundo Belkan.
 * El técnico colabora con el ingeniero para resolver el problema de instalación de tuberías
 */


class ComportamientoTecnico : public Comportamiento {
public:
  // =========================================================================
  // CONSTRUCTORES
  // =========================================================================
  
  /**
   * @brief Constructor para niveles 0, 1 y 6 (sin mapa completo)
   * @param size Tamaño del mapa (si es 0, se inicializa más tarde)
   */
  ComportamientoTecnico(unsigned int size = 0) : Comportamiento(size) {
    // Inicializar Variables de Estado
    this->last_action = IDLE;
    this->tiene_zapatillas = false;
    
    this->permitidas_nivel0 = "UDC";
    this->permitidas_nivel1="?CDUXSHA";
    
    this-> mapa_pasos_inicializado=false;
    this->ultima_pos = {-1, -1};
    this->historial_acciones = {};
    this->hayPlan=false;
    //nivel 6
    this->estadoNivel6=FASE_EXPLORACION; 
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
  ComportamientoTecnico(std::vector<std::vector<unsigned char>> mapaR, 
                       std::vector<std::vector<unsigned char>> mapaC): 
                       Comportamiento(mapaR, mapaC) {
    // Inicializar Variables de Estado
    hayPlan = false;
    tiene_zapatillas = false;
    planMovimiento.clear();
    planEscape.clear();
    
    //Nivel 5
    estadoObra=ESPERANDO; 
    instalandoAntes=false;
    destinoF=-1;
    destinoC=-1;
    modo_rodeo=false;
    primera_casilla_alcanzada=false;
    ingeniero_listo=false;
    ingF= ingC=-1;

    //Nivel 6
    estadoNivel6=FASE_EXPLORACION;

  }

  ComportamientoTecnico(const ComportamientoTecnico &comport): Comportamiento(comport) {}
  ~ComportamientoTecnico() {}

  /**
   * @brief Bucle principal de decisión del técnico.
   * Estudia los sensores y decide la siguiente acción.
   * 
   * EJEMPLO DE USO:
   * Action accion = think(sensores);
   * return accion; // El motor ejecutará esta acción
   */
  Action think(Sensores sensores);

  ComportamientoTecnico *clone() {
    return new ComportamientoTecnico(*this);
  }

  // =========================================================================
  // ÁREA DE IMPLEMENTACIÓN DEL ESTUDIANTE
  // =========================================================================
  
/**
 * @brief Comportamiento del técnico para el Nivel 0.
 * @param sensores Datos actuales de los sensores.
 * @return Acción a realizar.
 */
  Action ComportamientoTecnicoNivel_0(Sensores sensores);
  
/**
 * @brief Comportamiento del técnico para el Nivel 1.
 * @param sensores Datos actuales de los sensores.
 * @return Acción a realizar.
 */
  Action ComportamientoTecnicoNivel_1(Sensores sensores);
  
/**
 * @brief Comportamiento del técnico para el Nivel 2.
 * @param sensores Datos actuales de los sensores.
 * @return Acción a realizar.
 */
  Action ComportamientoTecnicoNivel_2(Sensores sensores);
  
/**
 * @brief Comportamiento del técnico para el Nivel 3.
 * @param sensores Datos actuales de los sensores.
 * @return Acción a realizar.
 */
  Action ComportamientoTecnicoNivel_3(Sensores sensores);
  
/**
 * @brief Comportamiento del técnico para el Nivel 4.
 * @param sensores Datos actuales de los sensores.
 * @return Acción a realizar.
 */
  Action ComportamientoTecnicoNivel_4(Sensores sensores);
  
/**
 * @brief Comportamiento del técnico para el Nivel 5.
 * @param sensores Datos actuales de los sensores.
 * @return Acción a realizar.
 */
  Action ComportamientoTecnicoNivel_5(Sensores sensores);
  
/**
 * @brief Comportamiento del técnico para el Nivel 6.
 * @param sensores Datos actuales de los sensores.
 * @return Acción a realizar.
 */
  Action ComportamientoTecnicoNivel_6(Sensores sensores);


protected:
  // =========================================================================
  // FUNCIONES PROPORCIONADAS
  // =========================================================================

  /**
   * @brief Actualiza el mapaResultado y mapaCotas con la información de los sensores.
   * IMPORTANTE: Esta función ya está implementada. Actualiza mapaResultado y mapaCotas
   * con la información de los 16 sensores.
   */
  void ActualizarMapa(Sensores sensores);

  /**
   * @brief Determina si una casilla es transitable para el técnico.
   * NOTA: El técnico puede tener reglas de transitabilidad diferentes al ingeniero.
   * @param f Fila de la casilla.
   * @param c Columna de la casilla.
   * @param tieneZapatillas Indica si el agente posee las zapatillas.
   * @return true si la casilla es transitable.
   */
  bool EsCasillaTransitableLevel0(int f, int c, bool tieneZapatillas);

  /**
   * @brief Comprueba si la casilla de delante es accesible por diferencia de altura.
   * REGLA PARA TÉCNICO: Desnivel máximo siempre 1 (independiente de zapatillas).
   * @param actual Estado actual del agente (fila, columna, orientacion).
   * @return true si el desnivel con la casilla de delante es admisible.
   */
  bool EsAccesiblePorAltura(const ubicacion &actual);

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

  // =========================================================================
  // ÁREA DE IMPLEMENTACIÓN DEL ESTUDIANTE
  // =========================================================================
  
  
  // =========================================================================
  // NIVELES 0 Y 1 
  // =========================================================================
  void ActualizarMapaPasos(const Sensores &sensores);
  char ViablePorAltura (char casilla, int dif, bool zap);
  void EvaluarAlturas(const Sensores &sensores, char &i, char &c, char &d);
  void AplicarObstaculosAgentes(const Sensores &sensores, char &i, char &c, char &d);
  
  ubicacion GirarIzquierda(ubicacion p) const; 
  ubicacion GirarDerecha(ubicacion p) const;
  bool EsCamino(unsigned char c) const;

  Action RompeBucles(Action accion_propuesta, char c, const string &transitables);
  int ElegirMenosVisitada(const Sensores &sensores, char i, char c, char d, string permitidas);

  //NIVEL 0: TÉCNICO BUSCA 'U'
  int VeoCasillaInteresante (const Sensores &sensores, char i, char c, char d, bool zap);
  //NIVEL 1: TÉCNICO EXPLORA MAPA
  int VeoCasillaExplorable (const Sensores &sensores, char i, char c, char d, bool zap) ;

  

  // =========================================================================
  // NIVELES 3: ENCONTRAR BELKANITA CON MINIMO COSTE
  // =========================================================================
  EstadoT NextCasillaTecnico (const EstadoT &st, Action accion);
  bool CasillaAccesibleTecnico(const EstadoT &st, const vector<vector<unsigned char>> &terreno, const vector<vector<unsigned char>> &altura, Action accion);
  double CalcularCosteEnergia(const EstadoT &origen, const EstadoT &destino, Action a, const vector<vector<unsigned char>> &terreno, const vector<vector<unsigned char>> &altura);
  EstadoT applyT(Action a, const EstadoT &st, const vector<vector<unsigned char>> &terreno, const vector<vector<unsigned char>> &altura) ;
  double heuristica(const EstadoT &actual, const EstadoT &destino);
  list<Action> calcularRutaTecnico(const EstadoT &inicio, const EstadoT &final, const vector<vector<unsigned char>> &terreno, const vector<vector<unsigned char>> &altura) ;

  
  // =========================================================================
  // NIVELES 5: TÉNCICO E INGENIERO INSTALAN TUBERÍAS
  // =========================================================================
  list<Action> RodearIngeniero(const Sensores &sensores, int destinoF, int destinoC) ;
  

private:
  // =========================================================================
  // VARIABLES DE ESTADO (PUEDEN SER EXTENDIDAS POR EL ALUMNO)
  // =========================================================================

  Action last_action;            // Almacena la última acción realizada para detectar bucles.
  bool tiene_zapatillas;        // Indica si el agente posee las zapatillas actualmente.
  
  //NIVEL 0
  vector<vector<int>> mapaPasos; // Contador de visitas por cada casilla del mapa.
  bool mapa_pasos_inicializado;  // Control para inicializar la matriz de pasos una sola vez.
  pair<int, int> ultima_pos;     // Coordenadas (f, c) de la última casilla donde se registró paso.
  vector<Action> historial_acciones; // Memoria de acciones recientes para el sistema anti-bucles.
  string permitidas_nivel0 = "UDC";

  //NIVEL 1:
  string permitidas_nivel1 = "?CDUXSHA";

  //NIVEL 2: ESCAPE DEL INGENIERO
  list<Action> planEscape;

  //NIVEL 3: MINIMO COSTE (Algoritmo A*)
  
  bool hayPlan;                         // Indica si hay un plan establecido que ejecutar
  list <Action> planMovimiento;         // Almacena la secuencia de acciones a realizar
  
  //NIVEL 5
  enum EstadoObraT {ESPERANDO, CAMINO_PRIMERA_CASILLA, CAMINO_PASO_PROPIO,  ALINEANDO_INSTALANDO};

  EstadoObraT estadoObra;
  bool instalandoAntes;
  int destinoF, destinoC;
  bool modo_rodeo;
  bool primera_casilla_alcanzada;
  bool ingeniero_listo;
  int ingF, ingC;

  //NIVEL 6
  enum EstadoNivel6{FASE_EXPLORACION , FASE_EJECUCION};
  EstadoNivel6 estadoNivel6;



 
};

#endif

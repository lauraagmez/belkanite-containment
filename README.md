# Belkanite Containment


**Development period:** February – May 2026
**Course:** Artificial Intelligence
**University:** University of Granada
**Language:** C++

Academic Artificial Intelligence project developed in C++ as part of the Artificial Intelligence course at the University of Granada.

The project focuses on the design of autonomous agents capable of exploring an environment, planning routes, making decisions under different constraints and coordinating with each other to solve increasingly complex scenarios.

## Overview

The system includes two autonomous agents:

- **Engineer**
- **Technician**

Each agent has different capabilities and behaviours depending on the scenario. The project combines reactive decision-making, environment exploration, state-space search, heuristic planning and multi-agent coordination.

## Main features

- Autonomous agent design
- Reactive and deliberative behaviours
- Exploration of partially unknown environments
- Internal map construction using sensor information
- State-space search
- Route planning
- Heuristic search
- A* search
- Energy-aware planning
- Environmental impact constraints
- Obstacle avoidance and replanning
- Multi-agent coordination
- Pipeline network planning

## Artificial Intelligence techniques

### Reactive exploration

The agents make decisions using information received from their sensors while maintaining knowledge about previously explored areas.

They prioritise unexplored or less-visited positions and adapt their behaviour according to terrain, obstacles, height differences and other agents.

### State-space search

Navigation is modelled as a search problem considering:

- Position
- Orientation
- Available actions
- Terrain restrictions
- Height differences
- Agent equipment

Different search strategies are applied depending on the scenario.

### A* and heuristic planning

The Technician uses heuristic search and A* to calculate routes while minimising energy consumption.

The evaluation considers accumulated cost, heuristic distance, terrain type and height variations.

### Pipeline planning

The Engineer plans a pipeline route between the Belkanite installation and a treatment plant while respecting:

- Energy limits
- Ecological impact limits
- Terrain restrictions
- Terrain elevation
- Excavation operations
- Elevation operations
- Water-flow constraints

### Multi-agent coordination

In the advanced scenarios, the Engineer and Technician cooperate to construct the pipeline network.

The agents coordinate movement, terrain preparation and pipeline installation while avoiding collisions and replanning when necessary.

## Project structure

- `Comportamientos_Agentes/` — main autonomous-agent implementation
- `mapas/` — simulation environments
- `include/` — headers and simulator infrastructure
- `src/` — simulator source code
- `bin_src/` — executable entry points
- `ply/` — graphical model resources
- `CMakeLists.txt` — project build configuration

The main Artificial Intelligence implementation developed for the assignment is located in `Comportamientos_Agentes/`.

### Engineer

`ingeniero.cpp` and `ingeniero.hpp`

Includes exploration, route search, pipeline planning, ecological and energy constraints, and coordination with the Technician.

### Technician

`tecnico.cpp` and `tecnico.hpp`

Includes exploration, heuristic route planning, energy-aware search and coordination with the Engineer.

## Technologies

- C++
- STL
- CMake
- Git
- GitHub

Data structures used include `vector`, `list`, `queue`, `priority_queue`, `set`, `map` and `tuple`.

## Academic context

Developed for the **Artificial Intelligence** course during the 2025/2026 academic year at the University of Granada.

The simulation environment and part of the supporting infrastructure were provided as part of the course. The autonomous-agent behaviour and planning logic developed for the assignment is primarily contained in `Comportamientos_Agentes/`.

## Author

**Laura**  
Computer Engineering & Business Administration student  
University of Granada

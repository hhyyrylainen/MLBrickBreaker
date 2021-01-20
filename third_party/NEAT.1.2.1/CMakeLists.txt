# Modified for use in MLBrickBreaker, now this defines a single static library

cmake_minimum_required(VERSION 2.4)

# PROJECT( neat )

# FILE(GLOB neat_sources *.cpp)
SET(neat_sources gene.h gene.cpp
  genome.h genome.cpp
  innovation.h innovation.cpp
  link.h link.cpp
  neat.h neat.cpp
  network.h network.cpp
  nnode.h nnode.cpp
  organism.h organism.cpp
  population.h population.cpp
  species.h species.cpp
  trait.h trait.cpp)

# ADD_EXECUTABLE( neat ${neat_sources} )
add_library(neat STATIC ${neat_sources})

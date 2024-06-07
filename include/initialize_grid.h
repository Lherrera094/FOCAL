#ifndef INITIALIZE_GRID_H
#define INITIALIZE_GRID_H

#include <math.h>
#include <stdlib.h>
#include <string.h>
#include "focal-grid.h"

//functions in initialize grid
int set2zero_1D( size_t N_x, double arr_1D[N_x] );
int set2zero_3D( size_t N_x, size_t N_y, size_t N_z, double arr_3D[N_x][N_y][N_z] );
void gridConfInit(gridConfiguration *gridCfg, int boundary);
void antenaInit(gridConfiguration *gridCfg, beamConfiguration *beamCfg);
void gridInit(gridConfiguration *gridCfg, Grid *G);

#endif
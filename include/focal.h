// header guard first to prevent multiple declarations
#ifndef FOCAL_H
#define FOCAL_H

#include <math.h>
#include <stdlib.h>
#include <string.h>
#include "initialize_grid.h"

int advance_J( gridConfiguration *gridCfg, 
               double EB_WAVE[gridCfg->Nx][gridCfg->Ny][gridCfg->Nz], 
               double J_B0[gridCfg->Nx][gridCfg->Ny][gridCfg->Nz],
               double n_e[gridCfg->Nx/2][gridCfg->Ny/2][gridCfg->Nz/2] ); 

int advance_B( gridConfiguration *gridCfg, 
               double EB_WAVE[gridCfg->Nx][gridCfg->Ny][gridCfg->Nz] );

int advance_B_ref( gridConfiguration *gridCfg, 
                   double EB_WAVE[gridCfg->Nx][gridCfg->Ny][gridCfg->Nz_ref] );

int advance_E( gridConfiguration *gridCfg, 
               double EB_WAVE[gridCfg->Nx][gridCfg->Ny][gridCfg->Nz], 
               double J_B0[gridCfg->Nx][gridCfg->Ny][gridCfg->Nz] );

int advance_E_ref( gridConfiguration *gridCfg, 
                   double EB_WAVE[gridCfg->Nx][gridCfg->Ny][gridCfg->Nz_ref] ); 


#endif  // FOCAL_H


#ifndef BACKGROUND_PROFILES_H
#define BACKGROUND_PROFILES_H

#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#include "focal.h"
#include "hdf5.h"
#include "grid_io.h"

int make_density_profile( gridConfiguration *gridCfg, 
                          double cntrl_para, 
                          double n_e[gridCfg->Nx/2][gridCfg->Ny/2][gridCfg->Nz/2] );

int make_B0_profile( gridConfiguration *gridCfg,
                     double cntrl_para, 
                     double J_B0[gridCfg->Nx][gridCfg->Ny][gridCfg->Nz] );

#endif  // BACKGROUND_PROFILES_H

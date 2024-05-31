#ifndef POWER_CALC_H
#define POWER_CALC_H

#include <math.h>
#include <stdlib.h>
#include <string.h>
#include "focal.h"

double calc_poynt_4( gridConfiguration *gridCfg, 
                     int pwr_dect, char absorber[],
                     double EB_WAVE[gridCfg->Nx][gridCfg->Ny][gridCfg->Nz], 
                     double EB_WAVE_ref[gridCfg->Nx][gridCfg->Ny][gridCfg->Nz_ref] );
                     
double calc_poynt_5( size_t N_x, size_t N_y, size_t N_z, size_t N_z_ref,
                     int pwr_dect, char absorber[],
                     double EB_WAVE[N_x][N_y][N_z], double EB_WAVE_ref[N_x][N_y][N_z_ref] );
double calc_poynt_6( size_t N_x, size_t N_y, size_t N_z, size_t N_z_ref,
                     int pwr_dect, char absorber[],
                     double EB_WAVE[N_x][N_y][N_z], double EB_WAVE_ref[N_x][N_y][N_z_ref] );
double calc_poynt_7( size_t N_x, size_t N_y, size_t N_z, size_t N_z_ref,
                     int pwr_dect, char absorber[],
                     double EB_WAVE[N_x][N_y][N_z], double EB_WAVE_ref[N_x][N_y][N_z_ref] );
double calc_power_EE_1( size_t N_x, size_t N_y, size_t N_z, size_t N_z_ref,
                       int pwr_dect, char absorber[],
                       double EB_WAVE[N_x][N_y][N_z], double EB_WAVE_ref[N_x][N_y][N_z_ref] );

#endif // POWER_CALC_H

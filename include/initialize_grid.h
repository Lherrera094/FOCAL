#ifndef INITIALIZE_GRID_H
#define INITIALIZE_GRID_H

#include <math.h>
#include <stdlib.h>
#include <string.h>

// define structures
/*Grid configuration Structure*/
typedef struct gridConfiguration {
    int
        Nx, Ny, Nz,     // maybe size_t would be better
        Nz_ref,
        d_absorb,
        t_end,
        ne_profile, B0_profile;
    double
        period,
        dx,dt;
} gridConfiguration;

typedef struct beamConfiguration {
    int
        exc_signal,
        ant_x, ant_y, ant_z,
        rampUpMethod;
    double
        antAngle_zy, antAngle_zx,
        ant_w0x, ant_w0y,
        z2waist;
} beamConfiguration;

/*memory allocation macro*/
#define ALLOC_3D(PNTR, NUM, TYPE)                                       \
    PNTR = (TYPE *)calloc(NUM, sizeof(TYPE));                           \
    if (!PNTR){                                                         \
        perror("ALLOC_3D");                                             \
        fprintf(stderr,                                                 \
                "Allocation failed for " #PNTR ". Terminating...\n");   \
        exit(-1);                                                       \
    }

//functions in initialize grid
int set2zero_1D( size_t N_x, double arr_1D[N_x] );
int set2zero_3D( size_t N_x, size_t N_y, size_t N_z, double arr_3D[N_x][N_y][N_z] );
void gridInit(gridConfiguration *gridCfg, int boundary);

#endif
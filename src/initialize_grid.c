#include "initialize_grid.h"

int set2zero_1D( size_t N_x, double arr_1D[N_x] ){
//{{{

    size_t
        ii;

#pragma omp parallel for default(shared) private(ii)
    for (ii=0 ; ii<N_x ; ++ii) {
        arr_1D[ii] = .0;
    }

    return EXIT_SUCCESS;
} //}}}


int set2zero_3D( size_t N_x, size_t N_y, size_t N_z, double arr_3D[N_x][N_y][N_z] ){
//{{{

    size_t
        ii, jj, kk;

#pragma omp parallel for collapse(3) default(shared) private(ii,jj,kk)
    for (ii=0 ; ii<N_x ; ++ii) {
        for (jj=0 ; jj<N_y ; ++jj) {
            for (kk=0 ; kk<N_z ; ++kk) {
                arr_3D[ii][jj][kk]  = .0;
            }
        }
    }

    return EXIT_SUCCESS;
} //}}}

void gridInit(gridConfiguration *gridCfg, int boundary){

    int scale;

    // set-up grid
    scale           = 1;
    gridCfg->period  = 16*scale;

    if (boundary == 1){
        gridCfg->d_absorb= (int)(3*gridCfg->period);
    }else if (boundary ==2){
        gridCfg->d_absorb= 8;
    }
     
    gridCfg->Nx  = (400+0*200)*scale;
    gridCfg->Ny  = (300+0*100)*scale;
    gridCfg->Nz  = (200+0*150)*scale;
    gridCfg->Nz_ref  = 2*gridCfg->d_absorb + (int)gridCfg->period;
    gridCfg->t_end   = (int)((100-50)*gridCfg->period);

    gridCfg->B0_profile  = 1;
    gridCfg->ne_profile  = 3;

    // dt/dx = 0.5 is commenly used in 2D FDTD codes
    // Note that period refers to the wavelength in the numerical grid and not
    // in the "physical" grid (where one grid cell is equal to one Yee cell).
    // This means that in the physical grid, the wavelength is period/2, thus
    // in the equations we have to use period/2 for the wavelength.
    gridCfg->dx  = 1./(gridCfg->period/2);
    gridCfg->dt  = 1./(2.*(gridCfg->period/2));

}
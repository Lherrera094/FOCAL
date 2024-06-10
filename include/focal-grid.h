#ifndef FOCAL_GRID_H
#define FOCAL_GRID_H

// define structures
/*Grid configuration Structure*/
typedef struct gridConfiguration {
    int
        Nx, Ny, Nz,     // maybe size_t would be better
        Nz_ref,
        d_absorb,
        t_end,
        scale,
        ne_profile, B0_profile;
    double
        period,
        dx,dt;
    const char  *foldername, *path, *filename_h5, 
                *filename_timetraces;    
} gridConfiguration;

typedef struct Grid {
    double *EB_WAVE; /**EB_WAVE_REF,
           *J_B0, n_e; 
           data2save, 
           *antField_xy, *antPhaseTerms,
           */
} Grid;

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

#endif

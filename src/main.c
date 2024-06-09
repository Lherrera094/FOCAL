/**
 * Author:      Alf Köhn-Seemann
 * Email:       koehn@igvp.uni-stuttgart.de
 * Copyright:   University of Stuttgart
 * 
 * This is a 3D FDTD code for simulating electromagnetic waves in cold 
 * magnetized plasmas.
 *
 * NOTE: This is an early version, including some obsolete function, those
 *       will be removed in near future.
 *       Furthermore, everything will be properly split into separate 
 *       libraries, allowing the usage of a nice make file.
 *
 * Initial release on github: 2022-03-31
 *
 **/

#include "macros.h"
#include "focal-grid.h"
#include "focal-alloc.h"

#include "save_data.h"
#include "initialize_grid.h"
#include "focal.h"
#include "antenna.h"
#include "grid_io.h"
#include "boundaries.h"
#include "background_profiles.h"
#include "power_calc.h"


int main( int argc, char *argv[] ) {
//{{{
    //Initialize running time
    clock_t start, end;
    double cpu_time_used;
    start = clock();
    

    struct gridConfiguration gridCfg;
    struct beamConfiguration beamCfg;
    //struct Grid g;

    int
        ii,jj,kk,
        t_int, T_wave, 

#ifdef _OPENMP
        n_threads,                          // number of threads that will be used (OpenMP)
#endif
        pwr_dect,

#ifdef DETECTOR_ANTENNA_1D
        detAnt_01_zpos,
        detAnt_02_zpos,
        detAnt_03_zpos,
        detAnt_04_zpos,
        detAnt_01_ypos,
#endif

        len_str,                            // return value of snprintf
        opt_ret;                            // return value of getopt (reading input parameter)

    double
#if BOUNDARY == 1
        eco,
#endif

        //ant_phase, 

        poynt_x1, poynt_x2,
        poynt_y1, poynt_y2,
        poynt_z1, poynt_z2,
        poynt_z1_ref,

        power_abs_x1, power_abs_x2,
        power_abs_y1, power_abs_y2,
        power_abs_z1, power_abs_z2,
        power_abs_ref,

        /*
        power_EE_x1, power_EE_x2, 
        power_EE_y1, power_EE_y2, 
        power_EE_z1, power_EE_z2, 
        power_EE_ref, 
        */

        omega_t;

    char
        dSet_name[PATH_MAX],                //name of the saved variables in the hdf5 
        filename_hdf5[PATH_MAX],            // filename of hdf5 file for output
        filename_timetrazes[PATH_MAX],
        fullDir[2024];

    bool
        angle_zx_set,                       // is antAngle_zx set during call ?
        angle_zy_set;                       // is antAngle_zy set during call ?

    int boundary = 1;

    gridConfInit( &gridCfg, boundary);      //Initialize grid configuration values

    /*
    const char* foldername = "Prueba_1"; 
    const char* path = "../Simulations";
    const char* filename_h5 = "fileout.h5";
    char fullDir[2024];
    */
    /*Folder creation for results storage.*/
    create_folder_path( &gridCfg);
    
    if (snprintf(fullDir, sizeof(fullDir),"%s/%s", gridCfg.path, gridCfg.foldername) >= sizeof(fullDir)) {
        fprintf(stderr, "Error: Directory path is too long.\n");
        return -1;
    }
    //gridInit( &gridCfg, &g);

    // arrays realized as variable-length array (VLA)
    // E- and B-wavefield
    double (*EB_WAVE)[gridCfg.Ny][gridCfg.Nz]           = calloc(gridCfg.Nx, sizeof *EB_WAVE);
    double (*EB_WAVE_ref)[gridCfg.Ny][gridCfg.Nz_ref]   = calloc(gridCfg.Nx, sizeof *EB_WAVE_ref);
    // J-wavefield (in plasma) and background magnetic field
    double (*J_B0)[gridCfg.Ny][gridCfg.Nz]              = calloc(gridCfg.Nx, sizeof *J_B0);
    // background electron plasma density
    double (*n_e)[gridCfg.Ny/2][gridCfg.Nz/2]           = calloc(gridCfg.Nx/2, sizeof *n_e);
    // used when writing data into hdf5-files
    double (*data2save)[gridCfg.Ny/2][gridCfg.Nz/2]     = calloc(gridCfg.Nx/2, sizeof *data2save);
    // antenna: envelope of injected field
    double (*antField_xy)[gridCfg.Ny/2]                 = calloc(gridCfg.Nx/2, sizeof *antField_xy);
    // antenna: phase terms 
    double (*antPhaseTerms)[gridCfg.Ny/2]               = calloc(gridCfg.Nx/2, sizeof *antPhaseTerms);
    // time traces
    double (*timetraces)[8]                             = calloc((gridCfg.t_end/(int)gridCfg.period), sizeof *timetraces);
    
    // old E-fields required for Mur's boundary condition
#if BOUNDARY == 2
    double (*E_Xdir_OLD)[gridCfg.Ny][gridCfg.Nz]        = calloc(8,  sizeof *E_Xdir_OLD);
    double (*E_Ydir_OLD)[8][gridCfg.Nz]                 = calloc(gridCfg.Nx, sizeof *E_Ydir_OLD);
    double (*E_Zdir_OLD)[gridCfg.Ny][8]                 = calloc(gridCfg.Nx, sizeof *E_Zdir_OLD);
    double (*E_Xdir_OLD_ref)[gridCfg.Ny][gridCfg.Nz_ref]= calloc(8,  sizeof *E_Xdir_OLD_ref);
    double (*E_Ydir_OLD_ref)[8][gridCfg.Nz_ref]         = calloc(gridCfg.Nx, sizeof *E_Ydir_OLD_ref);
    double (*E_Zdir_OLD_ref)[gridCfg.Ny][8]             = calloc(gridCfg.Nx, sizeof *E_Zdir_OLD_ref);
#endif

    // array for detector antennas
    // sum_t(Ex*Ex) | sum_t(Ey*Ey) | sum_t(Ez*Ez) | sum_t(E*E) | rms(E)
#ifdef DETECTOR_ANTENNA_1D
    // TODO: change into 3D array, such that each detector antenna corresponds
    //       to one 2D array; that way it can be written much more failsafe...
    //       requires some changes in procedures for storing and saving
    double (*detAnt_01_fields)[5]       = calloc(gridCfg.Nx, sizeof *detAnt_01_fields);
    double (*detAnt_02_fields)[5]       = calloc(gridCfg.Nx, sizeof *detAnt_02_fields);
    double (*detAnt_03_fields)[5]       = calloc(gridCfg.Nx, sizeof *detAnt_03_fields);
    double (*detAnt_04_fields)[5]       = calloc(gridCfg.Nx, sizeof *detAnt_04_fields);
#endif

    antenaInit( &gridCfg, &beamCfg);
    
    // reading input parameter
    // used for checking if input parameter was provided
    angle_zx_set    = false;
    angle_zy_set    = false;

    // loop through input parameter
    printf( "number of input parameters provided during call: %d\n", argc-1 );
    while ( (opt_ret = getopt(argc, argv, "a:b:")) != -1 ){
        switch (opt_ret) {
            // angle between z=const plane and x=const plane
            case 'a': beamCfg.antAngle_zx   = atof(optarg);
                      angle_zx_set  = true;
                      break;
            case 'b': beamCfg.antAngle_zy   = atof(optarg);
                      angle_zy_set  = true;
                      break;
        }
    }
    if ( argc > 1 ) {
        printf( "following parameters were set during call: \n" );
        if (angle_zx_set)   printf( "    antAngle_zx = %f\n", beamCfg.antAngle_zx );
        if (angle_zy_set)   printf( "    antAngle_zy = %f\n", beamCfg.antAngle_zy );
    }

    pwr_dect    = gridCfg.d_absorb;

#ifdef DETECTOR_ANTENNA_1D
    detAnt_01_ypos  = beamCfg.ant_y;
    detAnt_01_zpos  = beamCfg.ant_z+2;
    detAnt_02_zpos  = round(beamCfg.ant_z+2 + 1*5*gridCfg.period); // steps of 5 cm for 28 GHz = 4.67*period
    detAnt_03_zpos  = round(beamCfg.ant_z+2 + 2*5*gridCfg.period);
    detAnt_04_zpos  = round(beamCfg.ant_z+2 + 3*5*gridCfg.period);
    // positions have to be even numbers, to ensure fields are accessed correctly
    if ((detAnt_01_ypos % 2) != 0)  ++detAnt_01_ypos;
    if ((detAnt_01_zpos % 2) != 0)  ++detAnt_01_zpos;
    if ((detAnt_02_zpos % 2) != 0)  ++detAnt_02_zpos;
    if ((detAnt_03_zpos % 2) != 0)  ++detAnt_03_zpos;
    if ((detAnt_04_zpos % 2) != 0)  ++detAnt_04_zpos;
    // issue a warning when detector antenna position is beyond Nz
    if (detAnt_04_zpos > (gridCfg.Nz - gridCfg.d_absorb)) {
        printf( "ERROR: check the detector antenna positions into z direction\n" );
        printf( "       Nz-d_absorb = %d, detAnt_04_zpos = %d\n", 
                gridCfg.Nz-gridCfg.d_absorb, detAnt_04_zpos );
    }
#endif


#if BOUNDARY == 1
    eco         = 10./(double)(gridCfg.period);
#endif

    T_wave      = 0;
    omega_t     = .0;

    // the arrays are initialized with calloc() and thus don't require zeroing
    printf( "starting to set all variables to 0...\n" );
    power_abs_x1    = .0;
    power_abs_x2    = .0;
    power_abs_y1    = .0;
    power_abs_y2    = .0;
    power_abs_z1    = .0;
    power_abs_z2    = .0;
    power_abs_ref   = 1e-7;
    poynt_x1       = .0;
    poynt_x2       = .0;
    poynt_y1       = .0;
    poynt_y2       = .0;
    poynt_z1       = .0;
    poynt_z1_ref   = .0;
    poynt_z2       = .0;
    
    /*
    power_EE_x1    = .0;
    power_EE_x2    = .0;
    power_EE_y1    = .0;
    power_EE_y2    = .0;
    power_EE_z1    = .0;
    power_EE_z2    = .0;
    power_EE_ref   = .0;
    */
    printf( "...done setting all variables to 0\n" );

    printf( "starting do define antenna field...\n" );
    make_antenna_profile( &gridCfg, &beamCfg, 
                          antField_xy, antPhaseTerms );
    printf( "...done defining antenna field\n" );

    printf( "starting defining background plasma density\n" );
            // ne_profile: 1 = plasma mirror
            //             2 = linearly increasing profile
    make_density_profile( &gridCfg,  
            // cntrl_para: ne_profile=1 --> 0: plane mirror; oblique mirror: -.36397; 20 degrees: -.17633
            //             ne_profile=2 --> k0*Ln: 25
            25,
            n_e );
    printf( " ...setting density in absorber to 0...\n ");
    //set_densityInAbsorber_v2( &gridCfg, "z1", n_e );
    //set_densityInAbsorber_v2( &gridCfg, "x1x2y1y2z1", n_e );
    printf( "...done defining background plasma density\n" );

    printf( "starting defining background magnetic field...\n" );
    // B0_profile: 1 = constant field
    make_B0_profile(
            &gridCfg,
            // cntrl_para: B0_profile=1 --> value of X
            .85, 
            J_B0 );
    printf( "...done defining background magnetic field\n" );

    // print some info to console
    printf( "Nx = %d, Ny = %d, Nz = %d\n", gridCfg.Nx, gridCfg.Ny, gridCfg.Nz );
    printf( "period = %d\n", (int)(gridCfg.period) );
    printf( "d_absorb = %d\n", gridCfg.d_absorb );
    printf( "t_end = %d\n", (int)(gridCfg.t_end) );
    printf( "antAngle_zx = %.2f, antAngle_zy = %.2f\n", beamCfg.antAngle_zx, beamCfg.antAngle_zy );
    printf( "ant_w0x = %.2f, ant_w0y = %.2f\n", beamCfg.ant_w0x, beamCfg.ant_w0y ); 
    printf( "ant_x = %d, ant_y = %d, ant_z = %d\n", beamCfg.ant_x, beamCfg.ant_y, beamCfg.ant_z );
    printf( "Boundary condition set to '%d'\n", BOUNDARY );
#ifdef DETECTOR_ANTENNA_1D
    printf( "detector antenna positions: z1 = %d, y1 = %d\n", detAnt_01_zpos, detAnt_01_ypos );
    printf( "detector antenna positions: z2 = %d, y1 = %d\n", detAnt_02_zpos, detAnt_01_ypos );
    printf( "detector antenna positions: z3 = %d, y1 = %d\n", detAnt_03_zpos, detAnt_01_ypos );
    printf( "detector antenna positions: z4 = %d, y1 = %d\n", detAnt_04_zpos, detAnt_01_ypos );
#endif

#ifdef _OPENMP
#pragma omp parallel private(n_threads)
    {
    n_threads = omp_get_num_threads();
    printf( "number of threads that will be used (OpenMP) = %d\n", n_threads );
    }
#endif

    /*System's time evolution*/
    for (t_int=0 ; t_int <=gridCfg.t_end ; ++t_int) {
        
        omega_t += 2.*M_PI/gridCfg.period;

        // to avoid precision problems when a lot of pi's are summed up        
        if (omega_t >= 2.*M_PI) {
            omega_t    += -2.*M_PI;
            T_wave     += 1;
            //printf("status: number of oscillation periods: %d (t_int= %d) \n",T_wave,t_int);
        }

        // add source
        add_source( &gridCfg, &beamCfg,
                    .85*0.,     // .85=Y, this values should be calculated/extracted from ne-profile
                    t_int, omega_t, 
                    antField_xy, antPhaseTerms, EB_WAVE );
        add_source_ref( &gridCfg, &beamCfg,
                        .85*0.,     // .85=Y, this values should be calculated/extracted from ne-profile
                        t_int, omega_t, 
                        antField_xy, antPhaseTerms, EB_WAVE_ref );

        // apply absorbers
#if BOUNDARY == 1
        apply_absorber(     &gridCfg, eco, EB_WAVE );
        apply_absorber_ref( &gridCfg, eco, EB_WAVE_ref );
#endif

        // advance J
        // Jx: odd-even-even
        // Jy: even-odd-even
        // Jz: even-even-odd
        // B0x: even-odd-odd
        // B0y: odd-even-odd
        // B0z: odd-odd-even
        advance_J( &gridCfg, EB_WAVE, J_B0, n_e );

        // advance B
        advance_B(     &gridCfg, EB_WAVE );
        advance_B_ref( &gridCfg, EB_WAVE_ref );
        
        // advance E
        advance_E(     &gridCfg, EB_WAVE,     J_B0 );
        advance_E_ref( &gridCfg, EB_WAVE_ref       );

        // optionally, apply numerical viscosity
        //apply_numerical_viscosity( &gridCfg, EB_WAVE );

        // apply Mur's boundary conditions
#if BOUNDARY == 2
        abc_Mur_1st( &gridCfg, "x1x2y1y2z1z2",  
                     EB_WAVE, E_Xdir_OLD, E_Ydir_OLD, E_Zdir_OLD );
        abc_Mur_1st_ref( &gridCfg, 
                         EB_WAVE_ref, E_Xdir_OLD_ref, E_Ydir_OLD_ref, E_Zdir_OLD_ref );
        abc_Mur_saveOldE_xdir(    &gridCfg, EB_WAVE, E_Xdir_OLD );
        abc_Mur_saveOldE_ydir(    &gridCfg, EB_WAVE, E_Ydir_OLD );
        abc_Mur_saveOldE_zdir(    &gridCfg, EB_WAVE, E_Zdir_OLD );
        abc_Mur_saveOldEref_xdir( &gridCfg, EB_WAVE_ref, E_Xdir_OLD_ref );
        abc_Mur_saveOldEref_ydir( &gridCfg, EB_WAVE_ref, E_Ydir_OLD_ref );
        abc_Mur_saveOldEref_zdir( &gridCfg, EB_WAVE_ref, E_Zdir_OLD_ref );
#endif

#ifdef DETECTOR_ANTENNA_1D
        // store wavefields for detector antennas over the final 10 
        // oscillation periods, it was found previously that only one period
        // does not result in a too nice average
        if ( t_int >= (gridCfg.t_end-10*gridCfg.period) ) {
            if (detAnt_01_zpos < (gridCfg.Nz - gridCfg.d_absorb)) {
                detAnt1D_storeValues( &gridCfg, detAnt_01_ypos, detAnt_01_zpos,
                                      t_int,  
                                      EB_WAVE, detAnt_01_fields );
            }
            if (detAnt_02_zpos < (gridCfg.Nz - gridCfg.d_absorb)) {
                detAnt1D_storeValues( &gridCfg, detAnt_01_ypos, detAnt_02_zpos,
                                      t_int, 
                                      EB_WAVE, detAnt_02_fields );
            }
            if (detAnt_03_zpos < (gridCfg.Nz - gridCfg.d_absorb)) {
                detAnt1D_storeValues( &gridCfg, detAnt_01_ypos, detAnt_03_zpos,
                                      t_int,
                                      EB_WAVE, detAnt_03_fields );
            }
            if (detAnt_04_zpos < (gridCfg.Nz - gridCfg.d_absorb)) {
                detAnt1D_storeValues( &gridCfg, detAnt_01_ypos, detAnt_04_zpos,
                                      t_int,
                                      EB_WAVE, detAnt_04_fields );
            }
        }
#endif

        // IQ detector for power detection
        if ( t_int >= 20*gridCfg.period ) {
            // z1-plane and z2-plane
            poynt_z1_ref    = calc_poynt_4( &gridCfg, pwr_dect, "ref_z1", EB_WAVE, EB_WAVE_ref );
            poynt_z1        = calc_poynt_4( &gridCfg, pwr_dect, "z1",     EB_WAVE, EB_WAVE_ref );
            poynt_z2        = calc_poynt_4( &gridCfg, pwr_dect, "z2",     EB_WAVE, EB_WAVE_ref );
            // x1-plane and x2-plane
            poynt_x1        = calc_poynt_4( &gridCfg, pwr_dect, "x1", EB_WAVE, EB_WAVE_ref );
            poynt_x2        = calc_poynt_4( &gridCfg, pwr_dect, "x2", EB_WAVE, EB_WAVE_ref );
            // y1-plane and y2-plane
            poynt_y1        = calc_poynt_4( &gridCfg, pwr_dect, "y1", EB_WAVE, EB_WAVE_ref );
            poynt_y2        = calc_poynt_4( &gridCfg, pwr_dect, "y2", EB_WAVE, EB_WAVE_ref );

            
//            printf( "t = %d, power_abs_ref = %13.5e, power_abs_z1 = %13.5e, power_abs_z2 = %13.5e, poynt_z1 = %13.5e, poynt_z2 = %13.5e\n",
//                    t_int, power_abs_ref, power_abs_z1, power_abs_z2, poynt_z1, poynt_z2 );

            power_abs_ref   = .99*power_abs_ref + .01*poynt_z1_ref;
            power_abs_z1    = .99*power_abs_z1  + .01*poynt_z1;
            power_abs_z2    = .99*power_abs_z2  + .01*poynt_z2;
            power_abs_x1    = .99*power_abs_x1  + .01*poynt_x1;
            power_abs_x2    = .99*power_abs_x2  + .01*poynt_x2;
            power_abs_y1    = .99*power_abs_y1  + .01*poynt_y1;
            power_abs_y2    = .99*power_abs_y2  + .01*poynt_y2;

            /*
            // EE
            // z1-plane and z2-plane
            power_EE_ref    += calc_power_EE_1( gridCfg.Nx, gridCfg.Ny, gridCfg.Nz, gridCfg.Nz_ref, gridCfg.d_absorb, "ref_z1", EB_WAVE, EB_WAVE_ref );
            power_EE_z1     += calc_power_EE_1( gridCfg.Nx, gridCfg.Ny, gridCfg.Nz, gridCfg.Nz_ref, gridCfg.d_absorb, "z1",     EB_WAVE, EB_WAVE_ref );
            power_EE_z2     += calc_power_EE_1( gridCfg.Nx, gridCfg.Ny, gridCfg.Nz, gridCfg.Nz_ref, gridCfg.d_absorb, "z2",     EB_WAVE, EB_WAVE_ref );
            // x1-plane and x2-plane
            power_EE_x1     += calc_power_EE_1( gridCfg.Nx, gridCfg.Ny, gridCfg.Nz, gridCfg.Nz_ref, gridCfg.d_absorb, "x1",     EB_WAVE, EB_WAVE_ref );
            power_EE_x2     += calc_power_EE_1( gridCfg.Nx, gridCfg.Ny, gridCfg.Nz, gridCfg.Nz_ref, gridCfg.d_absorb, "x2",     EB_WAVE, EB_WAVE_ref );
            // y1-plane and y2-plane
            power_EE_y1     += calc_power_EE_1( gridCfg.Nx, gridCfg.Ny, gridCfg.Nz, gridCfg.Nz_ref, gridCfg.d_absorb, "y1",     EB_WAVE, EB_WAVE_ref );
            power_EE_y2     += calc_power_EE_1( gridCfg.Nx, gridCfg.Ny, gridCfg.Nz, gridCfg.Nz_ref, gridCfg.d_absorb, "y2",     EB_WAVE, EB_WAVE_ref );
            */

        }


        if ( (t_int % (int)(gridCfg.period)) == 4 )  {
            printf( "status: number of oscillation periods: %d (t_int= %d) \n",T_wave,t_int);
            printf( "        Poynting-power: z1 = %13.6e, z2 = %13.6e, x1 = %13.6e, x2 = %13.6e, y1 = %13.6e, y2 = %13.6e, (z1+z2+x1+x2+y1+y2)/z1_ref = %13.6e %%\n",
                    power_abs_z1/power_abs_ref, 
                    power_abs_z2/power_abs_ref,
                    power_abs_x1/power_abs_ref, 
                    power_abs_x2/power_abs_ref,
                    power_abs_y1/power_abs_ref, 
                    power_abs_y2/power_abs_ref,
                    (power_abs_x1+power_abs_x2 + power_abs_y1+power_abs_y2 + power_abs_z1+power_abs_z2)/power_abs_ref * 100.
                    );
            /*
            printf( "        Power_EE_d-abs: z1 = %13.6e, z2 = %13.6e, x1 = %13.6e, x2 = %13.6e, y1 = %13.6e, y2 = %13.6e, ref = %13.6e\n",
                    power_EE_z1, 
                    power_EE_z2,
                    power_EE_x1, 
                    power_EE_x2,
                    power_EE_y1, 
                    power_EE_y2,
                    power_EE_ref
//                    (power_abs_x1+power_abs_x2 + power_abs_y1+power_abs_y2 + power_abs_z1+power_abs_z2)/power_abs_ref * 100.
                    );
            */
            timetraces[T_wave][0]   = (double)t_int;
            timetraces[T_wave][1]   = (double)T_wave;
            timetraces[T_wave][2]   = power_abs_z1/power_abs_ref;
            timetraces[T_wave][3]   = power_abs_z2/power_abs_ref;
            timetraces[T_wave][4]   = power_abs_x1/power_abs_ref;
            timetraces[T_wave][5]   = power_abs_x2/power_abs_ref;
            timetraces[T_wave][6]   = power_abs_y1/power_abs_ref;
            timetraces[T_wave][7]   = power_abs_y2/power_abs_ref;

        }
    } // end of time loop

    printf( "-------------------------------------------------------------------------------------------------------------\n" );
    printf( "  T   |   poynt_z1   |   poynt_z2   |   poynt_x1   |   poynt_x2   |   poynt_y1   |   poynt_y2   |  P_out     \n" );
    printf( "------+--------------+--------------+--------------+--------------+--------------+--------------+------------\n" );
    for ( ii=0 ; ii<(gridCfg.t_end/(int)gridCfg.period) ; ++ii )
        printf( " %4d |%13.6e |%13.6e |%13.6e |%13.6e |%13.6e |%13.6e |%13.6e\n",
                (int)timetraces[ii][1], //timetraces[ii][1],
                timetraces[ii][2], timetraces[ii][3],
                timetraces[ii][4], timetraces[ii][5],
                timetraces[ii][6], timetraces[ii][7],
                (timetraces[ii][2]+timetraces[ii][3] + timetraces[ii][4]+timetraces[ii][5] + timetraces[ii][6]+timetraces[ii][7])
              );
    printf( "-------------------------------------------------------------------------------------------------------------\n" );

    // write timetrace data into file
    // open file in w(rite) mode; might consider using a+ instead    
    snprintf( filename_timetrazes, sizeof(filename_timetrazes), "%s/%s", fullDir, gridCfg.filename_timetraces);
    writeTimetraces2ascii( (gridCfg.t_end/(int)gridCfg.period), 8, gridCfg.t_end, gridCfg.period, 
                           filename_timetrazes, timetraces );

    // save into hdf5
    // abs(E)
    // prepare array for that
#pragma omp parallel for collapse(3) default(shared) private(ii,jj,kk)
    for (ii=0 ; ii<gridCfg.Nx ; ii+=2) {
        for (jj=0 ; jj<gridCfg.Ny ; jj+=2) {
            for (kk=0 ; kk<gridCfg.Nz ; kk+=2) {
                data2save[(ii/2)][(jj/2)][(kk/2)] = 
                    sqrt( pow(EB_WAVE[ii+1][jj  ][kk  ],2) 
                         +pow(EB_WAVE[ii  ][jj+1][kk  ],2) 
                         +pow(EB_WAVE[ii  ][jj  ][kk+1],2) );
            }
        }
    }
    len_str = snprintf( filename_hdf5, sizeof(filename_hdf5), "%s/%s", fullDir, gridCfg.filename_h5);
    if ( (len_str < 0) || (len_str >= sizeof(filename_hdf5)) ) {
        printf( "ERROR: could not write filename_hdf5 string\n" );  // use a proper error handler here
    } else {
        sprintf( dSet_name, "E_abs__tint%05d", t_int );
        printf( "status of writeMyHDF_v4: %d\n", writeMyHDF_v4( gridCfg.Nx/2, gridCfg.Ny/2, gridCfg.Nz/2, filename_hdf5, dSet_name, data2save) ) ;
    }
    set2zero_3D( gridCfg.Nx/2, gridCfg.Ny/2, gridCfg.Nz/2, data2save );
    // density
    sprintf( dSet_name, "n_e" );
    printf( "status of writeMyHDF_v4: %d\n", writeMyHDF_v4( gridCfg.Nx/2, gridCfg.Ny/2, gridCfg.Nz/2, filename_hdf5, dSet_name, n_e) ) ;
    // background magnetic field
    // B0x: even-odd-odd
#pragma omp parallel for collapse(3) default(shared) private(ii,jj,kk)
    for (ii=0 ; ii<gridCfg.Nx ; ii+=2) {
        for (jj=0 ; jj<gridCfg.Ny ; jj+=2) {
            for (kk=0 ; kk<gridCfg.Nz ; kk+=2) {
                data2save[(ii/2)][(jj/2)][(kk/2)] = J_B0[ii  ][jj+1][kk+1];
            }
        }
    }
    printf( "status of writeMyHDF_v4: %d\n", writeMyHDF_v4( gridCfg.Nx/2, gridCfg.Ny/2, gridCfg.Nz/2, filename_hdf5, "B0x", data2save) ) ;
    set2zero_3D( gridCfg.Nx/2, gridCfg.Ny/2, gridCfg.Nz/2, data2save );
    // B0y: odd-even-odd
#pragma omp parallel for collapse(3) default(shared) private(ii,jj,kk)
    for (ii=0 ; ii<gridCfg.Nx ; ii+=2) {
        for (jj=0 ; jj<gridCfg.Ny ; jj+=2) {
            for (kk=0 ; kk<gridCfg.Nz ; kk+=2) {
                data2save[(ii/2)][(jj/2)][(kk/2)] = J_B0[ii+1][jj  ][kk+1];
            }
        }
    }
    printf( "status of writeMyHDF_v4: %d\n", writeMyHDF_v4( gridCfg.Nx/2, gridCfg.Ny/2, gridCfg.Nz/2, filename_hdf5, "B0y", data2save) ) ;
    set2zero_3D( gridCfg.Nx/2, gridCfg.Ny/2, gridCfg.Nz/2, data2save );
    // B0z: odd-odd-even
#pragma omp parallel for collapse(3) default(shared) private(ii,jj,kk)
    for (ii=0 ; ii<gridCfg.Nx ; ii+=2) {
        for (jj=0 ; jj<gridCfg.Ny ; jj+=2) {
            for (kk=0 ; kk<gridCfg.Nz ; kk+=2) {
                data2save[(ii/2)][(jj/2)][(kk/2)] = J_B0[ii+1][jj+1][kk  ];
            }
        }
    }
    printf( "status of writeMyHDF_v4: %d\n", writeMyHDF_v4( gridCfg.Nx/2, gridCfg.Ny/2, gridCfg.Nz/2, filename_hdf5, "B0z", data2save) ) ;
    set2zero_3D( gridCfg.Nx/2, gridCfg.Ny/2, gridCfg.Nz/2, data2save );

    writeConfig2HDF( &gridCfg, &beamCfg, filename_hdf5 );


#if defined(HDF5) && defined(DETECTOR_ANTENNA_1D)
    if (detAnt_01_zpos < (gridCfg.Nz - gridCfg.d_absorb)) {
        detAnt1D_write2hdf5( gridCfg.Nx, filename_hdf5, "/detAnt_01" , 
                             detAnt_01_ypos, detAnt_01_zpos,
                             detAnt_01_fields );
    }
    if (detAnt_02_zpos < (gridCfg.Nz - gridCfg.d_absorb)) {
        detAnt1D_write2hdf5( gridCfg.Nx, filename_hdf5, "/detAnt_02" , 
                             detAnt_01_ypos, detAnt_02_zpos,
                             detAnt_02_fields );
    }
    if (detAnt_03_zpos < (gridCfg.Nz - gridCfg.d_absorb)) {
        detAnt1D_write2hdf5( gridCfg.Nx, filename_hdf5, "/detAnt_03" , 
                             detAnt_01_ypos, detAnt_03_zpos,
                             detAnt_03_fields );
    }
    if (detAnt_04_zpos < (gridCfg.Nz - gridCfg.d_absorb)) {
        detAnt1D_write2hdf5( gridCfg.Nx, filename_hdf5, "/detAnt_04" , 
                             detAnt_01_ypos, detAnt_04_zpos,
                             detAnt_04_fields );
    }
#endif

    free( EB_WAVE );
    printf( "freed EB_WAVE\n" );
    free( J_B0 );
    printf( "freed J_B0\n" );
    free( n_e );
    printf( "freed n_e\n" );
    free( data2save );
    printf( "freed data2save\n" );

    end = clock(); 
    cpu_time_used = ( (double)(end - start) )/CLOCKS_PER_SEC;
    printf("Running time: %.2e s.\n", cpu_time_used);
    
    return EXIT_SUCCESS;
}//}}}

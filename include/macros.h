#ifndef MACROS_H
#define MACROS_H

//libraries
#include <limits.h>
#include <math.h>
#include <stdio.h>
#include <string.h>
#include <strings.h>
#include <stdlib.h>
#include <stdarg.h>
#include <getopt.h>
#include <sys/stat.h>
#include <stdbool.h>
#include <time.h>


// check if compiler understands OMP, if not, this file does probably not exist
#ifdef _OPENMP
    #include <omp.h>  
#endif

#define HDF5
#ifdef HDF5
    #include "hdf5.h"
#endif

#ifndef M_PI
  #define M_PI 3.14159265358979323846
#endif

// setting boundary conditions, possible choices are
// 1: simple_abc
// 2: Mur
#define BOUNDARY 1

#define DETECTOR_ANTENNA_1D

#endif
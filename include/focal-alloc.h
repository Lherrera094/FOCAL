#ifndef FOCAL_ALLOC_H
#define FOCAL_ALLOC_H

/*memory allocation macro*/
#define ALLOC_3D(PNTR, NUM, TYPE)                                       \
    PNTR = (TYPE *)calloc(NUM, sizeof(TYPE));                           \
    if (!PNTR){                                                         \
        perror("ALLOC_3D");                                             \
        fprintf(stderr,                                                 \
                "Allocation failed for " #PNTR ". Terminating...\n");   \
        exit(-1);                                                       \
    }

#endif
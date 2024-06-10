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

void gridConfInit(gridConfiguration *gridCfg, int boundary){

    /*Read JSON and extract data*/
    char *json_file = read_json();
    
    if(json_file == NULL){
        printf("JSON file doesn't exists.");
        return;
    }

    //Parse the JSON string
    cJSON *json = cJSON_Parse(json_file);
    if(json == NULL){
        printf("Error at parse JSON string");
        free(json_file);
        return;
    }

    //Extract data from JSON file. Save folder info
    cJSON *Main_Project = cJSON_GetObjectItemCaseSensitive(json, "Main_Project");   //Main Project path
    if( cJSON_IsString(Main_Project) && (Main_Project->valuestring != NULL) ){
        gridCfg->path = strdup(Main_Project->valuestring);
    }

    cJSON *FolderName = cJSON_GetObjectItemCaseSensitive(json, "foldername");       //Simulation folder name
    if( cJSON_IsString(FolderName) && (FolderName->valuestring != NULL) ){
        gridCfg->foldername = strdup(FolderName->valuestring);
    }

    cJSON *Filename_HDF5 = cJSON_GetObjectItemCaseSensitive(json, "filename_hdf5");   //filename hdf5
    if( cJSON_IsString(Filename_HDF5) && (Filename_HDF5->valuestring != NULL) ){
        gridCfg->filename_h5 = strdup(Filename_HDF5->valuestring);
    }

    cJSON *Filename_TimeTrace = cJSON_GetObjectItemCaseSensitive(json, "filename_timetraces");   //filename datatraces
    if( cJSON_IsString(Filename_TimeTrace) && (Filename_TimeTrace->valuestring != NULL) ){
        gridCfg->filename_timetraces = strdup(Filename_TimeTrace->valuestring);
    }
    
    //Initialize Grid Configuration values
    cJSON *item_scale = cJSON_GetObjectItemCaseSensitive(json, "scale");   //scale factor
    if( cJSON_IsNumber(item_scale) ){
        gridCfg->scale = item_scale->valueint;
    }

    cJSON *item_period = cJSON_GetObjectItemCaseSensitive(json, "period");   //wave period
    if( cJSON_IsNumber(item_period) ){
        gridCfg->period = item_period->valuedouble;
        gridCfg->period = gridCfg->period * gridCfg->scale;
    }

    //clean up
    cJSON_Delete(json);
    free(json_file);

    if (boundary == 1){
        gridCfg->d_absorb = (int)(3*gridCfg->period);
    }else if (boundary ==2){
        gridCfg->d_absorb = 8;
    }
     
    gridCfg->Nx  = (400+0*200)*gridCfg->scale;
    gridCfg->Ny  = (300+0*100)*gridCfg->scale;
    gridCfg->Nz  = (200+0*150)*gridCfg->scale;
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

void antenaInit(gridConfiguration *gridCfg, beamConfiguration *beamCfg){

    // default values to be used if input parameter are not set
    beamCfg->antAngle_zx     = 0;
    beamCfg->antAngle_zy     = 0;

    beamCfg->exc_signal  = 5;//3;//4;
    beamCfg->rampUpMethod= 1;
    beamCfg->ant_x       = gridCfg->d_absorb + 8*gridCfg->period;//gridCfg.Nx/2;
    beamCfg->ant_y       = gridCfg->Ny/2;
    beamCfg->ant_z       = gridCfg->d_absorb + 4;
    // positions have to be even numbers, to ensure fields are accessed correctly
    if ((beamCfg->ant_x % 2) != 0)  ++beamCfg->ant_x;
    if ((beamCfg->ant_y % 2) != 0)  ++beamCfg->ant_y;
    if ((beamCfg->ant_z % 2) != 0)  ++beamCfg->ant_z;
    beamCfg->ant_w0x     = 2;
    beamCfg->ant_w0y     = 2;
    beamCfg->z2waist     = -(298.87)*.0;                // .2/l_0*period = -298.87

}

void gridInit(gridConfiguration *gridCfg, Grid *g){

    double (*EB_WAVE)[gridCfg->Ny][gridCfg->Nz]           = calloc(gridCfg->Nx, sizeof *EB_WAVE);


}

char *read_json(){

    FILE *file = fopen("../input_FOCAL.json", "rb");
    if (file == NULL) {
        perror("Error openning file.");
        return NULL;
    }

    fseek(file, 0, SEEK_END);
    long length = ftell(file);
    fseek(file, 0, SEEK_SET);

    char *json_data = (char *)malloc(length + 1);
    if (json_data == NULL) {
        perror("Error at allocating memory");
        fclose(file);
        return NULL;
    }

    fread(json_data, 1, length, file);
    json_data[length] = '\0';

    fclose(file);
    return json_data;

}




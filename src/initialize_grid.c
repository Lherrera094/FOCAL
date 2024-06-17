#include "initialize_grid.h"

void gridConfInit(gridConfiguration *gridCfg, namePath *pathFile, int boundary, beamConfiguration *beamCfg){

    write_JSON_onGrid( gridCfg, pathFile, beamCfg);

    //Grid configuration variables computation
    if (boundary == 1){ 
        gridCfg->d_absorb = (int)(3*gridCfg->period);
    }else if (boundary == 2){
        gridCfg->d_absorb = 8;
    }
    
    gridCfg->Nz_ref  = 2*gridCfg->d_absorb + (int)gridCfg->period;
    gridCfg->t_end   = (int)((100-50)*gridCfg->period);
    
    // dt/dx = 0.5 is commenly used in 2D FDTD codes
    // Note that period refers to the wavelength in the numerical grid and not
    // in the "physical" grid (where one grid cell is equal to one Yee cell).
    // This means that in the physical grid, the wavelength is period/2, thus
    // in the equations we have to use period/2 for the wavelength.
    gridCfg->dx  = 1./(gridCfg->period/2);
    gridCfg->dt  = 1./(2.*(gridCfg->period/2));

    //Beam Configuration variables computation
    beamCfg->ant_x       = gridCfg->d_absorb + 8*gridCfg->period;//gridCfg.Nx/2;
    beamCfg->ant_y       = gridCfg->Ny/2;
    beamCfg->ant_z       = gridCfg->d_absorb + 4;
    // positions have to be even numbers, to ensure fields are accessed correctly
    if ((beamCfg->ant_x % 2) != 0)  ++beamCfg->ant_x;
    if ((beamCfg->ant_y % 2) != 0)  ++beamCfg->ant_y;
    if ((beamCfg->ant_z % 2) != 0)  ++beamCfg->ant_z;

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

void write_JSON_onGrid(gridConfiguration *gridCfg, namePath *pathFile, beamConfiguration *beamCfg){

    /*Read JSON and extract data*/
    char *json_file = read_json();
    int scale;

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
        pathFile->projectPath = strdup(Main_Project->valuestring);
    }

    cJSON *FolderName = cJSON_GetObjectItemCaseSensitive(json, "foldername");       //Simulation folder name
    if( cJSON_IsString(FolderName) && (FolderName->valuestring != NULL) ){
        pathFile->foldername = strdup(FolderName->valuestring);
    }
    
    cJSON *Filename_HDF5 = cJSON_GetObjectItemCaseSensitive(json, "filename_hdf5");   //filename hdf5
    if( cJSON_IsString(Filename_HDF5) && (Filename_HDF5->valuestring != NULL) ){
        pathFile->file_hdf5 = strdup(Filename_HDF5->valuestring);
    }

    cJSON *Filename_TimeTrace = cJSON_GetObjectItemCaseSensitive(json, "filename_timetraces");   //filename datatraces
    if( cJSON_IsString(Filename_TimeTrace) && (Filename_TimeTrace->valuestring != NULL) ){
        pathFile->file_trace = strdup(Filename_TimeTrace->valuestring);
    }
    
    cJSON *item_scale = cJSON_GetObjectItemCaseSensitive(json, "scale");   //scale factor
    if( cJSON_IsNumber(item_scale) ){
        scale = item_scale->valueint;
    }

    /*Grid configuration Input values*/
    cJSON *item_period = cJSON_GetObjectItemCaseSensitive(json, "period");   //wave period
    if( cJSON_IsNumber(item_period) ){
        gridCfg->period = item_period->valuedouble;
        gridCfg->period = gridCfg->period * scale;
    }
    
    cJSON *item_Nx = cJSON_GetObjectItemCaseSensitive(json, "Grid_size_Nx");   //scale factor
    if( cJSON_IsNumber(item_Nx) ){
        gridCfg->Nx = item_Nx->valueint;
        gridCfg->Nx = (gridCfg->Nx + 0*200) * scale;
    }

    cJSON *item_Ny = cJSON_GetObjectItemCaseSensitive(json, "Grid_size_Ny");   //scale factor
    if( cJSON_IsNumber(item_Ny) ){
        gridCfg->Ny = item_Ny->valueint;
        gridCfg->Ny = (gridCfg->Ny + 0*100) * scale;
    }

    cJSON *item_Nz = cJSON_GetObjectItemCaseSensitive(json, "Grid_size_Nz");   //scale factor
    if( cJSON_IsNumber(item_Nz) ){
        gridCfg->Nz = item_Nz->valueint;
        gridCfg->Nz = (gridCfg->Nz + 0*150) * scale;
    }

    cJSON *item_B0 = cJSON_GetObjectItemCaseSensitive(json, "B0_profile");   //scale factor
    if( cJSON_IsNumber(item_B0) ){
        gridCfg->B0_profile = item_B0->valueint;
    }

    cJSON *item_ne = cJSON_GetObjectItemCaseSensitive(json, "ne_profile");   //scale factor
    if( cJSON_IsNumber(item_ne) ){
        gridCfg->ne_profile = item_ne->valueint;
    }


    /*Antenna Input values*/
    // default values to be used if input parameter are not set
    cJSON *item_antAngleZX = cJSON_GetObjectItemCaseSensitive(json, "Antenna_Angle_zx");   //scale factor
    if( cJSON_IsNumber(item_antAngleZX) ){
        beamCfg->antAngle_zx = item_antAngleZX->valueint;
    }

    cJSON *item_antAngleZY = cJSON_GetObjectItemCaseSensitive(json, "Antenna_Angle_zy");   //scale factor
    if( cJSON_IsNumber(item_antAngleZY) ){
        beamCfg->antAngle_zy = item_antAngleZY->valueint;
    }

    cJSON *item_exc_signal = cJSON_GetObjectItemCaseSensitive(json, "External_signal");   //scale factor
    if( cJSON_IsNumber(item_exc_signal) ){
        beamCfg->exc_signal = item_exc_signal->valueint;
    }

    cJSON *item_rampUpMethod = cJSON_GetObjectItemCaseSensitive(json, "RampUp_Method");   //scale factor
    if( cJSON_IsNumber(item_rampUpMethod) ){
        beamCfg->rampUpMethod = item_rampUpMethod->valueint;
    }

    cJSON *item_ant_w0x = cJSON_GetObjectItemCaseSensitive(json, "Antena_w0x");   //scale factor
    if( cJSON_IsNumber(item_ant_w0x) ){
        beamCfg->ant_w0x = item_ant_w0x->valuedouble;
    }

    cJSON *item_ant_w0y = cJSON_GetObjectItemCaseSensitive(json, "Antena_w0y");   //scale factor
    if( cJSON_IsNumber(item_ant_w0y) ){
        beamCfg->ant_w0y = item_ant_w0y->valuedouble;
    }

    //beamCfg->ant_w0x     = 2;
    //beamCfg->ant_w0y     = 2;
    beamCfg->z2waist     = -(298.87)*.0;                // .2/l_0*period = -298.87

    //clean up
    cJSON_Delete(json);
    free(json_file);

}





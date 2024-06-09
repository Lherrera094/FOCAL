#include "save_data.h"

void simulation_folder(const char *path){
    
    struct stat st = {0};

    /*Checks if directory exists*/
    if( stat(path, &st) == -1){
        //Directory does not exists. Create it.
        if(mkdir(path, 0700) == 0){
            printf("Simulations folder created successfully.\n");
        }else{
            printf("Error creating directory: %s\n", strerror(errno));
            return;
        }
    }else{
        printf("Directory already exists.\n");
    }
}

void data_folder(const char *path, const char *foldername){
    
    char fullPath[1024];

    // Create the full directory path and check for buffer overflow
    if (snprintf(fullPath, sizeof(fullPath), "%s/%s", path, foldername) >= sizeof(fullPath)) {
        fprintf(stderr, "Error: Directory path is too long.\n");
        return;
    }

    struct stat st = {0};

    /*Checks if directory exists.*/
    if( stat(fullPath, &st) == -1){
        //Directory does not exists. Create it.
        if( mkdir(fullPath, 0700) == 0){
            printf("%s folder created successfully. \n", foldername);
        }else{
            printf("Error creating directory: %s\n", strerror(errno));
            return;
        }
    }else{
        printf("Directory already exists.\n");
    }
}

void copyJSON(const char *path, const char *foldername){
    printf("Copy JSON");
}

void create_folder_path(gridConfiguration *gridCfg){

    simulation_folder(gridCfg->path);
    data_folder(gridCfg->path, gridCfg->foldername);
    
}


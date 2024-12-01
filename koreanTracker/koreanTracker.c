#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

#include "../KatArrayLib/katarray.h"

// structure for koreandata, the katarray will point to koreandata structs
typedef struct KoreanData{
    short watches;
    char* name;
    char* link;
} korean_data_t;


// main functions for add, rm, show, increment
void instruction_add(katarray_voidp_t *KatArray, short watched, char* name, char* link);
void instruction_rm(katarray_voidp_t *KatArray, short id);
void instruction_show();
void instruction_increment();


// will store the positions of add flags into the vars
// 0 means it was not found
void argv_set_flags_add(char **argv, short argc, char** name, char** link, short* watched);
void argv_set_flags_rm(char **argv, short argc, short* id);
void argv_set_flags_show(char **argv, short argc, short* id);
void argv_set_flags_increment(char **argv, short argc, short* id);


// katarray extentions
void katarray_free(katarray_voidp_t *KatArray);
void katarray_insert_sorted(katarray_voidp_t *KatArray, korean_data_t *new_data);

// file handling
#define KOREANDATA_FORMAT_PRINT "[%hd, %s, %s]\n" // watches, name, link
#define KOREANDATA_FORMAT_SCANF "[%hd, %35[^,], %100[^]]\n"   // watches, name, link
#define WATCHLIST_FORMAT_PRINT "{ %s | %s | %s }\n" // name, date, link

#define PATH_TO_REPLIST "./koreanTracker/rep-list.dat"
#define PATH_TO_WATCHEDLIST "./koreanTracker/watched-list.txt"

FILE* fopen_wrapper(char* file_name, char* file_instruction, const char* function_name);
void katarray_deserialize_replist(katarray_voidp_t *KatArray);
void katarray_serialize_replist(katarray_voidp_t *KatArray);
void write_log_entry_watchedlist(korean_data_t *korean_data);


// time functions
void time_str_set(char* time_str_buffer);


////////* MAIN *////////

int main(int argc, char** argv) {
    argc = (short)argc;
    //argc = 4;
    //argv[1] = strdup("rm");
    //argv[2] = strdup("-i");
    //argv[3] = strdup("1");
    


    // entrance to add, remove, show, increment, help
    if (argc > 1) {

        // initialize static katarray
        katarray_voidp_t *KatArray_KoreanData = katarray_voidp_create(0, NULL, 50, 0);

        // ADD feature
        if      (strcmp(argv[1], "add") == 0) {

            // store flags
            char* name = NULL;
            char* link = NULL;
            short watched = 1;
            argv_set_flags_add(argv, argc, &name, &link, &watched);

            // quit if no name is given
            if (name == NULL) {
                perror("you forgot to put a name dummy\n\n");
                exit(EXIT_FAILURE);
            }

            // perform ADD instruction
            instruction_add(KatArray_KoreanData, watched, name, link);

        }

        // REMOVE feature
        else if (strcmp(argv[1], "rm") == 0) {
            
            // store flags
            short id = -1;
            argv_set_flags_rm(argv, argc, &id);

            
            
            // rm the entry
            instruction_rm(KatArray_KoreanData, id);
            
        }

        // SHOW feature
        else if (strcmp(argv[1], "show") == 0) {
            
            // store flags
            short id = 0;
            argv_set_flags_show(argv, argc, &id);

            

        }

        // INCREMENT feature
        else if (strcmp(argv[1], "increment") == 0) {

            // store flags
            short id = 0;
            argv_set_flags_increment(argv, argc, &id);
        }

        else if (strcmp(argv[1], "logs") == 0) {

            // execute cat on watched-list.txt
            char *cat_arg[] = {"cat", "watched-list.txt", NULL};
            execvp("cat", cat_arg);

            // if execvp fails
            perror("execvp failed");
        }

        else {

            // HELP feature
            if (strcmp(argv[1], "-h") || strcmp(argv[1], "--help")) {
                
            }

            
        }

        katarray_free(KatArray_KoreanData);
        return 0;
    }

    // error occured

    printf("ERROR");

    return 1;
}


////* add, rm, show, increment instructions *////

// add instruction
void instruction_add(katarray_voidp_t *KatArray, short watched, char* name, char* link) {
    
    // deserialize rep-list.dat to katarray
    katarray_deserialize_replist(KatArray);

    // if link not given, default it
    if (!link) {
        link = strdup("no_string");
    }

    // create new korean_data object
    korean_data_t *korean_data_ptr = malloc_wrapper(sizeof(*korean_data_ptr), __func__);
    korean_data_ptr->watches = watched;
    korean_data_ptr->name    = strdup(name);
    korean_data_ptr->link    = strdup(link);

    // put the data sorted with watches in katarray
    katarray_insert_sorted(KatArray, korean_data_ptr);

    // write to watched-list.txt
    write_log_entry_watchedlist(korean_data_ptr);

    // serialize object
    katarray_serialize_replist(KatArray);

    return;
}

// rm instruction
void instruction_rm(katarray_voidp_t *KatArray, short id) {
    
    // deserialize rep-list.dat to katarray
    katarray_deserialize_replist(KatArray);

    // quit if no id is given
    if (id < 0 || id >= (short)KatArray->length) {
        perror("you forgot to give me the id smh..\nor you're trying to pull some tricks with those out of bounders!\n\n");
        exit(EXIT_FAILURE);
    }

    // free pointer
    korean_data_t *rm_data = katarray_voidp_get_value_at(KatArray, id);
    if (rm_data) {
        free(rm_data->name);
        free(rm_data->link);
        free(rm_data);
    }
    

    // remove from katarray pointer from katarray
    katarray_voidp_remove_overwrite_at(&KatArray, id);


    // serialize object
    katarray_serialize_replist(KatArray);

    return;
}

////* command line flag position storer *////

// will store the positions of add flags into the vars
// 0 means it was not found
void argv_set_flags_add(char **argv, short argc, char** name, char** link, short* watched) {
    for (short i = 2; i < argc; i++) {

        // set flags
        if (((strcmp(argv[i], "--name")    == 0) || (strcmp(argv[i], "-n") == 0)) && (i + 1) < argc) *name    = argv[i+1];
        if (((strcmp(argv[i], "--link")    == 0) || (strcmp(argv[i], "-l") == 0)) && (i + 1) < argc) *link    = argv[i+1];
        if (((strcmp(argv[i], "--watches") == 0) || (strcmp(argv[i], "-w") == 0)) && (i + 1) < argc) *watched = (short)(atoi(argv[i+1]));
    }
}

// will store the positions of add flags into the vars
// 0 means it was not found
void argv_set_flags_rm(char **argv, short argc, short* id) {
    for (short i = 2; i < argc; i++) {

        // set flags
        if (((strcmp(argv[i], "--id") == 0) || (strcmp(argv[i], "-i") == 0)) && (i + 1) < argc) *id = (short)(atoi(argv[i+1]));
    }
}

// will store the positions of add flags into the vars
// 0 means it was not found
void argv_set_flags_show(char **argv, short argc, short* id) {
    for (short i = 2; i < argc; i++) {

        // set flags
        if (((strcmp(argv[i], "--id") == 0) || (strcmp(argv[i], "-i") == 0)) && (i + 1) < argc) *id = (short)(atoi(argv[i+1]));
    }
}

// will store the positions of add flags into the vars
// 0 means it was not found
void argv_set_flags_increment(char **argv, short argc, short* id) {
    for (short i = 2; i < argc; i++) {

        // set flags
        if (((strcmp(argv[i], "--id") == 0) || (strcmp(argv[i], "-i") == 0)) && (i + 1) < argc) *id = (short)(atoi(argv[i+1]));
    }
}



void katarray_free(katarray_voidp_t *KatArray) {
    
    // free pointers
    for (short i = 0; i < (short)KatArray->length; i++) {

        // get ptr
        korean_data_t *korean_data_ptr = katarray_voidp_get_value_at(KatArray, i);

        // quit if null
        if (!korean_data_ptr) continue;

        // free up
        free(korean_data_ptr->name);
        free(korean_data_ptr->link);
        free(korean_data_ptr);
    }

    // free the whole KatArray
    katarray_voidp_free(KatArray);

    return;
}


////* FILE HANDLING *////

// fopen_wrapper
FILE* fopen_wrapper(char* file_name, char* file_instruction, const char* function_name) {
    FILE *file = fopen(file_name, file_instruction);
    if (!file) {
        fprintf(stderr, "[ERROR] %s: (%s)\n", __func__, function_name);
        exit(EXIT_FAILURE);
    }
    return file;
}

// deserialization into katarray
void katarray_deserialize_replist(katarray_voidp_t *KatArray) {
    
    // open rep-list.dat
    FILE* file = fopen_wrapper("./koreanTracker/rep-list.dat", "r", __func__);

    // stop when end of file
    while (!feof(file)) {
        
        // allocate korean_data
        korean_data_t *korean_data_ptr = malloc_wrapper(sizeof(*korean_data_ptr), __func__);
        korean_data_ptr->name = malloc_wrapper(35, __func__);
        korean_data_ptr->link = malloc_wrapper(100, __func__);

        // save file's objects into korean_data_struct
        if (fscanf(file, KOREANDATA_FORMAT_SCANF, &(korean_data_ptr->watches), korean_data_ptr->name, korean_data_ptr->link) == EOF) {
            free(korean_data_ptr->name);
            free(korean_data_ptr->link);
            free(korean_data_ptr);
            //perror("error [func: katarray_deserialize_replist()] fscanf, could not print koreandata_format to replist");
            break;
        }

        // consume trailing chars
        char c;
        while ((c = fgetc(file)) != '\n' && c != EOF);

        // save to katarray
        katarray_voidp_push(&KatArray, korean_data_ptr);
    }

    fclose(file);

    return;
}

// serialization from katarray into rep-list.dat
void katarray_serialize_replist(katarray_voidp_t *KatArray) {
    
    // open rep-list.dat
    FILE* file = fopen_wrapper("./koreanTracker/rep-list.dat", "w", __func__);

    for (short i = 0; i < (short)KatArray->length; i++) {
        
        // get current pointer from katarray
        korean_data_t *korean_data_ptr = katarray_voidp_get_value_at(KatArray, i);
        if (!korean_data_ptr) continue; // will skip the loop to next iteration

        // save objects from katarray into rep-list.dat
        fprintf(file, KOREANDATA_FORMAT_PRINT, korean_data_ptr->watches, korean_data_ptr->name, korean_data_ptr->link);
    } 

    fclose(file);

    return;
}

// write entry of new korean_data into watched-list.txt
void write_log_entry_watchedlist(korean_data_t *korean_data) {
    
    // open watched-list.txt
    FILE* file = fopen_wrapper("./koreanTracker/watched-list.txt", "a", __func__);

    // get current time into string
    #define TIME_STR_SIZE 11
    char time_str_buffer[TIME_STR_SIZE];
    time_str_set(time_str_buffer);

    // write log to file
    fprintf(file, WATCHLIST_FORMAT_PRINT, korean_data->name, time_str_buffer, korean_data->link);

    fclose(file);

    return;
}

// sort katarray
void katarray_insert_sorted(katarray_voidp_t *KatArray, korean_data_t *new_data) {

    if (KatArray->length == 0 || new_data->watches == 1) {
        katarray_voidp_set_append(&KatArray, new_data);
        return;
    }

    for (short i = 0; i < (short)KatArray->length; i++) {
        
        // get old_data for i
        korean_data_t *old_data = katarray_voidp_get_value_at(KatArray, i);

        if (old_data->watches == new_data->watches) {
            katarray_voidp_set_insert_at(&KatArray, i, new_data);
            return;
        }
        else if (old_data->watches < new_data->watches) {
            katarray_voidp_set_insert_at(&KatArray, i, new_data);
            return;
        }
    }

    katarray_voidp_set_append(&KatArray, new_data);
    return;
}


////* time functions *////

// get current time
void time_str_set(char* time_str_buffer) {

    // Get the current time
    time_t now = time(NULL);
    if (now == -1) {
        perror("Error getting current time");
        return;
    }

    // Convert to local time
    struct tm *local = localtime(&now);
    if (!local) {
        perror("Error converting time to local");
        return;
    }

    // Format the time
    if (strftime(time_str_buffer, TIME_STR_SIZE, "%Y-%m-%d", local) == 0) {
        fprintf(stderr, "Error formatting time\n");
        return;
    }

    return;
}


#include <stddef.h>

#include "plist.h"

static struct plist process_list[61]; 

void plist_init(){
  int i = 0;
  int list_size = sizeof(process_list)/sizeof(process_list[0]);

  for(i; i < list_size; ++i){
    process_list[i].used = false;
    sema_init(&process_list[i].semaphore,0);
   }

  //printf("leaving plist_init;");
}

void plist_print(){
  
  int i = 0;
  int list_size = sizeof(process_list)/sizeof(process_list[0]);
  //printf("\n\n### PROCESS LIST###\n\n");
  for(i; i < list_size ; ++i){
    if(process_list[i].used && process_list[i].name != NULL){
    printf("INDEX: %3d PID: %3d NAME: %15s PARENT: %3d EXIT_STATUS: %3d USED: %1d ALIVE: %1d\n",i , process_list[i].pid, 
    process_list[i].name, process_list[i].parent, process_list[i].exit_status, process_list[i].used, process_list[i].alive);
    }
  }
   //printf("\n\n### END PROCESS LIST###\n\n");
}

int plist_insert(int id, char* name, int parent_id){

  int i = 0;
  int list_size = sizeof(process_list)/sizeof(process_list[0]);
  
 //printf("# \n\npname: %s \n\n", process_list[i].name);
  for(i; i < list_size; ++i){
 
    if(!process_list[i].used){
        process_list[i].name = malloc(strlen(name)+1);
	    strlcpy(process_list[i].name,name,strlen(name)+1);
        process_list[i].pid = id;
	    process_list[i].parent = parent_id;
	    process_list[i].alive = true;
	    process_list[i].parent_alive = true;
	    process_list[i].used = true;	
	    process_list[i].waiting = false;
	    process_list[i].exit_status = -3;
	    sema_init(&process_list[i].semaphore,0);
	    //printf("\nplist_insert index: %d, pid: %d \n", i, id);
	return id;
    }
  }
//plist_print();
//printf("\n\n############### INSERT FAILED, ID: %d ###############################\n\n",id);

return -1;
}

struct plist* plist_find(int id){
  
  int i = 0;
  int list_size = sizeof(process_list)/sizeof(process_list[0]);

  for(i; i < list_size; ++i){
    if(process_list[i].pid == id){
	//printf("\nplist_find process number: %d\n", id);
        return &process_list[i];
    }
  }
//printf("\nplist_find process: NULL, id not found:%d\n",id);
return NULL;
}

void plist_remove_zombies(int parent_id){

  int i = 0;
  int list_size = sizeof(process_list)/sizeof(process_list[0]);
  //printf("\n\n REMOVE ZOMBIES ENTERED\n");
  for(i; i < list_size; ++i){
    if(process_list[i].parent == parent_id && !process_list[i].alive){
        free(process_list[i].name);
	    process_list[i].used = false; 
    }
  }
   // printf("\n\nEND REMOVE ZOMBIES \n");
}
int plist_remove(int id){
  
  int i = 0;
  int exit_status;
  int list_size = sizeof(process_list)/sizeof(process_list[0]);
  
  //printf("\nplist_remove id: %d\n",id);
  for(i; i < list_size; ++i){ 
    if(process_list[i].pid == id && process_list[i].used){
	    //plist_print();
        exit_status = process_list[i].exit_status;
        if(process_list[i].name != NULL){
            free(process_list[i].name);
        }
	    process_list[i].used = false; 
	    //printf("\nplist_remove succeded\n");
	    return exit_status;
    }
  }
  //printf("\nplist_remove failed\n");
return -1;
}

#include <stddef.h>
#include "plist.h"
#include "threads/malloc.h"

static struct plist process_list[256]; 

static struct lock plist_lock;

void plist_init(void){
  int i = 0;
  int list_size = sizeof(process_list)/sizeof(process_list[0]);
  lock_init(&plist_lock);
  for(i; i < list_size; ++i){
    process_list[i].used = false;
    sema_init(&process_list[i].semaphore,0);
  }
}

void plist_print(void){
  
  int i = 0;
  int list_size = sizeof(process_list)/sizeof(process_list[0]);
  lock_acquire(&plist_lock);
  for(i; i < list_size ; ++i){
    if(//process_list[i].used &&
 process_list[i].name != NULL){
      printf("INDEX: %3d PID: %3d NAME: %15s PARENT: %3d EXIT_STATUS: %3d USED: %1d ALIVE: %1d PARENT: %1d\n",i , process_list[i].pid, 
	     process_list[i].name, process_list[i].parent, process_list[i].exit_status, process_list[i].used, process_list[i].alive, process_list[i].parent);
    }
  }
  lock_release(&plist_lock);
}

int plist_insert(int id, char* name, int parent_id){

  int i = 0;
  int list_size = sizeof(process_list)/sizeof(process_list[0]);
  
  lock_acquire(&plist_lock);
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
      lock_release(&plist_lock);
      return id;
    }
  }
  lock_release(&plist_lock);
  return -1;
}

struct plist* plist_find(int id){
  
  int i = 0;
  int list_size = sizeof(process_list)/sizeof(process_list[0]);
  lock_acquire(&plist_lock);

  for(i; i < list_size; ++i){
    if(process_list[i].pid == id){
      lock_release(&plist_lock);
      return &process_list[i];
    }
  }
  lock_release(&plist_lock);
  return NULL;
}

void plist_remove_zombies(int parent_id){

  int i = 0;
  int list_size = sizeof(process_list)/sizeof(process_list[0]);
  lock_acquire(&plist_lock);
  for(i; i < list_size; ++i){
    if(process_list[i].parent == parent_id && !process_list[i].alive){
      free(process_list[i].name);
      process_list[i].name = NULL;
      process_list[i].used = false; 
    }
  }
  lock_release(&plist_lock);
}
int plist_remove(int id){
  
  int i = 0;
  int exit_status;
  int list_size = sizeof(process_list)/sizeof(process_list[0]);
  
  lock_acquire(&plist_lock);

  for(i; i < list_size; ++i){ 
    if(process_list[i].pid == id && process_list[i].used){

      exit_status = process_list[i].exit_status;
      if(process_list[i].name != NULL){
	free(process_list[i].name);
	process_list[i].name = NULL;
      }
      process_list[i].used = false; 
      lock_release(&plist_lock);
      return exit_status;
    }
  }
  lock_release(&plist_lock);
  return -1;
}

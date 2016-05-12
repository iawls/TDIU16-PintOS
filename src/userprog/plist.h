#ifndef _PLIST_H_
#define _PLIST_H_
#include "threads/synch.h"
#include <stdbool.h>


/* Place functions to handle a running process here (process list).
   
   plist.h : Your function declarations and documentation.
   plist.c : Your implementation.

   The following is strongly recommended:

   - A function that given process inforamtion (up to you to create)
     inserts this in a list of running processes and return an integer
     that can be used to find the information later on.

   - A function that given an integer (obtained from above function)
     FIND the process information in the list. Should return some
     failure code if no process matching the integer is in the list.
     Or, optionally, several functions to access any information of a
     particular process that you currently need.

   - A function that given an integer REMOVE the process information
     from the list. Should only remove the information when no process
     or thread need it anymore, but must guarantee it is always
     removed EVENTUALLY.
     
   - A function that print the entire content of the list in a nice,
     clean, readable format.
     
 */
  
 struct plist{
    char* name;
    int pid;
    int parent;
    int exit_status;
    bool alive;
    bool parent_alive;
    bool used; 
    bool waiting;
    struct semaphore semaphore;
  };


  
  void plist_init(void);

  void plist_print(void);

  int plist_insert(int id, char* name, int parent_id);
  
  struct plist* plist_find(int id);

  int plist_remove(int id);


#endif

#include <stdio.h>
#include <syscall-nr.h>
#include "userprog/syscall.h"
#include "threads/interrupt.h"
#include "threads/thread.h"
#include "threads/init.h"
#include "plist.h"


/* header files you probably need, they are not used yet */
#include <string.h>
#include "filesys/filesys.h"
#include "filesys/file.h"
#include "threads/vaddr.h"
#include "userprog/pagedir.h"
#include "userprog/process.h"
#include "devices/input.h"
#include "devices/timer.h"
#include "flist.h"

static void syscall_handler (struct intr_frame *);

bool verify_fix_length(void* start, int length);
bool verify_variable_length(char* start);

void
syscall_init (void) 
{
  intr_register_int (0x30, 3, INTR_ON, syscall_handler, "syscall");
}


/* This array defined the number of arguments each syscall expects.
   For example, if you want to find out the number of arguments for
   the read system call you shall write:
   
   int sys_read_arg_count = argc[ SYS_READ ];
   
   All system calls have a name such as SYS_READ defined as an enum
   type, see `lib/syscall-nr.h'. Use them instead of numbers.
 */
const int argc[] = {
  /* basic calls */
  0, 1, 1, 1, 2, 1, 1, 1, 3, 3, 2, 1, 1, 
  /* not implemented */
  2, 1,    1, 1, 2, 1, 1,
  /* extended */
  0
};

void sys_read(struct intr_frame *f)
{
  int32_t* esp = (int32_t*)f->esp;
  unsigned i = 0;
  char c;
  int fd = esp[1];
  unsigned length = esp[3]; 
  char* buf = (char*)esp[2];
  struct thread* t = thread_current();
  struct file* tmp_file;

  if(verify_fix_length( (void*)buf ,length) ){
    if(fd == STDIN_FILENO){		  
      for(i; i < length; ++i){
	c = input_getc();
	if(c == '\r')
	  c = '\n';
	buf[i] = c;
	putbuf(&c,1);
      }
      f->eax = i; 
    }
    else if(2 <= fd && fd <= 16){
      tmp_file = map_find(&(t->filetable),fd);
      if(tmp_file == NULL){
	f->eax = -1;
      }
      else
	f->eax = file_read(tmp_file, buf, length);
    }
  }
  else
    process_exit(-1);
  // f->eax= -1;
};

void sys_write(struct intr_frame *f)
{
  int32_t* esp = (int32_t*)f->esp;
  struct thread* t = thread_current();
  int fd = esp[1];
  char* buf = (char*)esp[2];
  unsigned length = esp[3]; 

  if(verify_fix_length( (void*)buf ,length) && verify_variable_length(buf) ){
    if(fd == STDOUT_FILENO){		  
      putbuf(buf,length);
      f->eax = length;
    }
    else if(2 <= fd && fd <= 16){
      struct file* file = map_find(&(t->filetable), fd);
      if(file != NULL)
	f->eax = file_write (file, buf, length);
      else
	f->eax = -1;
    }
	
  }
  else
    process_exit(-1);

    // f->eax = -1;
  };

void sys_remove(struct intr_frame *f)
{
  int32_t* esp = (int32_t*)f->esp;
  char* file_name = esp[1];

  bool success = filesys_remove(file_name);
  f->eax = success;   
};

void sys_open(struct intr_frame *f)
{
  int32_t* esp = (int32_t*)f->esp;
  char* file_name = esp[1];
  struct thread* t = thread_current();

   if(!verify_variable_length(file_name) || file_name == NULL){
    process_exit(-1);
    return;
    }

  struct file* filep = filesys_open(file_name);
  
  if(filep == NULL)
    f->eax = -1;
  else
    f->eax = map_insert(&(t->filetable),filep);
};

void sys_create(struct intr_frame *f)
{
  int32_t* esp = (int32_t*)f->esp;
  char* file_name = esp[1];
  unsigned file_size = esp[2];
  if(!verify_variable_length(file_name) || file_name == NULL)
    process_exit(-1);

  bool success = filesys_create(file_name, file_size);
  f->eax = success;
};

void sys_close(struct intr_frame *f)
{
  int32_t* esp = (int32_t*)f->esp;
  int fd = esp[1];
  struct thread* t = thread_current();

  file_close(map_find(&(t->filetable),fd));
  map_remove(&(t->filetable),fd);
};

void sys_seek(struct intr_frame *f)
{
  int32_t* esp = (int32_t*)f->esp;
  int fd = esp[1];
  unsigned pos = esp[2];
  struct thread* t = thread_current();

  if(map_find(&(t->filetable),fd) != NULL){

    if(pos <= file_length(map_find(&(t->filetable),fd))){
      file_seek(map_find(&(t->filetable),fd),pos);
    }
    
  }
}

void sys_tell(struct intr_frame* f)
{
  int32_t* esp = (int32_t*)f->esp;
  int fd = esp[1];
  struct thread* t = thread_current();

  if(map_find(&(t->filetable),fd) != NULL){
    f->eax = file_tell(map_find(&(t->filetable),fd));
  }
  else
    f->eax = -1;
};

void sys_filesize(struct intr_frame* f)
{
  int32_t* esp = (int32_t*)f->esp;
  int fd = esp[1];
  struct thread* t = thread_current();

  if(map_find(&(t->filetable),fd) != NULL)
    f->eax = file_length(map_find(&(t->filetable),fd));
  else
    f->eax = -1;
};

void sys_sleep(int millis)
{
  timer_msleep(millis);
}



static void
syscall_handler (struct intr_frame *f) 
{
  int32_t* esp = (int32_t*)f->esp;
  
  if(esp >= PHYS_BASE-4 || 0 > esp || f == NULL || esp == NULL || !verify_fix_length(esp,4)) 
    process_exit(-1);

  

  switch (esp[0]) //get retrive call from esp 
  {
    case SYS_HALT : 
	power_off();
	break;
    
    case SYS_EXIT : 
	//printf("\n\nexit status: %d\n\n", esp[1]);
	process_exit(esp[1]);
	break;
	
    case SYS_READ :
	sys_read(f);
	break;

    case SYS_WRITE : 
	sys_write(f);
	break;	
	
    case SYS_OPEN : 
	sys_open(f);
	break;
 
    case SYS_CREATE : 
	sys_create(f);
	break;

    case SYS_CLOSE : 
	sys_close(f);
	break;
   
    case SYS_REMOVE : 
	sys_remove(f);
	break;

    case SYS_SEEK : 
	sys_seek(f);
	break;
    
    case SYS_TELL : 
	sys_tell(f);
	break;

    case SYS_FILESIZE : 
	sys_filesize(f);
	break;
    
    case SYS_PLIST :
	plist_print();
	break;

    case SYS_EXEC : 
	f->eax = process_execute(esp[1]);
	break;

    case SYS_SLEEP :
	sys_sleep(esp[1]);
	break;
	
	case SYS_WAIT :
	f->eax = process_wait(esp[1]);
	break;

    default:
    {
      printf ("\n\nExecuted an unknown system call!\n");
      printf ("\n\nStack top + 0: %d\n", esp[0]);
      printf ("\n\nStack top + 1: %d\n\n\n", esp[1]);
      
      thread_exit ();
    }
  }
}

bool verify_fix_length(void* start, int length)
{ 
  if(start >= PHYS_BASE)
    return false;

  if(pagedir_get_page(thread_current()->pagedir, start) == NULL )
    return false;

unsigned current_pg,last_pg,i = 0;
last_pg = pg_no(start);
  
for(i; i < length ; ++i)
  {
    current_pg = pg_no(start+i); 
    if(current_pg != last_pg){
      last_pg = current_pg;
      if(pagedir_get_page(thread_current()->pagedir, start+i) == NULL)
	return false;
    }
  }  
 return true;
}

/* Kontrollera alla adresser från och med start till och med den
 * adress som först innehåller ett noll-tecken, `\0'. (C-strängar
 * lagras på detta sätt.) */
bool verify_variable_length(char* start)
{
  char* adr = start;
  unsigned current_pg, last_pg;

  if(start >= PHYS_BASE)
    return false;
  if(pagedir_get_page(thread_current()->pagedir,(void*)start) == NULL )
    return false;

  last_pg = pg_no(start);

  while(true)
    {
      current_pg = pg_no(adr);
      if(last_pg != current_pg){
	last_pg = current_pg;
	if(pagedir_get_page(thread_current()->pagedir,(void*)adr) == NULL)
	  return false;
      }

      if(*adr == '\0')
	return true;
    
      ++adr;
    }
}

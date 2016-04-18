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
#include "flist.h"

static void syscall_handler (struct intr_frame *);

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
  else
    f->eax= -1;
};

void sys_write(struct intr_frame *f)
{
  int32_t* esp = (int32_t*)f->esp;
  struct thread* t = thread_current();

  int fd = esp[1];
  char* buf = (char*)esp[2];
  unsigned length = esp[3]; 

   if(fd == STDOUT_FILENO){		  
     putbuf(buf,length);
     f->eax = length;
  }
  else if(2 <= fd && fd <= 16)
    f->eax = file_write (map_find(&(t->filetable),fd), buf, length);
  else
    f->eax = -1;
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

static void
syscall_handler (struct intr_frame *f) 
{
  int32_t* esp = (int32_t*)f->esp;
  switch (esp[0]) //get retrive call from esp 
  {
    case SYS_HALT : 
	power_off();
	break;
    
    case SYS_EXIT : 
	printf("\n\nexit status: %d\n\n", esp[1]);
	thread_exit();
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

    default:
    {
      printf ("\n\nExecuted an unknown system call!\n");
      printf ("\n\nStack top + 0: %d\n", esp[0]);
      printf ("\n\nStack top + 1: %d\n\n\n", esp[1]);
      
      thread_exit ();
    }
  }
}
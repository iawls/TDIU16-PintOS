#include <stdlib.h>
#include "pagedir.h"
#include "thread.h"
#include "ctype.h"


/* verfy_*_lenght �r t�nkta att anv�ndas i systemanrop som f�r in
 * op�litliga adresser fr�n user mode. Operativsystemet ska inte kunna
 * luras att � processens v�gnar anv�nda adresser processen inte har
 * tillg�ng till. I pagedir.h finns funktioner som kan hj�lpa dig sl�
 * upp adresser i pagetable. Fundra hur du kan g�ra s� f� slagningar
 * som m�jligt.
 *
 * Rekommenderat kompileringskommando:
 *
 *  gcc -Wall -Wextra -std=gnu99 -pedantic -g pagedir.o verify_adr.c
 */

/* Kontrollera alla adresser fr�n och med start till och inte med
 * (start+length). */
bool verify_fix_length(void* start, int length)
{ 

  if(pagedir_get_page(thread_current()->pagedir, start) == NULL)
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

/* Kontrollera alla adresser fr�n och med start till och med den
 * adress som f�rst inneh�ller ett noll-tecken, `\0'. (C-str�ngar
 * lagras p� detta s�tt.) */
bool verify_variable_length(char* start)
{
  // ADD YOUR CODE HERE
  char* adr = start;
  unsigned current_pg, last_pg;

  if(pagedir_get_page(thread_current()->pagedir,(void*)start) == NULL)
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

      if(is_end_of_string(adr))
	return true;
    
      ++adr;
    }
}


/* Definition av testfall */
struct test_case_t
{
  void* start;
  unsigned length;
};

#define TEST_CASE_COUNT 6

const struct test_case_t test_case[TEST_CASE_COUNT] =
{
  {(void*)100, 100}, /* one full page */
  {(void*)199, 102},
  {(void*)101, 98},
  {(void*)250, 190},
  {(void*)250, 200},
  {(void*)250, 210}
};

/* Huvudprogrammet utv�rderar din l�sning. */
int main(int argc, char* argv[])
{
  int i;
  bool result;

  if ( argc == 2 )
  {
    simulator_set_pagefault_time( atoi(argv[1]) );
  }
  thread_init();
  
  /* Testa algoritmen med ett givet intervall (en buffer). */
  for (i = 0; i < TEST_CASE_COUNT; ++i)
  {
    start_evaluate_algorithm(test_case[i].start, test_case[i].length);
    result = verify_fix_length(test_case[i].start, test_case[i].length);
    evaluate(result);
    end_evaluate_algorithm();
  }
    
  /* Testa algoritmen med en str�ng. */
  for (i = 0; i < TEST_CASE_COUNT; ++i)
  {
    start_evaluate_algorithm(test_case[i].start, test_case[i].length);
    result = verify_variable_length(test_case[i].start);
    evaluate(result);    
    end_evaluate_algorithm();
  }
  return 0;
}

/* Tests the halt system call. */

#include "tests/lib.h"
#include "tests/main.h"

void
test_main (void) 
{
  printf("\n\nSTART HAL TEST\n\n");
  halt ();
  fail ("should have halted");
}

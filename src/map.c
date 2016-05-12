#include "map.h"

void map_init(struct map* m)
{
  int i = 0;
  for(i; i < MAP_SIZE; ++i)
  {
    m->content[i] = NULL;
  }
};

key_t map_insert(struct map* m, value_t v)
{
  int i = 0;
  for(i; i < MAP_SIZE; ++i)
  {
    if(m->content[i] == NULL)
       {
	m->content[i] = v;
	return i;
       }
  } 
  return -1;
};

value_t map_find(struct map* m, key_t k)
{
  if(k <=128 && 0 <= k)
    return m->content[k];
  else
    return NULL;
};

value_t map_remove(struct map* m, key_t k)
{
  if(k <=128 && 0 <= k)
  {
    value_t tmp = m->content[k];
    m->content[k] = NULL;
    return tmp;
  }
  else
    return NULL;
};

void map_for_each(struct map* m,void (*exec)(key_t k, value_t v, int aux),int aux)
{
  int i = 0;
  for(i; i < MAP_SIZE; ++i)
  {
    if(m->content[i] != NULL)
       {
	exec(i,m->content[i],aux);
       }
  } 
};

void map_remove_if(struct map* m, bool (*cond)(key_t k, value_t v, int aux),int aux)
{
  int i = 0;
  for(i; i < MAP_SIZE; ++i)
  {

    if(m->content[i] != NULL)
       {
	if(cond(i,m->content[i],aux))
	{
	  m->content[i] = NULL;
	}
       }
  }   
};











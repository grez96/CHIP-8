#ifndef UTILITY_H
#define UTILITY_H


#include <stddef.h>


void exit_log(const char *file_name, int msg_count, ...);


typedef struct Map_t* Map;

Map map_create(size_t size, ...);

void map_add(Map map, int key, int value);

int map_get(Map map, int key);

const int* map_get_keys(Map map);

const int* map_get_values(Map map);

size_t map_get_size(Map map);

void map_set(Map map, int key, int value);

void map_destroy(Map map);


#endif

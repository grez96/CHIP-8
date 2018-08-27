#include "utility.h"

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>


#define FNAME "utility.c"


void exit_log(const char *file_name, int msg_count, ...)
{
	va_list ap;
	va_start(ap, msg_count);

	fprintf(stderr, "%s ERROR:\n", file_name);
	for (int i = 0; i < msg_count; ++i)
		fprintf(stderr, "%s\n", va_arg(ap, const char*));

	va_end(ap);
	exit(1);
}


struct Map_t {
	size_t size;
	int *keys;
	int *values;
};

Map map_create(size_t size, ...)
{
	Map map = (Map)malloc(sizeof(struct Map_t));
	if (!map)
		exit_log(FNAME, 1, "Failed to create map, memory allocation fail.");

	map->size = size / 2;

	map->keys = (int*)malloc(sizeof(int) * map->size);
	if(!map->keys)
		exit_log(FNAME, 1, "Failed to create map, memory allocation fail.");
	map->values = (int*)malloc(sizeof(int) * map->size);
	if(!map->values)
		exit_log(FNAME, 1, "Failed to create map, memory allocation fail.");

	va_list ap;
	va_start(ap, size);
	for (size_t i = 0; i < map->size; ++i) {
		map->keys[i] = va_arg(ap, int);
		map->values[i] = va_arg(ap, int);
	}
	va_end(ap);

	return map;
}

void map_add(Map map, int key, int value)
{
	int *keys_tmp = (int*)malloc(sizeof(int) * (map->size + 1));
	if (!keys_tmp)
		exit_log(FNAME, 1, "Failed to add to map, memory allocation fail.");
	int *values_tmp = (int*)malloc(sizeof(int) * (map->size + 1));
	if (!values_tmp)
		exit_log(FNAME, 1, "Failed to add to map, memory allocation fail.");

	for (size_t i = 0; i < map->size; ++i) {
		keys_tmp[i] = map->keys[i];
		values_tmp[i] = map->values[i];
	}
	keys_tmp[map->size] = key;
	values_tmp[map->size] = value;
	++map->size;

	free(map->keys);
	free(map->values);

	map->keys = keys_tmp;
	map->values = values_tmp;
}

int map_get(Map map, int key)
{
	for (size_t i = 0; i < map->size; ++i)
		if (map->keys[i] == key)
			return map->values[i];

	char key_str[32];
	sprintf(key_str, "\tkey: %d", key);
	exit_log(FNAME, 2, "Failed to get value in map, key not found.", key_str);
}

const int* map_get_keys(Map map)
{
	return map->keys;
}

const int* map_get_values(Map map)
{
	return map->values;
}

size_t map_get_size(Map map)
{
	return map->size;
}

void map_set(Map map, int key, int value)
{
	for (size_t i = 0; i < map->size; ++i)
		if (map->keys[i] == key) {
			map->values[i] = value;
			return ;
		}

	char key_str[32];
	sprintf(key_str, "\tkey: %d", key);
	exit_log(FNAME, 2, "Failed to set value in map, key not found.", key_str);
}

void map_destroy(Map map)
{
	free(map->keys);
	free(map->values);
	free(map);
}

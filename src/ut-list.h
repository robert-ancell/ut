#include <stdarg.h>
#include <stdbool.h>
#include <stddef.h>

#include "ut-object.h"

#pragma once

typedef struct {
  bool is_mutable;
  size_t (*get_length)(UtObject *object);
  UtObject *(*get_element)(UtObject *object, size_t index);
  UtObject *(*get_sublist)(UtObject *object, size_t index, size_t count);
  UtObject *(*copy)(UtObject *object);
  void (*insert)(UtObject *object, size_t index, UtObject *item);
  void (*remove)(UtObject *object, size_t start, size_t count);
  void (*resize)(UtObject *object, size_t length);
} UtListInterface;

extern int ut_list_id;

UtObject *ut_list_new();

UtObject *ut_list_new_with_data(UtObject *item0, ...);

UtObject *ut_list_new_with_data_take(UtObject *item0, ...);

size_t ut_list_get_length(UtObject *object);

// Returns a reference.
UtObject *ut_list_get_element(UtObject *object, size_t index);

// Returns a reference.
UtObject *ut_list_get_first(UtObject *object);

// Returns a reference.
UtObject *ut_list_get_last(UtObject *object);

UtObject *ut_list_get_sublist(UtObject *object, size_t start, size_t count);

// If was previously immutable, copy will be mutable
// Returns a reference.
UtObject *ut_list_copy(UtObject *object);

bool ut_list_is_mutable(UtObject *object);

void ut_list_append(UtObject *object, UtObject *item);

void ut_list_append_take(UtObject *object, UtObject *item);

void ut_list_append_list(UtObject *object, UtObject *list);

void ut_list_prepend(UtObject *object, UtObject *item);

void ut_list_prepend_take(UtObject *object, UtObject *item);

void ut_list_prepend_list(UtObject *object, UtObject *list);

void ut_list_insert(UtObject *object, size_t index, UtObject *item);

void ut_list_insert_take(UtObject *object, size_t index, UtObject *item);

void ut_list_insert_list(UtObject *object, size_t index, UtObject *list);

void ut_list_remove(UtObject *object, size_t index, size_t count);

void ut_list_clear(UtObject *object);

void ut_list_resize(UtObject *object, size_t length);

char *ut_list_to_string(UtObject *object);

bool ut_object_implements_list(UtObject *object);

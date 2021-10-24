#include <stdbool.h>
#include <stdint.h>

#include "ut-object.h"

#pragma once

typedef struct {
  bool is_mutable;
  const char *(*get_text)(UtObject *object);
  char *(*take_text)(UtObject *object);
  UtObject *(*get_utf8)(UtObject *object);
  void (*clear)(UtObject *object);
  void (*prepend)(UtObject *object, const char *text);
  void (*prepend_code_point)(UtObject *object, uint32_t code_point);
  void (*append)(UtObject *object, const char *text);
  void (*append_code_point)(UtObject *object, uint32_t code_point);
} UtStringInterface;

extern int ut_string_id;

UtObject *ut_string_new(const char *text);

UtObject *ut_string_new_constant(const char *text);

UtObject *ut_string_new_sized(const char *text, size_t length);

const char *ut_string_get_text(UtObject *object);

char *ut_string_take_text(UtObject *object);

// UtObject *ut_string_get_code_points(UtObject *object);

// Returns a reference.
UtObject *ut_string_get_utf8(UtObject *object);

// UtObject *ut_string_get_utf16(UtObject *object);

bool ut_string_is_mutable(UtObject *object);

void ut_string_clear(UtObject *object);

void ut_string_prepend(UtObject *object, const char *text);

void ut_string_prepend_code_point(UtObject *object, uint32_t code_point);

void ut_string_append(UtObject *object, const char *text);

void ut_string_append_code_point(UtObject *object, uint32_t code_point);

char *ut_string_to_string(UtObject *object);

bool ut_string_equal(UtObject *object, UtObject *other);

int ut_string_hash(UtObject *object);

bool ut_object_implements_string(UtObject *object);

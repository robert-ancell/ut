#pragma once

typedef struct _UtObject UtObject;

const char *ut_object_get_type_name(UtObject *object);

char *ut_object_to_string(UtObject *object);

bool ut_object_equal(UtObject *object, UtObject *other);

int ut_object_get_hash(UtObject *object);

UtObject *ut_object_ref(UtObject *object);

void ut_object_unref(UtObject *object);

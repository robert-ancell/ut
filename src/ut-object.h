typedef struct _UtObject UtObject;

typedef struct {
  void (*init)(UtObject *self);
  void (*cleanup)(UtObject *self);
  void (*get_interface)(UtObject *object, void *interface_id);
} UtObjectFunctions;

UtObject *ut_object_new(UtObjectFunctions *functions);

UtObject *ut_object_ref(UtObject *object);

void ut_object_unref(UtObject *object);

void *ut_object_get_interface(UtObject *object, void *interface_id);

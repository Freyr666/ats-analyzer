#include<stdint.h>
#include<stdbool.h>

typedef struct {} Probe;

/*
 * Backend initialization struct
 * TODO add type: SDI, external stream, internal UUID stream
 * tag:  stream's UUID
 * arg1: streams's addr
 */
struct init_val {
  char *tag;
  char *arg1;
};

struct callback {
  int32_t id;
  void(*cb)(int32_t, char*);
  void(*reg_thread)();
  void(*unreg_thread)();
};

/**
 * Creates context. Returns NULL and sets [error]
 * on error.
 */
Probe * qoe_probe_create (struct init_val * val,
                          struct callback streams_cb, /* Streams */
                          char **error);

/**
 * Starts context's inner processing pipeline
 */
void qoe_probe_run (Probe *ctx);

/**
 * Does the required cleanup. Invalidates
 * the pointer (the pointer should be set to
 * NULL afterwards)!
 */
void qoe_probe_free (Probe *ctx);

/**
 * Returns parsed streams as json string
 */
char * qoe_probe_get_structure (Probe *ctx);

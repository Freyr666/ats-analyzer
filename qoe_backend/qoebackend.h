#include<stdint.h>
#include<stdbool.h>

typedef struct {} Context;

enum {Video, Audio} typ;
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
        void(*cb)(char*);
        void(*reg_thread)();
        void(*unreg_thread)();
};

struct data_callback {
        void(*cb)(int32_t, char*, uint32_t, uint32_t, void*);
        void(*reg_thread)();
        void(*unreg_thread)();
};

struct status_callback {
        void(*cb)(char*, uint32_t, uint32_t, bool);
        void(*reg_thread)();
        void(*unreg_thread)();
};


/** 
 * Initialize backend logger if needed
 */
void qoe_backend_init_logger (void);

/**
 * Creates context. Returns NULL and sets [error]
 * on error.
 */
Context* qoe_backend_create (struct init_val * vals,
                             uint32_t vals_n,
                             struct callback, /* Streams */
                             struct callback, /* Graph */
                             struct callback, /* Wm */
                             struct data_callback, /* data */
                             struct status_callback, /* status */
                             char **error);

/**
 * Starts context's inner processing pipeline
 */
void qoe_backend_run (Context *ctx);

/**
 * Does the required cleanup. Invalidates
 * the pointer (the pointer should be set to
 * NULL afterwards)!
 */
void qoe_backend_free (Context *ctx);

/**
 * Returns parsed streams as json string
 */
char * qoe_backend_stream_parser_get_structure (Context *ctx);

/**
 * Returns applied streams as json string
 */
char * qoe_backend_graph_get_structure (Context *ctx);

/**
 * Applies [streams] (json string) to the context's processing
 * pipeline. Returns 0 on success and -1 on error, setting [error].
 */
int32_t qoe_backend_graph_apply_structure (Context *ctx,
                                           const char* streams,
                                           char ** error);

/**
 * Returns applied settings as json string
 */
char * qoe_backend_graph_get_settings (Context *ctx,
                                       char** error);

/**
 * Applies [settings] (json string) to the context's processing
 * pipeline. Returns 0 on success and -1 on error, setting [error].
 */
int32_t qoe_backend_graph_apply_settings (Context *ctx,
                                          const char* streams,
                                          char ** error);

/**
 * Returns applied WM layout as json string
 */
char * qoe_backend_wm_get_layout (Context *ctx);

/**
 * Applies [layout] (json string) to the context's WM.
 * Returns 0 on success and -1 on error, setting [error].
 */
int32_t qoe_backend_wm_apply_layout (Context *ctx,
                                     const char* layout,
                                     char ** error);

#ifndef _trios_operator_h_
#define _trios_operator_h_

#ifdef __cplusplus
extern "C" {
#endif

#include "img.h"
#include "dataset.h"
#include "window.h"

struct _image_operator {
    struct _operator_type *type;
    window_t *win;
    
    void *params;
};

typedef struct image_operator image_operator_t;

struct _operator_type {
    int type;
    
    /*image_operator_t *(*train)(xpl_t *data, window_t *win, void *operator_params);*/
    int (*apply)(int *); /* aplica para um ponto */
    int (*free)();
    
    int (*write)(char *filename);
    image_operator_t *(*read)(char *filename);

};

typedef struct _operator_type operator_type;

typedef struct operator_registry_ {
    operator_type *op_type;
    struct operator_registry_ *next;
} operator_registry;

extern operator_registry *reg;

void register_operator_type(operator_type *op_type);

operator_type *get_operator_type(int type);

image_operator_t *image_operator_build(int type, imgset_t *dataset, window_t *win, void *params);

img_t *image_operator_apply(image_operator_t *op, img_t *input, img_t *mask);

image_operator_t *image_operator_read(char *src);

void image_operator_write(char *src);

#ifdef __cplusplus
}
#endif

#endif

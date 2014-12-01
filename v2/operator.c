#include "operator.h"
#include "img.h"
#include "window.h"
#include <stdlib.h>

operator_registry *reg = NULL;

void register_operator_type(operator_type *op_type) {
    operator_registry *new_reg = NULL;
    
    if (op_type == NULL) return;
    
    new_reg = malloc(sizeof(operator_registry));
    new_reg->op_type = op_type;
    new_reg->next = reg;
    
    reg = new_reg;
}

operator_type *get_operator_type(int type) {
    operator_registry *r = reg;
    while (r != NULL) {
        if (r->op_type->type == type) {
            return r->op_type;
        } else {
            r = r->next;
        }
    }
    return NULL; /* or call trios_error ? */
}


image_operator_t *image_operator_build(int type, imgset_t *dataset, window_t *win, void *params) {
    operator_type *tp = NULL;
    image_operator_t *op;
    /*xpl_t *data = NULL;*/
    
    tp = get_operator_type(type);
    if (tp == NULL) {
        /* ERROR: OPERATOR TYPE NOT FOUND!!!! */
    }
    
    /* collec */
    /* pre process xpl ??? */
    /*op = tp->train(data, win, params);*/
    return op;
}

img_t *image_operator_apply(image_operator_t *op, img_t *input, img_t *mask) {
    operator_type *tp;
    img_t *output = NULL;
    int i, j, w, h;
        
    /* put border in mask */
    
    /* slide window through all pixels */
    /* call tp->apply(int*) in each case */ 
    
    return output;
}

image_operator_t *image_operator_read(char *src) {
    operator_type *tp;
    int type = 0;
    /* read operator type from file */
    tp = get_operator_type(type);
    return tp->read(src);
}

void image_operator_write(char *src) {
    operator_type *tp;
    int type = 0;
    /* write operator type to file */
    tp = get_operator_type(type);
    tp->read(src);
}

void trios_register_standard_operators() {
    
}



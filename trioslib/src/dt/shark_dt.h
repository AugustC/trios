

#ifdef __cplusplus
extern "C" {
#endif

#include "trios.h"

dTree *build_shark_tree(mtm_t *mtm);


img_t *apply_shark_tree(dTree *tree__, window_t *win, img_t *input, img_t *mask);

#ifdef __cplusplus
}
#endif

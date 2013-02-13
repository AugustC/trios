#include "../opencv_interface/opencv_interface.h"
#include "../dt/shark_dt.h"

#include "trios_mtm.h"

#include "trios_xpl.h"

dTree *ltrainGG_memory(mtm_t *mtm) {
    //return train_cv_tree(mtm);
    return build_shark_tree(mtm);
}

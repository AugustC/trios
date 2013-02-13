#include <shark/Algorithms/Trainers/CARTTrainer.h>

#include <vector>
#include "trios.h"

extern "C" dTree *build_shark_tree(mtm_t *mtm) {
    int i, j, k, sz, s, nshifts;
    mtm_GX *mt;
    std::vector<shark::RealVector> wpats;
    std::vector<unsigned int> labels;

    sz = mtm->wsize;
    for (i = 0; i < mtm->nmtm; i++) {
        mt = ((mtm_GX *) mtm->mtm_data) + i;
        for (k = 0; k < mt->fq; k++) {
            shark::RealVector rv(sz);
            for(j=0; j < sz; j++) {
                s = j/NB ;
                nshifts = j%NB ;
                rv[j] = (float) ((unsigned char) mt->wpat[j]);
            }
            labels.push_back(mt->label);
            wpats.push_back(rv);
        }
    }

    shark::LabeledData<shark::RealVector, unsigned int> training(wpats, labels, shark::ClassificationDataset::DefaultBatchSize);
    shark::CARTClassifier<shark::RealVector> *tree;
    tree = new shark::CARTClassifier<shark::RealVector>();

    shark::CARTTrainer trainer;
    trainer.train(*tree, training);

    return (dTree *) tree;
}

extern "C" img_t *apply_shark_tree(dTree *tree__, window_t *win, img_t *input, img_t *mask) {
    int i, j, k, l, m, w, h, sz, *offset, label;
    double prob;
    img_t *output;
    printf("SDFSD\n\n");
    shark::CARTClassifier<shark::RealVector> *tree = (shark::CARTClassifier<shark::RealVector> *) tree__;
    w = input->width;
    h = input->height;

    output = img_create(w, h, input->nbands, input->pixel_size);
    sz = win_get_wsize(win);
    shark::RealVector wpat(sz);
    offset = offset_create(sz);
    offset_set(offset, win, w, 1);

    for (i = m = 0; i < h; i++) {
        for (j = 0; j < w; j++, m++) {
            if (img_get_pixel(mask, i, j, 0) == 0) continue;
            for (k = 0; k < sz; k++) {
                l = m + offset[k];
                wpat[k] = img_get_pixel(input, l / input->width, l % input->width, 0);
            }
            shark::RealVector prediction = (*tree)(wpat);

            label = -1;
            prob = -1;
            for (k = 0; k < prediction.size(); k++) {
                if (prediction[k] > prob) {
                    label = k;
                    prob = prediction[k];
                }
            }
            std::cout << prediction << std::endl;
            img_set_pixel(output, i, j, 0, label);
        }

    }
    return output;
}

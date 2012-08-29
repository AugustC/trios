
#include "opencv2/ml/ml.hpp"
#include "opencv2/opencv.hpp"

#include "trios_img.h"
#include "trios_mtm.h"
#include "trios_win.h"

#include "trios_misc.h"
#include "trios_common.h"

#include "stdio.h"

typedef void dTree;

img_t *apply(img_t *img, CvDTree *tr, window_t *win) {
    img_t *output = img_create(img->width, img->height, img->nbands, img->pixel_size);
    cv::Mat wpat(1, win->wsize, CV_32FC1);
    int *offset = offset_create(win_get_wsize(win));
    offset_set(offset, win, img->width, 1);

    printf("Offset\n");
    for (int j = 0; j < win_get_wsize(win); j++) {
        printf("%d ", offset[j]);
    }
    printf("\n\n");
    int n = img->width * img->height;
    for (int k = 0; k < n; k++) {

        for (int j = 0; j < win_get_wsize(win); j++) wpat.at<float>(0, j) = 0.0;

        /* monta padrao */
        for (int j = 0; j < win_get_wsize(win); j++) {
            int l = k + offset[j];
            if (l >= 0 && l < n) {
                wpat.at<float>(0, j) = (float) img_get_pixel(img, l / img->width, l % img->width, 0);
            }
        }
        /* classifica */
        CvDTreeNode *node= tr->predict(wpat);
        int val = node->value;
        /* coloca na imagem */
        img_set_pixel(output, k / img->width, k % img->width, 0, (unsigned char) val);

    }

    return output;
}

extern "C" img_t *lapplyClassifier(img_t *input, dTree *tree, window_t *win) {
    return apply(input, (CvDTree *)tree, win);
}

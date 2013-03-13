#include <cstdio>

#include "opencv2/ml/ml.hpp"
#include "trios_mtm.h"
#include "trios_io.h"

cv::Mat build_classes_from_mtm(mtm_t *mtm) {
    int width = 1;
    int height = mtm->nsum;
    cv::Mat classes(height, width, CV_32FC1);
    int k = 0;
    for (int i = 0; i < mtm->nmtm; i++) {
        mtm_GX *m = (mtm_GX *) mtm->mtm_data + i;
        for (int j = 0; j < m->fq; j++, k++) {
            classes.at<float>(k, 0) = m->label;
        }
    }
    return classes;
}

cv::Mat build_samples_from_mtm(mtm_t *mtm) {
    int width = mtm->wsize;
    int height = mtm->nsum;
    cv::Mat samples(height, width, CV_32FC1);

    for (int i = 0, k = 0; i < mtm->nmtm; i++) {
        mtm_GX *m = (mtm_GX *) mtm->mtm_data + i;
        for (int j = 0; j < m->fq; j++, k++) {
            for (int l = 0; l < mtm->wsize; l++) {
                //printf("%d ", m->wpat[l]);
                samples.at<float>(k, l) = m->wpat[l];
            }
            //printf("\n");
        }
    }
    return samples;
}

int build_samples_xpl_gg_R(xpl_t *xpl, xpl_GG *node, cv::Mat &samples, int i) {
    int j, l;
    freq_node *f;

    if (node == NULL) return i;

    for (f = node->freqlist; f != NULL; f = f->next) {
        for (int j = 0; j < f->freq; j++, i++) {
            for (int l = 0; l < xpl->wsize; l++) {
                samples.at<float>(i, l) = (unsigned char) node->wpat[l];
            }
        }
    }

    i = build_samples_xpl_gg_R(xpl, node->left, samples, i);
    i = build_samples_xpl_gg_R(xpl, node->right, samples, i);

    return i;
}

int build_classes_xpl_gg_R(xpl_t *xpl, xpl_GG *node, cv::Mat &classes, int i) {
    int j, l;
    freq_node *f;

    if (node == NULL) return i;

    for (f = node->freqlist; f != NULL; f = f->next) {
        for (int j = 0; j < f->freq; j++, i++) {
            for (int l = 0; l < xpl->wsize; l++) {
                classes.at<float>(i, 0) = f->label;
            }
        }
    }

    i = build_classes_xpl_gg_R(xpl, node->left, classes, i);
    i = build_classes_xpl_gg_R(xpl, node->right, classes, i);

    return i;
}


cv::Mat build_samples_from_xpl(xpl_t *xpl) {
    int width = xpl->wsize;
    int height = xpl->sum;
    printf("Width %d height %d\n", width, height);
    cv::Mat samples(height, width, CV_32FC1);
    build_samples_xpl_gg_R(xpl, (xpl_GG *)xpl->root, samples, 0);
    return samples;
}

cv::Mat build_classes_from_xpl(xpl_t *xpl) {
    int width = 1;
    int height = xpl->sum;
    cv::Mat classes(height, width, CV_32FC1);
    build_classes_xpl_gg_R(xpl, (xpl_GG *)xpl->root, classes, 0);
    return classes;
}


void test_accuracy(CvDTree &tr, cv::Mat samples, cv::Mat labels) {
    cv::Mat wpat(1, samples.cols, CV_32FC1);
    unsigned long right, wrong, mse;
    int value, label;
    right = wrong = mse = 0;
    for (int i = 0; i < samples.rows; i++) {
        for (int j = 0; j < samples.cols; j++) {
            wpat.at<float>(0, j) = samples.at<float>(i, j);
        }

        CvDTreeNode *node = tr.predict(wpat);
        value = (int) (node->value + 0.5);
        label = (int) labels.at<float>(i, 0);
        if (value == label) {
            right++;
        } else {
            mse += pow(value - labels.at<float>(i, 0), 2);
            wrong++;
            for (int j = 0; j < samples.cols; j++) {
                printf("%d ", (int) samples.at<float>(i, j));
            }
            printf(" val %d pred %d\n", value, label);
        }
    }
    fprintf(stderr, "Acc %f wrong %lu prop %lu/%lu MSE %lu\n", 1.0 * right/(right + wrong), wrong, right, right + wrong, mse);
}

extern "C" void *train_cv_tree(mtm_t *mtm) {
    cv::Mat samples_mtm = build_samples_from_mtm(mtm);
    cv::Mat labels_mtm = build_classes_from_mtm(mtm);
    CvDTreeParams params;
    params.truncate_pruned_tree = false;
    params.min_sample_count = 0;
    params.max_depth = INT_MAX;
    params.cv_folds = 1;
    params.use_1se_rule = false;
    params.use_surrogates = false;
    params.max_categories = INT_MAX;
    cv::Mat var_type(1, mtm->wsize + 1, CV_8UC1);
    for (int i = 0; i < mtm->wsize+1; i++) {
        var_type.at<unsigned char>(0, i) = CV_VAR_CATEGORICAL;
    }
    CvDTree *tr_mtm = new CvDTree();
    tr_mtm->pruned_tree_idx = -INT_MAX;
    tr_mtm->train(samples_mtm, CV_ROW_SAMPLE, labels_mtm, cv::Mat(), cv::Mat(), var_type, cv::Mat(), params);
    test_accuracy(*tr_mtm, samples_mtm, labels_mtm);

    ~samples_mtm;
    ~labels_mtm;

    void *ptr = (void *) tr_mtm;
    return ptr;
}

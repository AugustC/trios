#include <paclib.h>
#include <pacappl.h>
#include "minunit.h"

TEST(test_decision_pair1) {
    int i, j;
    imgset_t *set = imgset_create(1, 2);
    imgset_set_fname(set, 1, 1, "./test_img/input1.pgm");
    imgset_set_fname(set, 2, 1, "./test_img/ideal1.pgm");
    imgset_write("IMGSET.s", set);
    imgset_free(set);

    window_t *win = win_create(2, 2, 1);
    for (i = 0; i < 2; i++) {
        for (j = 0; j < 2; j++) {
            win_set_point(i, j, 1, 1, win);
        }
    }
    win_write("WIN.w", win);

    itv_t *itv = itv_gen_itv(win, 1, BB, 0, 1, 0);

    itv_write("ITV_TEST.itv", itv, win, NULL);
    itv_free(itv);
    win_free(win);


    mu_assert("lcollec failed.", 1 == lcollec("IMGSET.s", "WIN.w", NULL, 1, 1, 0, "XPL_RESULT.xpl", NULL));
    mu_assert("ldecision failed", 1 == ldecision("XPL_RESULT.xpl", 1, 0, AVERAGE, 0, 0, "DECISION_RESULT.mtm"));
    mu_assert("lisi failed", 1 == lisi("DECISION_RESULT.mtm", "ITV_TEST.itv", 3, 5, 0, 0, "ITV_MIN.itv", 0, NULL, NULL ));
    mu_assert("lpapplic failed", 1 == lpapplic("./test_img/ideal1.pgm", "ITV_MIN.itv", NULL, 0, 0, 255, "./test_img/test1.pgm"));
} TEST_END


#include "runner.h"


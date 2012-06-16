#include <trios_img.h>
#include "trios_win.h"
#include "trios_imgset.h"

/**
    Reads three images from an IMGSET: the input image, the ideal image and the mask. If no mask is present,
    an img_t with all pixels set to 1 is returned.

    \param imgset Image set with the images.
    \param map_type Type of the operator that will use the images.
    \param win Estimation window.
    \param k Index of the images in the IMGSET.
    \param img1 Address to put the input image.
    \param img2 Address to put ideal image.
    \param img3 Address to put mask image.
    \return 1 on sucess. 0 on failure.
  */
int get_setofimages(imgset_t *imgset, int map_type, window_t *win, int k, img_t **img1, img_t **img2, img_t **img3);

/**
    Collects examples from a set of images.

    \param fname_i1 IMGSET file.
    \param fname_i2 WINSPEC file.
    \param fname_i3 Optional input XPL file. It must be NULL if not used.
    \param input_type Flag to indicate if the input images are binary. YES=1, NO=0.
    \param output_type Flag to indicate if the output images are binary. YES=1, NO=0.
    \param cv_flag Flag to indicate if all pixels that form a w-pattern must have the same value as the pixel under the central point of the window. This is useful to collect w-patterns ignoring neighboring objects.
    \param fname_o1 Output XPL file.
    \param fname_o2 Optional output report file. It must be NULL if not given.
    \return 1 on success. 0 on failure.

*/
int lcollec(char *fname_i1, char *fname_i2, char *fname_i3, int  input_type, int  output_type, int  cv_flag, char *fname_o1, char *fname_o2);

/* Internal functions. */
int lcollec_main(imgset_t *imgset, window_t *win, xpl_t *xpl, int map_type, int cv_flag, char *log_file);
xpl_t *collec_BB(unsigned short *s1, unsigned char *p2, unsigned char *p3, int *offset, int wsize, int npixels, int cv);
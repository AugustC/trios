#include <trios.h>

/*!
 * Builds an example's set from a pair of input-output
 * images that represents a gray-scale-to-gray-scale transformation. The
 * examples collecting process can be gathered by a mask image.
 * These three images are considered gray-scale ones.
 * \param p1 Input image data.
 * \param p2 Output image data.
 * \param p3 Mask image data.
 * \param offset Vector of offsets (window).
 * \param wsize Size of the offset vector.
 * \param npixels Number of pixels in the images.
 * \return An example's set on success, NULL on failure.
 */
xpl_t *collec_GX(img_t *input, img_t *output, img_t *mask,
		 int *offset, int wsize, int npixels, int type, int q)
{
	int i, j, k, l, m, w, h;
	int *wpat;		/* w-pattern */
	int st, y_off, x_off;
	unsigned int val;
	xpl_t *xpl;		/* XPL structure */
	freq_node *freqnode;	/* pointer to a frequency node */
#ifdef _DEBUG_
	trios_debug("Entrei no collec_GG");
#endif
#ifdef _DEBUG_1_
	for (i = 0; i < wsize; i++) {
		trios_debug("offset[%d]=%d\n", i, offset[i]);
	}
#endif

	wpat = (int *)malloc(sizeof(int) * wsize);
	if (wpat == NULL) {
		return (xpl_t *) trios_error(1, "Memory allocation failed.");
	}

	if ((xpl = xpl_create(wsize, type)) == NULL) {
		free(wpat);
		return (xpl_t *) trios_error(MSG,
					     "collec_GG: xpl_create() failed.");
	}

	w = img_get_width(input);
	h = img_get_height(input);
	
	for (i = 0; i < h; i++) {
		for (j = 0; j < w; j++) {
			if (img_get_pixel(mask, i, j, 0) == 0) continue;
			
			/* blank w-pattern */
			for (k = 0; k < xpl->wsize; k++) {
				wpat[k] = 0;
			}
			
			/* slide window */
			for (k = 0; k < wsize; k++) {
				y_off = offset[k] / w;
				x_off = offset[k] - offset[k] / w * w;
				
				wpat[k] = (int) img_get_pixel_quant(input, i + y_off, j + x_off, 0, q);
			}
			
			/* add into the xpl tree. */
			if (type == GB) {
				val = img_get_pixel(output, i, j, 0);
			} else {
				val = img_get_pixel_quant(output, i, j, 0, q);
			}
			if ((freqnode =
			     freq_node_create((int) val, 1)) == NULL) {
				free(wpat);
				return (xpl_t *) trios_error(MSG,
							     "collec_GG: freq_node_create() failed.");
			}
			
			st = xpl_GG_insert(xpl, (xpl_GG **) (&xpl->root), wpat,
					   freqnode);

			if (st == -1) {
				xpl_free(xpl);
				free(wpat);
				return (xpl_t *) trios_error(MSG,
							     "collec_GG: xpl_GG_insert() failed.");
			}
		}
	}
	
	
	free(wpat);
	return (xpl);

}

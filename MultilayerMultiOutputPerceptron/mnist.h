#ifndef MNIST_H
#define MNIST_H

#include <stdio.h>

#include "mathlib.h"

#ifdef __cplusplus
extern "C" {
#endif

	typedef struct {
		FILE *file_images, *file_labels;
		char *filename_images, *filename_labels;
		int items;
		int rows, cols;
		uint8_t **images;
		uint8_t *labels;
		//uint8_t *image; // deprecated
		//uint8_t label;  // deprecated
	} mnist_t;

	mnist_t *mnist_new(int training);
	void mnist_next(mnist_t *mnist);
	void mnist_destroy(mnist_t *mnist);

#ifdef __cplusplus
}
#endif

#endif

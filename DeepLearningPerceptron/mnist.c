#include <windows.h> // char_to_wchar

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "mnist.h"

uint32_t ChangeEndianness(uint32_t value)
{
	uint32_t result = 0;
	result |= (value & 0x000000FF) << 24;
	result |= (value & 0x0000FF00) << 8;
	result |= (value & 0x00FF0000) >> 8;
	result |= (value & 0xFF000000) >> 24;
	return result;
}


/////
wchar_t *char_to_wchar(char *buf)
{
	int wcsize = MultiByteToWideChar(CP_UTF8, 0, buf, -1, NULL, 0);
	wchar_t *str = (wchar_t *)malloc(wcsize * sizeof(wchar_t));
	MultiByteToWideChar(CP_UTF8, 0, buf, -1, str, wcsize);
	return str;
}
///////


mnist_t *
mnist_new(int training)
{
	mnist_t *mnist = (mnist_t *)malloc(sizeof(mnist_t));
	char buf[BUFSIZ];
	wchar_t *str;

	if (training)
		mnist->filename_images = _strdup("train-images-idx3-ubyte\\train-images.idx3-ubyte");
	else
		mnist->filename_images = _strdup("t10k-images-idx3-ubyte\\t10k-images.idx3-ubyte");
	mnist->file_images = fopen(mnist->filename_images, "rb");
	if (mnist->file_images == NULL) {
		sprintf_s(buf, BUFSIZ, "ERROR: cannot open %s\n", mnist->filename_images);
		str = char_to_wchar(buf);
		OutputDebugString(str);
		free(str);
		return NULL;
	}

	if (training)
		mnist->filename_labels = _strdup("train-labels-idx1-ubyte\\train-labels.idx1-ubyte");
	else
		mnist->filename_labels = _strdup("t10k-labels-idx1-ubyte\\t10k-labels.idx1-ubyte");
	mnist->file_labels = fopen(mnist->filename_labels, "rb");
	if (mnist->file_labels == NULL) {
		sprintf_s(buf, BUFSIZ, "ERROR: cannot open %s\n", mnist->filename_labels);
		str = char_to_wchar(buf);
		OutputDebugString(str);
		free(str);
		return NULL;
	}

	uint32_t magic;
	int size;

	if ((size = fread(&magic, 4, 1, mnist->file_images)) != 1) {
		sprintf_s(buf, BUFSIZ, "ERROR: cannot read header from %s (%d)\n", mnist->filename_images, size);
		str = char_to_wchar(buf);
		OutputDebugString(str);
		free(str);
		return NULL;
	}
	magic = ChangeEndianness(magic);

	if (magic != 0x0803) {
		sprintf_s(buf, BUFSIZ, "ERROR: incorrect magic number 0x%08x %s\n", magic, mnist->filename_images);
		str = char_to_wchar(buf);
		OutputDebugString(str);
		free(str);
		return NULL;
	}

	if ((size = fread(&mnist->items, 4, 1, mnist->file_images)) != 1) {
		sprintf_s(buf, BUFSIZ, "ERROR: cannot read header from %s (%d)\n", mnist->filename_images, size);
		str = char_to_wchar(buf);
		OutputDebugString(str);
		free(str);
		return NULL;
	}
	mnist->items = ChangeEndianness(mnist->items);

	if ((size = fread(&mnist->rows, 4, 1, mnist->file_images)) != 1) {
		sprintf_s(buf, BUFSIZ, "ERROR: cannot read rows from %s (%d)\n", mnist->filename_images, size);
		str = char_to_wchar(buf);
		OutputDebugString(str);
		free(str);
		return NULL;
	}
	mnist->rows = ChangeEndianness(mnist->rows);

	if ((size = fread(&mnist->cols, 4, 1, mnist->file_images)) != 1) {
		sprintf_s(buf, BUFSIZ, "ERROR: cannot read cols from %s (%d)\n", mnist->filename_images, size);
		str = char_to_wchar(buf);
		OutputDebugString(str);
		free(str);
		return NULL;
	}
	mnist->cols = ChangeEndianness(mnist->cols);

	/*
	sprintf_s(buf, BUFSIZ, "INFO: Have %d %dx%d images to process\n", items, rows, cols);
	str = char_to_wchar(buf);
	OutputDebugString(str);
	free(str);
	*/

	if ((size = fread(&magic, 4, 1, mnist->file_labels)) != 1) {
		sprintf_s(buf, BUFSIZ, "ERROR: cannot read header from %s (%d)\n", mnist->filename_labels, size);
		str = char_to_wchar(buf);
		OutputDebugString(str);
		free(str);
		return NULL;
	}
	magic = ChangeEndianness(magic);

	if (magic != 0x0801) {
		sprintf_s(buf, BUFSIZ, "ERROR: incorrect magic number 0x%08x %s\n", magic, mnist->filename_labels);
		str = char_to_wchar(buf);
		OutputDebugString(str);
		free(str);
		return NULL;
	}

	uint32_t items2;
	if ((size = fread(&items2, 4, 1, mnist->file_labels)) != 1) {
		sprintf_s(buf, BUFSIZ, "ERROR: cannot read header from %s (%d)\n", mnist->filename_labels, size);
		str = char_to_wchar(buf);
		OutputDebugString(str);
		free(str);
		return NULL;
	}
	items2 = ChangeEndianness(items2);

	if (mnist->items != items2) {
		sprintf_s(buf, BUFSIZ, "ERROR: image count and label count does not match %s (%d) %s (%d)\n", mnist->filename_images, mnist->items, mnist->filename_labels, items2);
		str = char_to_wchar(buf);
		OutputDebugString(str);
		free(str);
		return NULL;
	}

	//mnist->image = (uint8_t *)malloc(mnist->rows * mnist->cols);

	// read in all images and labels
	mnist->images = (uint8_t **)malloc(mnist->items * sizeof(uint8_t *));
	mnist->labels = (uint8_t *)malloc(mnist->items);

	int n = mnist->rows * mnist->cols;
	for (int i = 0; i < mnist->items; i++) {
		mnist->images[i] = (uint8_t *)malloc(n);
		if ((size = fread(mnist->images[i], n, 1, mnist->file_images)) != 1) {
			sprintf_s(buf, BUFSIZ, "ERROR: cannot read image from %s (%d)\n", mnist->filename_images, size);
			wchar_t *str = char_to_wchar(buf);
			OutputDebugString(str);
			free(str);
			return;
		}

		if ((size = fread(&mnist->labels[i], 1, 1, mnist->file_labels)) != 1) {
			sprintf_s(buf, BUFSIZ, "ERROR: cannot read header from %s (%d)\n", mnist->filename_labels, size);
			wchar_t *str = char_to_wchar(buf);
			OutputDebugString(str);
			free(str);
			return;
		}
	}

	return mnist;
}

/*
void
mnist_next(mnist_t *mnist)
{
	int size;
	char buf[BUFSIZ];
	
	if ((size = fread(mnist->image, mnist->rows*mnist->cols, 1, mnist->file_images)) != 1) {
		sprintf_s(buf, BUFSIZ, "ERROR: cannot read image from %s (%d)\n", mnist->filename_images, size);
		wchar_t *str = char_to_wchar(buf);
		OutputDebugString(str);
		free(str);
		return;
	}

	if ((size = fread(&mnist->label, 1, 1, mnist->file_labels)) != 1) {
		sprintf_s(buf, BUFSIZ, "ERROR: cannot read header from %s (%d)\n", mnist->filename_labels, size);
		wchar_t *str = char_to_wchar(buf);
		OutputDebugString(str);
		free(str);
		return;
	}
	
}
*/

void
mnist_destroy(mnist_t *mnist)
{
	for (int i = 0; i < mnist->items; i++) {
		free(mnist->images[i]);
	}
	free(mnist->images);
	free(mnist->labels);
	free(mnist->filename_images);
	free(mnist->filename_labels);
	fclose(mnist->file_images);
	fclose(mnist->file_labels);
	//free(mnist->image);
}
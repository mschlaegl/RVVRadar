/*
 * Copyright (C) 2021 Manfred Schlaegl <manfred.schlaegl@gmx.at>
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-3.0-only
 */

#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <errno.h>

#include <core/rvv_helpers.h>
#include "bmark.h"


/* data for bmark */
struct data {
	enum bmark_png_filters_filter filter;
	unsigned int len;
	unsigned int bpp;	// bytes per pixel
	uint8_t *prev_row;	// input last row
	uint8_t *row_orig;	// input row
	uint8_t *row;		// input/output row
	uint8_t *row_compare;	// output row to compare
};


/* data for subbmark */
typedef int (*png_filters_fp_t)(unsigned int bpp, unsigned int rowbytes, uint8_t *row, uint8_t *prev_row);
struct subdata {
	png_filters_fp_t png_filters;	// png_filters to be called by wrapper
};


static void diff_row(uint8_t *output, uint8_t *original, int len)
{
	fprintf(stderr, "\n");
	for (int i = 0; i < len; i++)
		if (output[i] != original[i])
			fprintf(stderr, "%s: ERROR: diff on idx=%i: output=%i != original=%i\n",
				__FILE__, i, output[i], original[i]);
	fprintf(stderr, "\n");
}


static int subbmark_preexec(subbmark_t *subbmark, int iteration, bool check)
{
	struct data *d = (struct data*)subbmark->bmark->data;
	/* restore row before benchmark run */
	memcpy(d->row, d->row_orig, d->len * sizeof(*d->row));
	return 0;
}


static int subbmark_exec_wrapper(subbmark_t *subbmark, bool check)
{
	struct data *d = (struct data*)subbmark->bmark->data;
	struct subdata *sd = (struct subdata*)subbmark->data;
	sd->png_filters(d->bpp, d->len, d->row, d->prev_row);
	return 0;
}


static int subbmark_postexec(subbmark_t *subbmark, bool check)
{
	/* result-check disabled -> nothing to do */
	if (!check)
		return 0;

	struct data *d = (struct data*)subbmark->bmark->data;

	/* use memcpy for speed -> use diff only if error was detected */
	int ret = memcmp(d->row, d->row_compare, d->len * sizeof(*d->row));
	if (ret) {
		diff_row(d->row, d->row_compare, d->len * sizeof(*d->row));
		return 1;	/* data error */
	}

	return 0;
}


static int subbmark_add(
	bmark_t *bmark,
	const char *name,
	png_filters_fp_t png_filters)
{
	subbmark_t *subbmark;

	subbmark = bmark_add_subbmark(bmark,
				      name,
				      NULL,
				      subbmark_preexec,
				      subbmark_exec_wrapper,
				      subbmark_postexec,
				      NULL,
				      sizeof(struct subdata));
	if (subbmark == NULL)
		return -1;

	struct subdata *sd = (struct subdata*)subbmark->data;
	sd->png_filters = png_filters;

	return 0;
}



/* up sub benchmarks */

extern void png_filters_up_c_byte_avect(unsigned int bpp, unsigned int rowbytes, uint8_t *row, uint8_t *prev_row);
extern void png_filters_up_c_byte_noavect(unsigned int bpp, unsigned int rowbytes, uint8_t *row, uint8_t *prev_row);
#if RVVBMARK_RVV_SUPPORT
extern void png_filters_up_rvv_m1(unsigned int bpp, unsigned int rowbytes, uint8_t *row, uint8_t *prev_row);
extern void png_filters_up_rvv_m2(unsigned int bpp, unsigned int rowbytes, uint8_t *row, uint8_t *prev_row);
extern void png_filters_up_rvv_m4(unsigned int bpp, unsigned int rowbytes, uint8_t *row, uint8_t *prev_row);
extern void png_filters_up_rvv_m8(unsigned int bpp, unsigned int rowbytes, uint8_t *row, uint8_t *prev_row);
#endif /* RVVBMARK_RVV_SUPPORT */

static int subbmarks_add_up(bmark_t *bmark)
{
	int ret = 0;

	ret |= subbmark_add(bmark, "c byte noavect",	(png_filters_fp_t)png_filters_up_c_byte_noavect);
	ret |= subbmark_add(bmark, "c byte avect",	(png_filters_fp_t)png_filters_up_c_byte_avect);
#if RVVBMARK_RVV_SUPPORT
	ret |= subbmark_add(bmark, "rvv_m1",		(png_filters_fp_t)png_filters_up_rvv_m1);
	ret |= subbmark_add(bmark, "rvv_m2",		(png_filters_fp_t)png_filters_up_rvv_m2);
	ret |= subbmark_add(bmark, "rvv_m4",		(png_filters_fp_t)png_filters_up_rvv_m4);
	ret |= subbmark_add(bmark, "rvv_m8",		(png_filters_fp_t)png_filters_up_rvv_m8);
#endif /* RVVBMARK_RVV_SUPPORT */

	if (ret)
		return -1;

	return 0;
}



/* sub sub benchmarks */

extern void png_filters_sub_c_byte_avect(unsigned int bpp, unsigned int rowbytes, uint8_t *row, uint8_t *prev_row);
extern void png_filters_sub_c_byte_noavect(unsigned int bpp, unsigned int rowbytes, uint8_t *row, uint8_t *prev_row);
#if RVVBMARK_RVV_SUPPORT
extern void png_filters_sub_rvv_dload(unsigned int bpp, unsigned int rowbytes, uint8_t *row, uint8_t *prev_row);
extern void png_filters_sub_rvv_reuse(unsigned int bpp, unsigned int rowbytes, uint8_t *row, uint8_t *prev_row);
#endif /* RVVBMARK_RVV_SUPPORT */

static int subbmarks_add_sub(bmark_t *bmark)
{
	int ret = 0;

	ret |= subbmark_add(bmark, "c byte noavect",	(png_filters_fp_t)png_filters_sub_c_byte_noavect);
	ret |= subbmark_add(bmark, "c byte avect",	(png_filters_fp_t)png_filters_sub_c_byte_avect);
#if RVVBMARK_RVV_SUPPORT
	ret |= subbmark_add(bmark, "rvv_dload",		(png_filters_fp_t)png_filters_sub_rvv_dload);
	ret |= subbmark_add(bmark, "rvv_reuse",		(png_filters_fp_t)png_filters_sub_rvv_reuse);
#endif /* RVVBMARK_RVV_SUPPORT */

	if (ret)
		return -1;

	return 0;
}



/* avg sub benchmarks */

extern void png_filters_avg_c_byte_avect(unsigned int bpp, unsigned int rowbytes, uint8_t *row, uint8_t *prev_row);
extern void png_filters_avg_c_byte_noavect(unsigned int bpp, unsigned int rowbytes, uint8_t *row, uint8_t *prev_row);
#if RVVBMARK_RVV_SUPPORT
extern void png_filters_avg_rvv(unsigned int bpp, unsigned int rowbytes, uint8_t *row, uint8_t *prev_row);
#endif /* RVVBMARK_RVV_SUPPORT */

static int subbmarks_add_avg(bmark_t *bmark)
{
	int ret = 0;

	ret |= subbmark_add(bmark, "c byte noavect",	(png_filters_fp_t)png_filters_avg_c_byte_noavect);
	ret |= subbmark_add(bmark, "c byte avect",	(png_filters_fp_t)png_filters_avg_c_byte_avect);
#if RVVBMARK_RVV_SUPPORT
	ret |= subbmark_add(bmark, "rvv",		(png_filters_fp_t)png_filters_avg_rvv);
#endif /* RVVBMARK_RVV_SUPPORT */

	if (ret)
		return -1;

	return 0;
}



/* paeth sub benchmarks */

extern void png_filters_paeth_c_byte_avect(unsigned int bpp, unsigned int rowbytes, uint8_t *row, uint8_t *prev_row);
extern void png_filters_paeth_c_byte_noavect(unsigned int bpp, unsigned int rowbytes, uint8_t *row, uint8_t *prev_row);
#if RVVBMARK_RVV_SUPPORT
extern void png_filters_paeth_rvv_read_bulk(unsigned int bpp, unsigned int rowbytes, uint8_t *row, uint8_t *prev_row);
extern void png_filters_paeth_rvv(unsigned int bpp, unsigned int rowbytes, uint8_t *row, uint8_t *prev_row);
#endif /* RVVBMARK_RVV_SUPPORT */

static int subbmarks_add_paeth(bmark_t *bmark)
{
	int ret = 0;

	ret |= subbmark_add(bmark, "c byte noavect",	(png_filters_fp_t)png_filters_paeth_c_byte_noavect);
	ret |= subbmark_add(bmark, "c byte avect",	(png_filters_fp_t)png_filters_paeth_c_byte_avect);
#if RVVBMARK_RVV_SUPPORT
	ret |= subbmark_add(bmark, "rvv_read_bulk",	(png_filters_fp_t)png_filters_paeth_rvv_read_bulk);
	ret |= subbmark_add(bmark, "rvv",		(png_filters_fp_t)png_filters_paeth_rvv);
#endif /* RVVBMARK_RVV_SUPPORT */

	if (ret)
		return -1;

	return 0;
}



static int bmark_preexec(struct bmark *bmark, int seed)
{
	struct data *d = (struct data*)bmark->data;

	/* alloc */
	d->prev_row = malloc(d->len * sizeof(*d->prev_row));
	if (d->prev_row == NULL)
		goto __err_alloc_prev_row;
	d->row_orig = malloc(d->len * sizeof(*d->row_orig));
	if (d->row_orig == NULL)
		goto __err_alloc_row_orig;
	d->row = malloc(d->len * sizeof(*d->row));
	if (d->row == NULL)
		goto __err_alloc_row;
	d->row_compare = malloc(d->len * sizeof(*d->row_compare));
	if (d->row_compare == NULL)
		goto __err_alloc_row_compare;

	/* init */
	srandom(seed);
	for (int i = 0; i < d->len; i++) {
		d->prev_row[i] = random();
		d->row_orig[i] = random();
	}
	memcpy(d->row, d->row_orig, d->len * sizeof(*d->row));

	/* calculate compare */
	switch (d->filter) {
	case up:
		png_filters_up_c_byte_avect(d->bpp, d->len, d->row, d->prev_row);
		break;
	case sub:
		png_filters_sub_c_byte_avect(d->bpp, d->len, d->row, d->prev_row);
		break;
	case avg:
		png_filters_avg_c_byte_avect(d->bpp, d->len, d->row, d->prev_row);
		break;
	case paeth:
		png_filters_paeth_c_byte_avect(d->bpp, d->len, d->row, d->prev_row);
		break;
	default:
		goto __err_calc_row_compare;
	}
	memcpy(d->row_compare, d->row, d->len * sizeof(*d->row_compare));

	return 0;

__err_calc_row_compare:
__err_alloc_row_compare:
	free(d->row);
__err_alloc_row:
	free(d->row_orig);
__err_alloc_row_orig:
	free(d->prev_row);
__err_alloc_prev_row:
	return -1;
}


static int bmark_postexec(struct bmark *bmark)
{
	struct data *d = (struct data*)bmark->data;
	if (d == NULL)
		return 0;
	free(d->prev_row);
	free(d->row_orig);
	free(d->row);
	free(d->row_compare);
	return 0;
}


int bmark_png_filters_add(
	bmarkset_t *bmarkset,
	enum bmark_png_filters_filter filter,
	enum bmark_png_filters_bpp bpp,
	unsigned int len)
{
	int ret = 0;
	unsigned int bppval;

	/* parse parameter bpp */
	switch (bpp) {
	case bpp3:
		bppval = 3;
		break;
	case bpp4:
		bppval = 4;
		break;
	default:
		errno = EINVAL;
		return -1;
	}

	/* parse parameter filter and build name string */
	char namestr[256] = "\0";
	switch (filter) {
	case up:
		sprintf(namestr, "png_filters_up");
		break;
	case sub:
		sprintf(namestr, "png_filters_sub%i", bppval);
		break;
	case avg:
		sprintf(namestr, "png_filters_avg%i", bppval);
		break;
	case paeth:
		sprintf(namestr, "png_filters_paeth%i", bppval);
		break;
	default:
		errno = EINVAL;
		return -1;
	}

	/* ensure, that parameter len is a multiple of bpp */
	len = ((len + bppval - 1) / bppval) * bppval;

	/* build parameter string */
	char parastr[256] = "\0";
	snprintf(parastr, 256, "len=%u", len);

	/* create benchmark */
	bmark_t *bmark = bmark_create(
				 namestr,
				 parastr,
				 bmark_preexec,
				 bmark_postexec,
				 sizeof(struct data));
	if (bmark == NULL)
		return -1;

	/* set private data and add sub benchmarks */
	struct data *d = (struct data*)bmark->data;
	d->filter = filter;
	d->len = len;
	d->bpp = bppval;

	/* add sub benchmarks according to parameter filter */
	switch (filter) {
	case up:
		ret = subbmarks_add_up(bmark);
		break;
	case sub:
		ret = subbmarks_add_sub(bmark);
		break;
	case avg:
		ret = subbmarks_add_avg(bmark);
		break;
	case paeth:
		ret = subbmarks_add_paeth(bmark);
		break;
	default:
		errno = EINVAL;
		ret = -1;
	}
	if (ret < 0) {
		bmark_destroy(bmark);
		return -1;
	}

	/* add benchmark to set */
	if (bmarkset_add_bmark(bmarkset, bmark) < 0) {
		bmark_destroy(bmark);
		return -1;
	}

	return 0;
}
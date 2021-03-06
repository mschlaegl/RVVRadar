/*
 * Copyright (C) 2021 Manfred Schlaegl <manfred.schlaegl@gmx.at>
 *
 * SPDX-License-Identifier: MIT
 */

#include <stdint.h>

#include <core/rvv_helpers.h>


#if RVVRADAR_RVV_SUPPORT

/*
 * add two vectors and save result back
 * different vector sizes m1, m2, m4, m8
 */

void png_filters_up_rvv_m1(unsigned int bpp, unsigned int rowbytes, uint8_t *row, uint8_t *prev_row)
{
	/*
	 * row:      | x |
	 * prev_row: | b |
	 *
	 * b = b + x
	 *
	 * b .. 	[v0](e8)
	 * x ..		[v8](e8)
	 */

	while (rowbytes) {
		unsigned int vl = 0;

		asm volatile ("vsetvli		%0, %1, e8, m1" : "=r" (vl) : "r" (rowbytes));

		/* b = *row + *prev_row */
		asm volatile (VLE8_V"		v0, (%0)" : : "r" (row));
		asm volatile (VLE8_V"		v8, (%0)" : : "r" (prev_row));
		prev_row += vl;
		asm volatile ("vadd.vv		v0, v0, v8");

		/* *row = b */
		asm volatile (VSE8_V"		v0, (%0)" : : "r" (row));
		row += vl;

		rowbytes -= vl;
	}
}


void png_filters_up_rvv_m2(unsigned int bpp, unsigned int rowbytes, uint8_t *row, uint8_t *prev_row)
{
	/*
	 * row:      | x |
	 * prev_row: | b |
	 *
	 * b = b + x
	 *
	 * b .. 	[v0-v1](e8)
	 * x ..		[v8-v9](e8)
	 */

	while (rowbytes) {
		unsigned int vl = 0;

		asm volatile ("vsetvli		%0, %1, e8, m2" : "=r" (vl) : "r" (rowbytes));

		/* b = *row + *prev_row */
		asm volatile (VLE8_V"		v0, (%0)" : : "r" (row));
		asm volatile (VLE8_V"		v8, (%0)" : : "r" (prev_row));
		prev_row += vl;
		asm volatile ("vadd.vv		v0, v0, v8");

		/* *row = b */
		asm volatile (VSE8_V"		v0, (%0)" : : "r" (row));
		row += vl;

		rowbytes -= vl;
	}
}


void png_filters_up_rvv_m4(unsigned int bpp, unsigned int rowbytes, uint8_t *row, uint8_t *prev_row)
{
	/*
	 * row:      | x |
	 * prev_row: | b |
	 *
	 * b = b + x
	 *
	 * b .. 	[v0-v3](e8)
	 * x ..		[v8-v11](e8)
	 */

	while (rowbytes) {
		unsigned int vl = 0;

		asm volatile ("vsetvli		%0, %1, e8, m4" : "=r" (vl) : "r" (rowbytes));

		/* b = *row + *prev_row */
		asm volatile (VLE8_V"		v0, (%0)" : : "r" (row));
		asm volatile (VLE8_V"		v8, (%0)" : : "r" (prev_row));
		prev_row += vl;
		asm volatile ("vadd.vv		v0, v0, v8");

		/* *row = b */
		asm volatile (VSE8_V"		v0, (%0)" : : "r" (row));
		row += vl;

		rowbytes -= vl;
	}
}


void png_filters_up_rvv_m8(unsigned int bpp, unsigned int rowbytes, uint8_t *row, uint8_t *prev_row)
{
	/*
	 * row:      | x |
	 * prev_row: | b |
	 *
	 * b = b + x
	 *
	 * b .. 	[v0-v7](e8)
	 * x ..		[v8-v15](e8)
	 */

	while (rowbytes) {
		unsigned int vl = 0;

		asm volatile ("vsetvli		%0, %1, e8, m8" : "=r" (vl) : "r" (rowbytes));

		/* b = *row + *prev_row */
		asm volatile (VLE8_V"		v0, (%0)" : : "r" (row));
		asm volatile (VLE8_V"		v8, (%0)" : : "r" (prev_row));
		prev_row += vl;
		asm volatile ("vadd.vv		v0, v0, v8");

		/* *row = b */
		asm volatile (VSE8_V"		v0, (%0)" : : "r" (row));
		row += vl;

		rowbytes -= vl;
	}
}

#endif /* RVVRADAR_RVV_SUPPORT */

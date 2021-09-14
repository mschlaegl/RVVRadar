#ifndef CHRONO_H
#define CHRONO_H

#include <time.h>
#include <stdio.h>

/*
 * Usage example
 *
 * chrono_t chrono;
 * chrono_init(&chrono);
 * loop {
 * 	chrono_start(&chrono);
 * 	<function to measure>
 * 	if fail
 * 		continue
 * 	chrono_stop(&chrono);
 * }
 * <print statistics from chrono_t>
 * chrono_cleanup(&chrono);
 *
 * Note:
 * A measurement can be aborted (e.g. fail of function to measure) simply
 * by skipping chrono_stop (see fail handling above)
 */


/*
 * maximum number of measurements
 */
#define CHRONO_MAX_MEASUREMENTS	10000


typedef struct chrono {
	/* last start and end times */
	struct timespec tstart;
	struct timespec tend;

	unsigned int max_nmeasure;
	unsigned int nmeasure;
	unsigned long long *tdlist;

	/* live statistics (calculated on each chrono_stop) */
	unsigned long long tdlast;
	unsigned long long tdmin;
	unsigned long long tdmax;
	unsigned long long tdsum;
	unsigned long long tdavg;

	/* statistics (calculated on demand) */
	unsigned int nmeasure_on_last_update;
	unsigned long long tdvar;
	unsigned long long tdstdev;
} chrono_t;


/*
 * init and reset chronometer and statistics
 *   * max_size .. maximum number of expected measurements
 * return: 0 .. ok; <0 .. error
 */
int chrono_init(chrono_t *chrono);


/*
 * cleanup chrono and free ressources
 */
void chrono_cleanup(chrono_t *chrono);


/*
 * start chronometer
 * return: 0 .. ok; <0 .. error
 */
int chrono_start(chrono_t *chrono);


/*
 * stop chronometer and update statistic
 * return: 0 .. ok; <0 .. error
 */
int chrono_stop(chrono_t *chrono);


/*
 * print csv head for chrono statistics
 * (nmeasure;td...)
 * return: same as for fprintf
 */
int chrono_print_csv_head(FILE *out);


/*
 * print chrono statistics as csv
 * (nmeasure;td...)
 * return: same as for fprintf
 */
int chrono_print_csv(chrono_t *chrono, FILE *out);


/*
 * print chrono statistics human readable
 * return: same as for fprintf
 */
int chrono_print_pretty(chrono_t *chrono, const char *indent, FILE *out);


#endif /* CHRONO_H */

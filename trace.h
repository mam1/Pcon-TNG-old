/*
 * trace.h
 *
 *  Created on: Nov 1, 2014
 *      Author: mam1
 */

#ifndef TRACE_H_
#define TRACE_H_

#include <stdbool.h>
#define _TRACE_FILE_NAME	"t01.trc"

int trace_on(char *,int *);
void trace(char *, char *);

#endif /* TRACE_H_ */

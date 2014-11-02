/*
 * trace.c
 *
 *  Created on: Nov 1, 2014
 *      Author: mam1
 */

#include <stdio.h>
#include "trace.h"

int trace_on(char *name, int *flag) {
	FILE *tracef;

	tracef = fopen(name, "w");		//make sure that there is an empty log file
	if (tracef != NULL){
		printf(" trace file <%s> opened\n", name);
		fclose(tracef);
		return 0;
	}
	printf(" can't open trace file <%s>\ntrace disabled\n", name);
	flag = false;
	return 1;
}
void trace(char *name,char *message){
	FILE *tracef;
	tracef = fopen(name, "a");
	fprintf(tracef,"%s\n",message);
	fclose(tracef);
	return;
}

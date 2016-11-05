#ifdef __cplusplus
extern "C" {
#endif 

#ifndef COFFEE_H
#define COFFEE_H

#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>

typedef enum coffee_types_t {
	ZOEGA,
	CAFFE_LATTE,
	CAFFE_MACCHIATO,
	ESPRESSO,
	CAPPUCCINO,
	NUMBER_OF_COFFEES
} coffee_types_t;

void write_current_coffe(int);
void output_coffee_action(int, char *);

#endif

#ifdef __cplusplus
}
#endif
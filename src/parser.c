#include "parser.h"

#include <stdio.h>
#include <string.h>

universe create_universe_example(void){     // TODO
    universe u;
	strcpy(u.name, "Animal");
    return u;
}

void print_universe(universe u){            // TODO
    printf("Universe name: %s\n", u.name);
}
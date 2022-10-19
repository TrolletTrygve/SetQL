#include "parser.h"
#include <stdio.h>

int main(void)
{
	//universe u = create_universe_example();
	//print_universe(u);

	universe u;
	parse_initialization(&u, "./initialization_example.txt");

	print_universe(u);

	return 0;
}
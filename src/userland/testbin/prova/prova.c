#include <stdio.h>
#include <stdlib.h>

int main() {
	int a = 0, b = 0, c = 0, d = 0, e = 0;

	a = 5;
	b = -321;
	a = a + b;
	b = a + a + 3;
	c = b - a;
	d = c * 2;
	e = d - a;

	printf("Result: %d.\n", e);

	return 0;
}
#include <stdio.h>
#include <conio.h>
#define kVALUE 100

int main()
{
	int a = kVALUE;
	while (a == kVALUE)
	{
		printf("addr of a : %p\n", &a);
	}

	printf("\nmodify Success! \n value : %d \n current value : %d", kVALUE, a);
	while (!_kbhit());
	
	return 0;
}
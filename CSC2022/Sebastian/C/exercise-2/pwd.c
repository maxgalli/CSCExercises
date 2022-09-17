#include <stdio.h>
#include <string.h>

#define LENGTH 50

char providedPassword[LENGTH];
char goodPassword[LENGTH];

int main(int argc, char *argv[])
{
	if (argc != 2) {
		printf("Provide a password\nfor example:\n\t./pwd secretPASSword\n");
		return 1;
	}

	strcpy(goodPassword, "This is a very magic secret password...");
	strcpy(providedPassword, argv[1]);

	if (memcmp(goodPassword, providedPassword, LENGTH) == 0) 
          printf("Congratulations, correct password provided!\n");
	else 
          printf("Wrong password!\n");
}

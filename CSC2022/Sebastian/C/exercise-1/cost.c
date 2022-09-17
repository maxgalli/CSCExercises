#include <math.h>
#include <stdio.h>

unsigned int machineDayPriceCents()
{
	// the price for one machine-day in cents (1/100th of an EURO)
	// 3765 cents = 37 EURO 65 cents
	return 3765;
}

unsigned int calculateUsagePrice(unsigned int unitPriceCents, unsigned int noOfUnits)
{
	return round(unitPriceCents * noOfUnits / 100.0);
}

int main(int argc, char *argv[])
{
	if (argc != 2) {
		printf("Provide number of machine-days as an argument\nfor example:\n\t./cost 32\n");
		return 1;
	}
	
	unsigned int units = atoi(argv[1]);
	if (units <= 0) {
		printf("Wrong number of machine-days: '%s'\n", argv[1]);
		return 2;
	}
	
	unsigned int price = machineDayPriceCents();
	printf("Price for one machine-day: %.2f EURO\n", price/100.0);
	
	printf(
		"The cost for %d machine-days is %u EURO\n", 
		units, calculateUsagePrice(price, units));
}


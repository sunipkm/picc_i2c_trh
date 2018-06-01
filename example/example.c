#include <stdio.h>
#include <stdlib.h>
#include <picc_i2c_trh.h>

int main ( void )
{
	hdc1010_dev * dev1 , * dev2 ;

	dev1 = hdc1010_begin(0x41) ;
	dev2 = hdc1010_begin(0x43) ;

	printf ( "Device 1:\nManufacturer ID: 0x%x,\n" , hdc1010_readMfId(dev1) ) ;
	printf ( "Device ID: 0x%x.\n\n" , hdc1010_readDevId(dev1)) ;
	printf ( "Device 2:\nManufacturer ID: 0x%x,\n" , hdc1010_readMfId(dev2) ) ;
	printf ( "Device ID: 0x%x.\n\n" , hdc1010_readDevId(dev2)) ;

	uint64_t time = 0 ;

	while(true)
	{
		printf ( "Acquisition Mode: Independent\n" ) ;
		printf ( "At %d seconds:\n" , time ) ;
		printf ( "Temperature from Device 1: %.3f C\n" , hdc1010_readT(dev1)) ;
		printf ( "Temperature from Device 2: %.3f C\n\n" , hdc1010_readT(dev2)) ;
		printf ( "Relative Humidity from Device 1: %.3f %\n" , hdc1010_readH(dev1)) ;
		printf ( "Relative Humidity from Device 2: %.3f %\n\n" , hdc1010_readH(dev2)) ;
		time = time + 10 ;
		if ( time > 20 )
			break ;
		hdc1010_sleep(10000) ; //spit out numbers in 1 second intervals
		for ( uint8_t i = 0 ; i < 8 ; i++ )
			printf("\x1b[A") ;
	}

	hdc1010_free(dev1) ; hdc1010_free(dev2) ;
}
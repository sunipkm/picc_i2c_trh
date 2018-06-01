#include <stdio.h>
#include <stdlib.h>
#include <picc_i2c_trh.h>

int main ( void )
{
	hdc1010 * dev1 , * dev2 ;

	dev1 = hdc1010_begin(0x41) ;
	dev2 = hdc1010_begin(0x43) ;

	printf ( "Dev1 mfid: 0x%x\n" , hdc1010_readMfId(dev1) ) ;
	printf ( "Dev2 mfid: 0x%x\n" , hdc1010_readMfId(dev2) ) ;

	hdc1010_free(dev1) ; hdc1010_free(dev2) ;
}
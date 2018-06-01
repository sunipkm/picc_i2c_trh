hdc1010 * hdc1010_begin ( uint8_t address )
{
	hdc1010 * dev = ( hdc1010 * ) malloc ( sizeof ( hdc1010_dev ) ) ;

	dev -> address = address ;

	dev -> i2cdevbus = open ( I2C_FILE , 0_RDWR ) ;

	if ( i2cdevbus < 0 )
	{
		fprintf ( stderr,"FILE %s Line %d Function %s: Error opening device %s, %s.\n" , __FILE__ , __LINE__ , __FUNCTION__ , I2C_FILE , strerror ( errno ) ) ;
		return false ;
	}

	if ( ioctl ( i2cdevbus , I2C_SLAVE, address ) < 0 ) 
	{
		fprintf ( stderr,"FILE %s Line %d: ioctl error: %s\n", __FILE__ , __LINE__ , strerror(errno) ) ;
	}

	hdc1010_writeData ( dev , CONFIGURATION ) ;
	hdc1010_writeData ( dev , 0x00 ) ;
	hdc1010_writeData ( dev , 0x00 ) ;

	dev -> mode = 0b00 ;

		/* config: default.
	Heater: Off
	Mode: T or RH
	T: 14 bits
	RH: 14 bits
	*/

	return dev ;
}

void hdc1010_free ( hdc1010 * dev )
{
	free ( dev ) ;
	return ;
}

float hdc1010_readT ( hdc1010_dev * dev )
{
	uint16_t rawT ;
	if ( dev -> mode )
	{
		#ifdef HDC1010_DEBUG
		fprintf(stderr,"FILE %s line %d Function %s: Simultaneous mode, flag: 0x%x\n" , __FILE__ , __LINE__ , __FUNCTION__ , _trh );
		#endif
		if ( dev -> trh == 0b01 ||  dev -> trh == 0b00 ) //if unset or temperature has been read before
		hdc1010_getTRH( dev ) ; //make measurement because temperature data not available anymore
		rawT = (uint16_t)((dev -> trh_buf)>>16) ;
		if ( dev -> trh == 0b11)
		{
			dev -> trh = 0b01 ; //temperature has been read
		}
		if ( _trh == 0b10 ) //if humidity has been read set both to 0
		{
			dev -> trh = 0b00 ;
			dev -> trh_buf = 0x0000000 ;
		}
	}
	else
	{
		rawT = hdc1010_readData(dev , TEMPERATURE) ;
	}
	return ( 1.0 * rawT / 65536.0 ) * 165.0 - 40.0 ;
}

float hdc1010_readH ( hdc1010_dev * dev )
{
	uint16_t rawH ;
	if ( dev -> mode )
	{
		#ifdef HDC1010_DEBUG
		fprintf(stderr,"FILE %s line %d Function %s: Simultaneous mode, flag: 0x%x\n" , __FILE__ , __LINE__ , __FUNCTION__ , _trh );
		#endif
		if ( dev -> trh == 0b10 ||  dev -> trh == 0b00 ) //if unset or humidity has been read before
			hdc1010_getTRH ( dev ) ; //make measurement because humidity data not available anymore
		rawH = (uint16_t)((dev -> trh_buf)) ; //lower word
		if ( dev -> trh == 0b11)
		{
			dev -> trh = 0b10 ; //temperature has been read
		}
		if ( _trh == 0b01 ) //if humidity has been read set both to 0
		{
			dev -> trh = 0b00 ;
			dev -> trh_buf = 0x0000000 ;
		}
	}
	else
	{
		rawT = hdc1010_readData(dev , HUMIDITY) ;
	}
	return ( 1.0 * rawT / 65536.0 ) * 100. ;
}

uint16_t hdc1010_readMfId(hdc1010_dev * dev) {
	return hdc1010_readData(dev , MANUFACTURER_ID);
}

uint16_t hdc1010_readDevId(hdc1010_dev * dev) {
	return hdc1010_readData(dev , DEVICE_ID);
}

hdc1010_regs hdc1010_readReg( hdc1010 * dev ) {
	hdc1010_regs reg;
	reg.rawData = (hdc1010_readData(dev , CONFIGURATION) >> 8);
	return reg;
}

void hdc1010_writeReg(hdc1010_dev * dev , hdc1010_regs reg) {
	hdc1010_writeData(dev , CONFIGURATION);
	hdc1010_writeData(dev , reg.rawData);
	hdc1010_writeData(0x00);
	usleep(PICC_TIME_USEC);
	return ;
}

void hdc1010_heatUp(hdc1010_dev * dev , uint8_t seconds) 
{
	hdc1010_regs reg = hdc1010_readReg(dev);
	reg.Heater = 1;
	reg.ModeOfAcquisition = 1;
	hdc1010_writeReg(dev , reg);

	uint8_t buf[4];
	for (int i = 1; i < (seconds*66); i++) {
		hdc1010_writeData(dev,TEMPERATURE);
		usleep(20000);
		hdc1010_readBytes(dev,buf,4) ;
	}
	reg.Heater = 0;
	reg.ModeOfAcquisition = 0;
	hdc1010_writeReg(dev,reg);
	return ;
}

uint16_t hdc1010_readData ( hdc1010 * dev , uint8_t reg )
{
	hdc1010_writeData(dev , reg) ;
	uint8_t buf[2] ;
	read ( dev -> i2cdevbus , buf , 2 ) ;
	return ( ((uint16_t)buf[0]) << 8 | buf[1] ) ;
}

void hdc1010_writeData(hdc1010 * dev , uint8_t val)
{
	uint8_t buf = val ;
	write ( dev -> i2cdevbus , &buf , 1 ) ;
	usleep ( PICC_TIME_USEC ) ; //wait 20ms before release to give time for response
}

void hdc1010_readBytes ( hdc1010 * dev , uint8_t * buf , uint8_t n )
{
	read( dev -> i2cdevbus , buf , n ) ;
	return ;
}

void hdc1010_sleep ( uint32_t ms )
{
	while(ms--)
		usleep(1000) ;
	return ;
}

void hdc1010_acquisition_mode ( hdc1010 * dev , uint8_t state )
{
	if ( state )
	{
		hd1010_regs reg = hdc1010_readReg ( dev ) ;
		reg.heater = 0 ;
		reg.ModeOfAcquisition = 0 ; //non-simultaneous
		hdc1010_writeReg ( dev , reg ) ;
		dev -> mode = 0 ;
		return ;
	}
	else
	{
		hd1010_regs reg = hdc1010_readReg ( dev ) ;
		reg.heater = 0 ;
		reg.ModeOfAcquisition = 1 ; //simultaneous
		hdc1010_writeReg ( dev , reg ) ;
		dev -> mode = 1 ;

		return ;
	}
	return ;
}

uint32_t hdc1010_getTRH ( hdc1010 * dev )
{
	uint32_t result = 0x0000 ;

    //check flags
    if ( dev -> mode == 0b1 )
  	{
    	uint8_t buf[4];
    	hdc1010_writeData(dev,TEMPERATURE);
    	usleep(PICC_TIME_USEC);
    	hdc1010_readBytes(dev,buf,4) ;
    	//encoding into 32bit word
    	result = ((uint32_t)buf[0]<<24)|((uint32_t)buf[1]<<16)|((uint32_t)buf[2]<<8)|buf[3] ;
    	dev -> trh_buf = result ;
    	dev -> trh = 0b11 ; //gcc only, both are available for read
    }
    return result ;
}
#include <avr/io.h>
#include <util/delay.h>
#include <stdio.h>
#include <string.h>

#include "diskio.h"
#include "pff.h"
#include "petit_util.h"

uint16_t SD_BUF_SIZE;
uint8_t *sd_buf;

uint8_t  petit_util_petit_init( void * petit_buf, uint16_t sd_buf_size, uint8_t spi2x ) {
	uint8_t res;

	SPI_DIR 	|= CS | MOSI | SCK;
	SPI_PORT 	|= CS;
	SPCR 		|= (1<<SPE)|(1<<MSTR);

	sd_buf = petit_buf;
	SD_BUF_SIZE = sd_buf_size;

	if( spi2x ) SPCR |= (1<<SPI2X);

#if USE_SD_PWR == 1
	sd_pwr(0);
	_delay_ms(50);
	sd_pwr(1);
	_delay_ms(50);
#endif

	res = disk_initialize();

	return res;
}

#if USE_FILE_APPEND == 1

int petit_util_pf_file_append(FATFS *wfs, char * fname, char *txt) {

	uint16_t 	i, buf_free=0, len_txt = strlen(txt);
	uint32_t 	sektor=0, tmr = 65000;
	uint8_t 	res=0;
	WORD 		rb;
	uint8_t 	*bf = sd_buf;

#if USE_SD_PWR == 1
	sd_pwr(0);	
	_delay_ms(100);
	sd_pwr(1);	
#endif

	res = petit_util_sd_init();	
	if( res ) return res;

	res = petit_util_mount(wfs);  
	if( res ) return res;

	res = petit_util_open(fname);	
	if( res ) return res;

	while(1) {

		res = 1;
		pf_read(sd_buf, 512, &rb);
		if( rb<512 ) break;

		for(i=0; i<512; i++) if( bf[i] == FILE_BLANK_CHAR ) break;

		if(512==i) { sektor++; continue; }

		buf_free = 512 - i;

		if(buf_free>=len_txt) {

			memcpy(&bf[i], txt, len_txt);

			tmr=1000; res=100;
			while( pf_lseek(sektor*512UL) && tmr-- );
			if(!tmr) break;

			res=5;
			pf_write(&bf[0], 512, &rb);
			if(rb<512) {
				break;	
			}

			res = 0; 

		} else { 

			memcpy(&bf[i], txt, buf_free);

			tmr=1000; res=100;
			while( pf_lseek(sektor*512UL) && tmr-- );
			if(!tmr) break;

			res=5;
			pf_write(&bf[0], 512, &rb);
			if(rb<512) break;
			pf_write(0, 0, &rb);

			res = petit_util_sd_init();
			if( res ) return res;

			res = petit_util_mount(wfs);
			if( res ) return res;

			res = petit_util_open(fname);
			if( res ) return res;

			tmr=1000; res=100;
			while( pf_lseek((sektor+1)*512UL) && tmr-- );
			if(!tmr) break;

			pf_read(sd_buf, SD_BUF_SIZE, &rb);

			memcpy(bf, &txt[buf_free], strlen(&txt[buf_free]));

			tmr=1000; res=100;
			while( pf_lseek((sektor+1)*512UL) && tmr-- );
			if(!tmr) break;
			pf_write(&bf[0], 512, &rb);

			pf_write(0, 0, &rb);

			res = petit_util_sd_init();
			if( res ) return res;

		}
		break;
	}	

	disk_initialize();

	return res;
}
#endif


#if USE_SD_PWR == 1

void sd_pwr( uint8_t OnOff ) {
	SD_PWR_DIR |= SD_PWR_PIN;
	if(OnOff) {
		SD_ON;
		SPCR |= (1<<SPE);
		_delay_ms(50);
		disk_initialize();
	} else {
		SD_OFF;
		SPCR &= ~(1<<SPE);
		PORTB &= ~(SCK|MISO|MOSI);
		_delay_ms(300);	
	}
}
#endif

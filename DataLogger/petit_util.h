#include "pff.h"
#include "diskio.h"

#ifndef MK_PETIT_UTIL_H_
#define MK_PETIT_UTIL_H_

#define USE_SD_PWR		0

#define USE_FILE_APPEND 1

#define FILE_BLANK_CHAR	0xA0

#define SCK 		(1<<PB5)
#define MISO 		(1<<PB4)
#define MOSI 		(1<<PB3)
#define CS 			(1<<PB2)
#define SPI_PORT	PORTB
#define SPI_DIR		DDRB

#define SD_PWR_PIN		(1<<PB1)
#define SD_PWR_PORT		PORTB
#define SD_PWR_DIR		DDRB

#define SD_ON SD_PWR_PORT 	&= ~SD_PWR_PIN
#define SD_OFF SD_PWR_PORT 	|= SD_PWR_PIN

extern uint8_t *sd_buf;

inline uint8_t petit_util_sd_init(void) {
	uint16_t tmr=1000;
	while( disk_initialize() && tmr-- );
	if( !tmr ) return 255;
	return 0;
}

inline uint8_t petit_util_mount(FATFS *wfs) {
	uint16_t tmr=1000;
	uint8_t res;
	while( (res = pf_mount(wfs)) && tmr-- );
	if( !tmr ) return 255;
	if( res ) return res;
	return 0;
}

inline uint8_t petit_util_open(char * fname) {
	uint16_t tmr=1000;
	uint8_t res;
	while( (res = pf_open(fname)) && tmr-- );
	if( !tmr ) return 255;
	if( res ) return res;
	return 0;
}

uint8_t petit_util_petit_init( void * petit_buf, uint16_t sd_buf_size, uint8_t spi2x );
int petit_util_pf_file_append( FATFS *wfs, char *fname, char *txt );

void sd_pwr( uint8_t OnOff );

#endif 

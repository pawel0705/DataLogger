#include <avr/io.h>
#include <avr/pgmspace.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <string.h>

#include "ds18b20.h" // DS18B20 code
#include "lcd.h" // LCD 2x16 code
#include "petit_util.h" // petitfs library add-on that supports saving to SD card
#include "diskio.h" // sd card input output
#include "pff.h" // petitfs methods

#define PCF8583_ADDR 0xA2 // space character for writing to the SD card

FATFS fs; // SD card file
uint8_t sd_bufor[512]; // sd card information buffer

char rekord[150]; // buffer for informations that will be save to SD card
char printbuff[100]; // buffer for temperature conversion to string
char minMaxTempBuff[100]; // buffer holding information about the maximum or minimum temperature in the form of a string

int volatile secondsToSave = 0; // number of past seconds for writing information to the SD card
int volatile secondsToScreen = 0; // number of past seconds for the display to go into temperature display state
int volatile secondsToUpdateTempCounter = 0; // number of past seconds to download a new temperature value
int volatile lightDisplay = 0; // number of past seconds until the display backlight goes off
int volatile refreshDisplayCounter = 0; // number of past seconds to refresh the display
int volatile checkSDCardCounter = 0; // number of past seconds to check the presence of the SD card in the system
int volatile diodeErrorCounter = 0; // number of past seconds to flash with LED indicating error status
int volatile alarmCounter = 0; // number of past seconds to turn on the buzzer

int timeSave = 10; // number of seconds to write
int secondsToUpdateTemp = 5; // number of seconds to update the temperature
int timeChoose = 0; // counter for selecting the time interval for saving to the SD card
int minTemperature = -1000; // minimum temperature value
int maxTemperature = 1000; // maximum temperature value
int res = 0; // sd card status
int sdMissing = 0; // variable indicating whether there is an SD card in the device
int tempSteps = 14; // number of possible values for the selection of temperature limits
int swapAlarmMode = 0; // alarm for temperature above or below

double temperature = 0; // temperature value

// interruption for timer1 mode that starts every second
ISR (TIMER1_COMPA_vect)
{
	secondsToSave += 1; // increment time to write to the SD card
	secondsToScreen += 1; // incrementing time to display transition to temperature display mode
	secondsToUpdateTempCounter += 1; // incrementing time to measure the new temperature
	lightDisplay += 1; // incrementing the time until the display backlight goes off
	refreshDisplayCounter += 1; // incrementing time to refresh the display
	checkSDCardCounter += 1; // incrementing time to check if the SD card is inserted
	diodeErrorCounter += 1; // incrementing time to blink LED indicating error
	alarmCounter += 1; // incrementing time to turn on the alarm buzzer
}

void alarmChange();
void playAlarm();
void blinkErrorDiode();
void refreshDisplay();
void saveToSDCard();
void displayTemperature();
void updateLightDisplay();
void updateTemperature();
void lightDisplayChange();
void saveTimeChange();
void checkSDCard();

// main program function, place where the program starts
int main(void)
{
	lcd_init(); // initialize lcd
	lcd_set_left_to_right(); // set lcd mode from left to right
	lcd_on(); // turn on lcd
	lcd_clear(); // clear any characters on the lcd
	
	lcd_puts("Data logger"); // put string on lcd
	lcd_set_cursor(0, 1); // set cursor to next line
	lcd_puts("Starting..."); // put string on lcd
	lcd_set_cursor(0,0); // reset cursor
	_delay_ms(1000); // wait 1s
	lcd_clear(); // clear display
	
	uint8_t ires; // variable indicating the state of the SD card
	ires = petit_util_petit_init(sd_bufor, sizeof(sd_bufor), 0); // initialize the SD card
	
	if( ires ) // failed to initialize SD card
	{
		lcd_puts("Initializing SD"); // put string on lcd
		lcd_set_cursor(0, 1); // set cursor to next line
		lcd_puts("card..."); // put string on lcd
	}

	while(petit_util_petit_init( sd_bufor, sizeof(sd_bufor), 0 )); // initialize the SD card until it is initialized
	_delay_ms(1000); // wait 1s
	
	// refresh LCD
	lcd_init(); // initialize LCD
	lcd_set_cursor(0, 0); // reset cursor position
	lcd_off(); // turn off lcd
	lcd_on(); // turn on lcd
	lcd_clear(); // clear lcd from any characters
	initPins(); // init pins
	
	sei(); // unlock interrupts
	
	temperature = ds18b20_gettemp(); // get the temperature from the sensor
	dtostrf(temperature, 9, 1, printbuff); // convert the temperature in the form of a floating point number to a buffer string
	lcd_puts("  Temperature"); // put string on lcd
	lcd_set_cursor(0, 1); // set cursor to next line
	lcd_puts(printbuff); // put temperature on lcd
	lcd_puts("C"); // put C (Celsius)
	
	while(1) // infinite main program loop
	{
		alarmChange();
		playAlarm();
		checkSDCard();
		refreshDisplay();
		saveToSDCard();
		displayTemperature();
		updateLightDisplay();
		updateTemperature();
		lightDisplayChange();
		saveTimeChange();
		blinkErrorDiode();
	}
	
	return 0; // return will not be achieved because of inifinite program loop
}

// deviuce pins initialization function
void initPins()
{
	DDRC |= (1<<PC1); // diode
	DDRC |= (1<<PC2); // buzzer
	DDRB |= (1<<PB0); // backlight LCD
	
	PORTC = 8; // button PC3
	PORTC = 16; // button PC4
	PORTC = 32; // button PC5
	
	OCR1A =  31250; // overflow count for 8MHz for 1 sec
	TCCR1B |= (1 << WGM12); // Mode 4, CTC on OCR1A
	TIMSK1 |= (1 << OCIE1A); //Set interrupt on compare match
	TCCR1B |= (1 << CS12); // set prescaler to 256 and start the timer
}

// buzzer function (alarm)
void playAlarm()
{
	// condition checking if the set minimum or maximum temperature has been exceeded
	if(minTemperature <= temperature && minTemperature != -1000 || maxTemperature >= temperature && maxTemperature != 1000)
	{
		// after three seconds turn off the alarm
		if(alarmCounter >= 3)
		{
			alarmCounter = 1; // set alarm duration to 1
			PORTC &= ~(1<<PC2); // turn off the alarm
		}
		else if(alarmCounter == 2) // turn on the alarm after two seconds
		{
			PORTC |= (1<<PC2); // turn on the alarm
		}
	}
}

// function responsible for modifying the temperature value after which the alarm will be triggered
void alarmChange()
{
	if(!(PINC & 16)) // the PC4 button was pressed
	{
		secondsToScreen = 0; // reset the time responsible for the display to enter the temperature display state
		lightDisplay = 0; // reset the time responsible for backlight display
		alarmCounter = 0; // reset the time responsible for tigger buzzer
		PORTB |= (1<<PB0); // turn on the display backlight
		PORTC &= ~(1<<PC2); // turn off the alarm
		
		if(tempSteps >= 14) // condition checking if the user has passed all proposed temperature values
		{
			minTemperature = -1000; // set the minimum temperature to very low so that the alarm would not have a chance to start
			maxTemperature = 1000; // set the maximum temperature to very high so that the alarm would not have a chance to start
			
			lcd_clear(); // clear lcd from characters
			lcd_puts("Alarm temp"); // put string on lcd
			lcd_set_cursor(0, 1); // set cursor to next line
			lcd_puts("Alarm OFF"); // put string on lcd
			lcd_set_cursor(0, 0); // reset cursor
			
			tempSteps = 0; // reset proposed temperature values counter
			PORTC &= ~(1<<PC2); // turn off the alarm
			
			if(swapAlarmMode == 0) // alarm mode for temperature above selected value
			{
				swapAlarmMode = 1;
			}
			else // alarm mode for temperature under selected value
			{
				swapAlarmMode = 0;
			}
			
			_delay_ms(300); // wait 0.3s (contact vibrations)
		}
		else // the user reviews the proposed temperature values after which the alarm is triggered
		{
			if(swapAlarmMode == 0) // alarm mode for temperatures above the selected range
			{
				if(minTemperature == -1000) // start of temperature selection
				{
					minTemperature = -30; // setting the threshold at which the selection will start
				}
				else
				{
					minTemperature += 5; // suggest another temperature range 5 greater than the previous one
				}
			}
			else // alarm mode for temperatures below the selected range
			{
				if(maxTemperature == 1000) // start of temperature selection
				{
					maxTemperature = 30; // // setting the threshold at which the selection will start
				}
				else
				{
					maxTemperature -= 5; // suggest another temperature range 5 less than the previous one
				}
			}
			
			lcd_clear(); // clear lcd from any characters
			
			if(swapAlarmMode == 0) // alarm mode for temperatures above the selected range
			{
				lcd_puts("Alarm temp above"); // put string on lcd
				lcd_set_cursor(0, 1); // set cursor to next line
				sprintf(minMaxTempBuff,"%iC", minTemperature); // convert temperature as floating point to string
				lcd_puts(minMaxTempBuff); // put temperature on screen
			}
			else // alarm mode for temperatures above the selected range
			{
				lcd_puts("Alarm temp below"); // put string on lcd
				lcd_set_cursor(0, 1); // set cursor to next line
				sprintf(minMaxTempBuff,"%iC", maxTemperature); // convert temperature as floating point to string
				lcd_puts(minMaxTempBuff); // put temperature on screen
			}
			
			lcd_set_cursor(0, 0); // reset cursor
			tempSteps++; // another temperature suggestion
			_delay_ms(300); // wait 0.3s (contact vibrations)
		}
	}
}

// function responsible for modifying the time interval to save data to the SD card
void saveTimeChange()
{
	if(!(PINC & 8)) // PIN3 of port C is high
	{
		timeChoose = 1; // setting a variable informing that the user is currently in the mode of selecting the time to save data to the SD card
		secondsToScreen = 0; // reset the time responsible for the display to enter the temperature display state
		lightDisplay = 0; // reset the time responsible for backlight display
		PORTB |= (1<<PB0); // turn on the display backlight
		
		if(timeSave == 10) // the current save time to the SD card is every 10 seconds
		{
			lcd_clear(); // clear lcd from any characters
			lcd_puts("Save time"); // put string to lcd
			lcd_set_cursor(0, 1); // put cursor to next line
			lcd_puts("30sec"); // put string to lcd
			lcd_set_cursor(0, 0); // reset cursor
			timeSave = 30; // set save time to 30 seconds
			_delay_ms(300); // wait 0.3s (contact vibrations)
		}
		else if(timeSave == 30) // the current save time to the SD card is every 30 seconds
		{
			lcd_clear(); // clear lcd from any characters
			lcd_puts("Save time"); // put string to lcd
			lcd_set_cursor(0, 1); // put cursor to next line
			lcd_puts("1min"); // put string to lcd
			lcd_set_cursor(0, 0); // reset cursor
			timeSave = 60; // set save time to 60 seconds
			_delay_ms(300); // wait 0.3s (contact vibrations)
		}
		else if(timeSave == 60) // the current save time to the SD card is every 60 seconds
		{
			lcd_clear(); // clear lcd from any characters
			lcd_puts("Save time"); // put string to lcd
			lcd_set_cursor(0, 1); // put cursor to next line
			lcd_puts("5min"); // put string to lcd
			lcd_set_cursor(0, 0); // reset cursor
			timeSave = 300; // set save time to 300 seconds
			_delay_ms(300); // wait 0.3s (contact vibrations)
		}
		else if(timeSave == 300) // the current save time to the SD card is every 300 seconds
		{
			lcd_clear(); // clear lcd from any characters
			lcd_puts("Save time"); // put string to lcd
			lcd_set_cursor(0, 1); // put cursor to next line
			lcd_puts("10min");  // put string to lcd
			lcd_set_cursor(0, 0); // reset cursor
			timeSave = 600; // set save time to 600 seconds
			_delay_ms(300); // wait 0.3s (contact vibrations)
		}
		else if(timeSave == 600) // the current save time to the SD card is every 600 seconds
		{
			lcd_clear(); // clear lcd from any characters
			lcd_puts("Save time"); // put string to lcd
			lcd_set_cursor(0, 1); // put cursor to next line
			lcd_puts("30min");  // put string to lcd
			lcd_set_cursor(0, 0); // reset cursor
			timeSave = 1800; // set save time to 1800 seconds
			_delay_ms(300); // wait 0.3s (contact vibrations)
		}
		else if(timeSave == 1800) // the current save time to the SD card is every 1800 seconds
		{
			lcd_clear(); // clear lcd from any characters
			lcd_puts("Save time"); // put string to lcd
			lcd_set_cursor(0, 1); // put cursor to next line
			lcd_puts("1h"); // put string to lcd
			lcd_set_cursor(0, 0); // reset cursor
			timeSave = 3600; // set save time to 3600 seconds
			_delay_ms(300); // wait 0.3s (contact vibrations)
		}
		else // the current save time to the SD card is every 3600 seconds
		{
			lcd_clear(); // clear lcd from any characters
			lcd_puts("Save time"); // put string to lcd
			lcd_set_cursor(0, 1); // put cursor to next line
			lcd_puts("10sec"); // put string to lcd
			lcd_set_cursor(0, 0); // reset cursor
			timeSave = 10; // set save time to 10 seconds
			_delay_ms(300); // wait 0.3s (contact vibrations)
		}
	}
}

// function checking if an SD card is inserted
void checkSDCard()
{
	if(checkSDCardCounter > 2) // after the right time has passed, check if the SD card is in the device
	{
		checkSDCardCounter = 0; // reset the time responsible for keeping an eye on checking the SD card
		if(petit_util_petit_init(sd_bufor, sizeof(sd_bufor), 0)) // try to initialize the card
		{
			sdMissing = 1; // sd is missing
		}
		else // sd is ok
		{
			sdMissing = 0; // sd isn't missing
			PORTC &= ~(1<<PC1); // turn alarm led off
		}
	}
}

// the method responsible for switching on the display backlight
void lightDisplayChange()
{
	if(!(PINC & 32)) // PIN3 of port C is high
	{
		lightDisplay = 0; // reset the counting time of the display backlight seconds
		PORTB |= (1<<PB0); // turn on the cacklight
	}
}

// function responsible for updating the temperature
void updateTemperature()
{
	// update the temperature if the specified time has passed and the user is not in the process of choosing the time
	if(secondsToUpdateTempCounter >= secondsToUpdateTemp && timeChoose == 0)
	{
		secondsToUpdateTempCounter = 0; // reset the time keeping the temperature updated
		temperature = ds18b20_gettemp(); // get a new temperature value from the sensor
		dtostrf(temperature, 9, 1, printbuff); // convert the temperature in a floating point to a string
		lcd_clear(); // clear lcd from any characters
		lcd_puts("  Temperature"); // put string on lcd
		lcd_set_cursor(0, 1); // set cursot to next line
		lcd_puts(printbuff); // put temperature on screen
		lcd_puts("C"); // put C (Celsius)
	}
	
}

// function responsible for turning off the display backlight
void updateLightDisplay()
{
	if(lightDisplay > 3) // after waiting for sufficient time, turn off the display backlight
	{
		PORTB &= ~(1<<PB0); // turn off the backlight
		lightDisplay = 0; // resetting the backlight duration
	}
}

// function responsible for refreshing the display
void refreshDisplay()
{
	// refresh the display after waiting for sufficient time
	if(refreshDisplayCounter > 6)
	{
		refreshDisplayCounter = 0; // reset the refresh time
		
		lcd_off(); // turn off lcd
		lcd_init(); // init lcd
		lcd_on(); // turn on lcd
		lcd_clear(); // clear any characters
		lcd_puts("  Temperature"); // put string to lcd
		lcd_set_cursor(0, 1); // set cursor to next line
		lcd_puts(printbuff); // set temperature to lcd
		lcd_puts("C"); // C (Celsius)
	}
}

// function responsible for blinking diode
void blinkErrorDiode()
{
	if(sdMissing == 1) // no SD card
	{
		if(diodeErrorCounter == 1) // light up diode every second
		{
			PORTC |= (1<<PC1); // light up diode
		}
		else if(diodeErrorCounter >= 2) // turn off the diode after 2 seconds
		{
			diodeErrorCounter = 0; // reset light diode time
			PORTC &= ~(1<<PC1); // turn off the diode
		}
	}
}

// function responsible for displaying the temperature after user selection on the display
void displayTemperature()
{
	// display the temperature if the user has finished his selection
	if(secondsToScreen % 2 == 0 && timeChoose == 1 && secondsToScreen != 0)
	{
		lcd_clear(); // clear lcd from any characters
		lcd_puts("  Temperature"); // put string on lcd
		lcd_set_cursor(0, 1); // set curson to next line
		lcd_puts(printbuff); // put temperature on screen
		lcd_puts("C"); // put C (Celsius)
		
		timeChoose = 0; // reset choosing time
	}
}

// function responsible for saving temperature measurement on an SD card
void saveToSDCard()
{
	// save the measurement to the SD card if the specified time has passed and the SD card is in the device
	if(secondsToSave >= timeSave && sdMissing == 0)
	{
		secondsToSave = 0; // reset save to sd time
		PORTC |= (1<<PC1); // turn on led
		sprintf(rekord,"Time mode:   %isec;Temp:%sC;\n",timeSave, printbuff); // prepare the appropriate text for writing
		petit_util_pf_file_append(&fs, "data.txt", rekord); // save data to sd card
		PORTC &= ~(1<<PC1); // turn off led
	}
}
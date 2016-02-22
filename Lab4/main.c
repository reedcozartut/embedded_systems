//***********************  main.c  ***********************
// Program written by:
// - Steven Prickett  steven.prickett@gmail.com
//
// Brief desicription of program:
// - Initializes an ESP8266 module to act as a WiFi client
//   and fetch weather data from openweathermap.org
//
//*********************************************************
/* Modified by Jonathan Valvano
 Feb 3, 2016
 Out of the box: to make this work you must
 Step 1) Set parameters of your AP in lines 59-60 of esp8266.c
 Step 2) Change line 39 with directions in lines 40-42
 Step 3) Run a terminal emulator like Putty or TExasDisplay at
         115200 bits/sec, 8 bit, 1 stop, no flow control
 Step 4) Set line 50 to match baud rate of your ESP8266 (9600 or 115200)
 */
#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>

#include "../inc/tm4c123gh6pm.h"

#include "pll.h"
#include "UART.h"
#include "esp8266.h"
#include "LED.h"
#include "ST7735.h"

// prototypes for functions defined in startup.s
void DisableInterrupts(void); // Disable interrupts
void EnableInterrupts(void);  // Enable interrupts
long StartCritical (void);    // previous I bit, disable interrupts
void EndCritical(long sr);    // restore I bit to previous value
void WaitForInterrupt(void);  // low power mode

int ParseResponse(char* resp); //subroutine for parsing the openweather.org json string

char Fetch[] = "GET /data/2.5/weather?q=Austin%20Texas&APPID=e18aa6ec1dab60a6867898c207404521 HTTP/1.1\r\nHost:api.openweathermap.org\r\n\r\n";
char ResponseJson[SERVER_RESPONSE_SIZE];

// 1) go to http://openweathermap.org/appid#use 
// 2) Register on the Sign up page
// 3) get an API key (APPID) replace the 1234567890abcdef1234567890abcdef with your APPID

int main(void){  
	char* lab4greeting = "Welcome to Lab 4!";
	
  DisableInterrupts();
  PLL_Init(Bus80MHz);
	//ST7735_InitR(INITR_REDTAB);
  LED_Init();  
  Output_Init();       // UART0 only used for debugging
	ST7735_Output_Init(); 
  printf("\n\r-----------\n\rSystem starting...\n\r");
	ST7735_DrawString(0,0, lab4greeting, ST7735_WHITE);
	ST7735_DrawString(0,1, "Fetching Weather Data...", ST7735_WHITE);
  ESP8266_Init(115200);      // connect to access point, set up as client
  ESP8266_GetVersionNumber();
	
  while(1){
    ESP8266_GetStatus();
    if(ESP8266_MakeTCPConnection("openweathermap.org")){ // open socket in server
      LED_GreenOn();
      ESP8266_SendTCP(Fetch);
    }
    ESP8266_CloseTCPConnection(); 
		ParseResponse(ResponseJson);
		printf(ResponseJson);
    while(Board_Input()==0){// wait for touch
    }; 
    LED_GreenOff();
    LED_RedToggle();
  }
}

// transparent mode for testing
void ESP8266SendCommand(char *);
int main2(void){  char data;
  DisableInterrupts();
  PLL_Init(Bus80MHz);
  LED_Init();  
  Output_Init();       // UART0 as a terminal
  printf("\n\r-----------\n\rSystem starting at 9600 baud...\n\r");
//  ESP8266_Init(38400);
  ESP8266_InitUART(9600,true);
  ESP8266_EnableRXInterrupt();
  EnableInterrupts();
  ESP8266SendCommand("AT+RST\r\n");
  data = UART_InChar();
//  ESP8266SendCommand("AT+UART=115200,8,1,0,3\r\n");
//  data = UART_InChar();
//  ESP8266_InitUART(115200,true);
//  data = UART_InChar();
  
  while(1){
// echo data back and forth
    data = UART_InCharNonBlock();
    if(data){
      ESP8266_PrintChar(data);
    }
  }
}

int ParseResponse(char* resp){
	char* sub_str;
	uint32_t offs;
	uint32_t temp;
	char temp_str[11] = {'T', 'e', 'm', 'p', ':' ,' ', '0', '0', '0', ' ', 'K'};
	
	
	
	sub_str = strstr(ResponseJson, "temp");
	offs = (int) ((int) sub_str - (int)ResponseJson);
	
	temp = ((int) ResponseJson[offs+6] - 0x30)*100;
	temp += ((int) ResponseJson[offs+7] -0x30)*10;
	temp += ((int) ResponseJson[offs+8] -0x30);
	
	temp_str[6] = ResponseJson[offs+6];
	temp_str[7] = ResponseJson[offs+7];
	temp_str[8] = ResponseJson[offs+8];
	
	ST7735_DrawString(0,1, "                        ", ST7735_WHITE);
	ST7735_DrawString(0,1, temp_str, ST7735_WHITE);
	
	return 0;
}

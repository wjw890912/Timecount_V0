#ifndef	DISPLAY_H
#define DISPLAY_H


void RotaryEncoderHWinit(void);	
void DisplayHWInit(void);
void Timetick50us(void);
void SetBuzzer(uint8_t status);
void SetNumber(uint8_t Num1,uint8_t Num2,uint8_t Num3,uint8_t Num4);
void DisplayPoll(void);
 void CountPoll(void);
 void BuzzePoll(void );
 void TimeInit(void);


#endif
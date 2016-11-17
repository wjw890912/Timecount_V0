#include <stdio.h>
#include "M051Series.h"
#include "Display.h"


//数码管的扫描频率	50HZ
#define DISPLAY_SCAN_TIME_HZ   50
//按键的扫描频率	250HZ
//#define KEY_SCAN_TIME_HZ	   20

#define DG1OPEN	               P25=1
#define DG1CLOSE               P25=0

#define DG2OPEN	               P26=1
#define DG2CLOSE               P26=0

#define DG3OPEN	               P27=1
#define DG3CLOSE               P27=0

#define DG4OPEN	               P44=1
#define DG4CLOSE               P44=0

#define BUZZER                 P42



#define TIMESTARTDEC		    1
#define TIMEINIT		        0
#define TIMESTOP		        2


#define  GET_BUTTON  	       (P43)

#define  RUNKEY 		        0x01 

const uint8_t NumberTab[10]={0xc0,0xf9,0xa4,0xb0,0x99,0x92,0x82,0xf8,0x80,0x90};/*CA mode 7seg data */
static uint8_t DG1number=0, DG2number=0, DG3number=0,DG4number=0;  //locke save the display memoery
 int32_t TimeCount=0;// the RotaryEncoder value  is seconed  
static int32_t LastValue=0;// 用来存储定时值的中间结果  结果=0的话不做任何处理，不为0意味着有数据需要存进FLASH的请求。
static uint8_t Arlm=0;//报警值 0不报警 1报警
static uint8_t ButtonNum=TIMEINIT;
static uint8_t Continuous=0;
int flash;


/*

  1、上电状态初始化为 TIMEINIT，并加载断电之前记住的定时值 。
  2、按下按键开始倒计时。状态变为 TIMESTARTDEC  时间开始减少，到0就报警。
  3、在此按下按键。如果为0则停止蜂鸣器并恢复倒计时之前的值，不为0则暂停。
  4、再按一下按键，继续开始倒计时，
  依次往复。

按键次数    0：不动作，1：开始倒计时，到0之后变为2，重新加载上次的技术值， 并变为0，重复之前的动作。





*/
void TimeInit(void)
{

	 ButtonNum=TIMEINIT;//初始化状态
//   LastValue = GetFlashValue();	 //from EEPROM or FLASH
	FMC_Open();
//	FMC_Erase(0x1f000);
//	FMC_Write(0x1f000, 0x12345678);
	LastValue=FMC_Read(0x1f000);
	 if(LastValue>100*60)
	 {
	   LastValue=0;
	 }
	 else
	 {
	   LastValue= LastValue;
	 }
	 TimeCount=LastValue;//加载上次状态
	 LastValue=0;//重新初始化掉--不为零就将写入FLASH中，这里不需要写入所以不允许。 

}
void SaveFlash(int32_t *pt)
{

	if(*pt!=0)
	{
	  //save value to FLASH
	  // PutFlashData(*pt);

		flash= *pt;//用内存来模拟

	FMC_Erase(0x1f000);
    FMC_Write(0x1f000, flash);

		*pt =0;//修改值重新为0，
	}
	else
	{
	   // do noting 
	
	}


}

int GetFlash(void)
{
	//get flash the time value 
	//return GetFlahData();

	flash=FMC_Read(0x1f000);

	return flash;

} 


void TimeRunHook(void)
{
  //按键扫描
		
				  switch (ButtonNum)
				  {


				  
					 case TIMEINIT: {

							  ButtonNum=TIMESTARTDEC;//开始倒计时
							  if((Continuous==0)|(Continuous==2))
							  {
							  Continuous=0;
							  LastValue=TimeCount;//暂存计数值，并送往FLASH进行暂存
							  SaveFlash(&LastValue); //存入FLASH中，并清空 LastValue
					 		  }

					 break;}

					 case 1: {
								 if(Arlm)
								 {
								 //如果已经报警那么
					 			 ButtonNum=TIMESTOP;//	停止蜂鸣器，并回复倒计时前的值。
								 Arlm=0;//关闭蜂鸣器
								 TimeCount=GetFlash();
								 Continuous=2;

								 }
								 else
								 {
							
								 //如果还没有报警，那么暂停当前定时即可
								 ButtonNum=TIMESTOP;//	停止蜂鸣器，并回复倒计时前的值。
								 Continuous=1;
								 }

								 ButtonNum=TIMEINIT;//	恢复重复开始
					 
					 break;}

				
				


				  
				  }


				
			

}


void DisplayHWInit(void)
{

	/*
	   SEG : 
	    P0端口对应数码管的数据总线
		   P00--a
		   p01--b
		   p02--c									
		   p03--d								   
		   p04--e									
		   p05--f
		   p06--g
		   p07--dp
		COM :

		   P2.5--DG1
		   P2.6--DG2
		   P2.7--DG3
		   P4.4--DG4
		BEEP ：
		   P4.2--BEEP 0:CLOSE 1 OPEN .
	
	
	
	*/
   GPIO_SetMode(P0, BIT0|BIT1|BIT2|BIT3|BIT4|BIT5|BIT6|BIT7, GPIO_PMD_OUTPUT); // 8bit SEG data bus	 setting output 
   GPIO_SetMode(P2, BIT5|BIT6|BIT7, GPIO_PMD_OUTPUT);// 3 bit COM data bus 
   GPIO_SetMode(P4, BIT4, GPIO_PMD_OUTPUT);// 4 com
   GPIO_SetMode(P4, BIT2, GPIO_PMD_OUTPUT);// BEEP
   GPIO_SetMode(P4, BIT3, GPIO_PMD_INPUT );// key
   DG1CLOSE;// close DG1
   DG2CLOSE;// close DG2
   DG3CLOSE;// close DG3


}
void TimeDec(void)
 {
 
 	
		
		static uint32_t	n=0; 


				    n++;
				if(n>(1000000/50)) //1S 时间到了执行一次
				{
				            n=0;
				   
						   if(ButtonNum==TIMESTARTDEC)
						   {
								   	   if(TimeCount==0)
									   {
									     Continuous=0;
									   	  Arlm = 1;//允许报警
									   }
									   else
									   {
									      Arlm = 0;//不允许报警
									   	 TimeCount--;//时间减少一秒
									   
									   }
						   }
				   
				}

		 
 
 
 
 
 }
void DisplayPoll(void)
{
		static uint8_t count=1;
		
			

			switch (count)
				   {
				   	  case 1:
					  {
					  	DG4CLOSE;// close DG3
						DG1OPEN; // hold DG1 utill next option 
						GPIO_SET_OUT_DATA(P0,(NumberTab[DG1number]));
					    
						break;
					  }

					  case 2:
					  {
					  	DG1CLOSE;
						DG2OPEN;
					    GPIO_SET_OUT_DATA(P0,NumberTab[DG2number]&0x7f);
					    
						break;
					  }

				   	  case 3:
					  {
					  	DG2CLOSE;
						DG3OPEN;
					    GPIO_SET_OUT_DATA(P0,NumberTab[DG3number]);
					    
					  	break;
					  }
				   	  case 4:
					  {
					  	DG3CLOSE;
						DG4OPEN;
					    GPIO_SET_OUT_DATA(P0,NumberTab[DG4number]);
					    
					  	break;
					  }
				   
				   }
				   
				  count++;
				  if(count>4)count=0;
			  
				
}

 static uint32_t time=0,time1=0;
 
 /*it must be pic ever 50us poll once again and again */
 void Timetick50us(void)
 {
  		 TimeDec();
		 time++;
		 time1++;
		
 }

 
  

unsigned char Trg;//触发
unsigned char Cont;//连续

void KeyRead( void )
{
    unsigned char ReadData = 255;

		  ReadData =255;
	if(GET_BUTTON==1){ReadData=(ReadData&0xfe);}

//	if(!GET_BUTTON2){ReadData=(ReadData&0xfd);}

//	if(!GET_BUTTON3){ReadData=(ReadData&0xfb);}

//	if(!GET_BUTTON4){ReadData=(ReadData&0xf7);}

	ReadData =ReadData ^0xff;
    Trg = ReadData & (ReadData ^ Cont); 
    Cont = ReadData; 

}

void KeyProc(void)
{
  
	if(Cont|Trg )
	{
		
	}
	
       if (Cont & RUNKEY )
    {    




	}
	if (Trg & RUNKEY )
	{
	  	 
	    TimeRunHook();//scan key and process fast so fast 			
				 
	  
	}

 }
 void GPIOP2P3P4_IRQHandler	()
 {
 
 	 GPIO_CLR_INT_FLAG(P4, BIT3);

	  TimeRunHook();//scan key and process fast so fast 


 
 }

 void DisplayScan(void)
 {
 
  /* 
		 
		 timecount  = (((1/f)*1000000)/50)
		 1000000: much us in a second
		 50: timer gernraled time pluse is 20KHZ so it is 50us once 

		 eg:  f=50HZ
		   
		 timecount = (((1/50)*1000000)/50)
		         =400
		
		 
		 */
		 if(time>(((1/DISPLAY_SCAN_TIME_HZ)*1000000)/50))
		 {
		    DisplayPoll();
		 	time=0;
		 }
		 // if(time1>(((1/KEY_SCAN_TIME_HZ)*1000000)/50))
		 //{	
		 
		  //	KeyRead();
		  //	KeyProc();
		//	time1=0;
 	//	 }


 }


/*
	 set diplay number that the number isn't full over number "9"

	 eg :  SetNumber(1,2,3);

	 option in evertime .

*/
void SetNumber(uint8_t Num1,uint8_t Num2,uint8_t Num3,uint8_t Num4)
{


		if(Num1<=9)
		{
	    DG1number = Num1;
		}
		if(Num2<=9)
		{ 
		DG2number = Num2;
		}
		if(Num3<=9)
		{ 
		DG3number = Num3;
		}
		if(Num4<=9)
		{ 
		DG4number = Num4;
		}



}



 void SetBuzzer(uint8_t status)
 {
 		  if(status)
		  {
		   BUZZER=1;
		  }
		  else
		  {
		   BUZZER=0;
		  }
 
 }
void BuzzePoll(void )
{
   if(Arlm)
   {

   	  SetBuzzer(1);
   
   }
   else
   {
   	 SetBuzzer(0);
   }

}

 /*
 The External INT0(P3.2) default IRQ, declared in startup_M051Series.s.
 */
void EINT0_IRQHandler(void)
{
    /* For P3.2, clear the INT flag */
    GPIO_CLR_INT_FLAG(P3, BIT2);


		   if(ButtonNum!=1)
		   {

			  
				if(P33==0)
				{
				  //正转
				  if(TimeCount<20*59)
				  {
				 TimeCount+=15;
				  }
				  else
				  {
				  
				   TimeCount+=59;
				  }
				}
				else
				{
				 //反转
				 if(TimeCount<20*59)
				 {
				 TimeCount-=15;
				 }
				 else
				 {
				  TimeCount-=59;
				 }
				}
			 
			 

		   }



}
/*
The External INT1(P3.3) default IRQ, declared in startup_M051Series.s.
 */
void EINT1_IRQHandler(void)
{
    /* For P3.3, clear the INT flag */
    GPIO_CLR_INT_FLAG(P3, BIT3);
}


void RotaryEncoderHWinit()
{

    /* Configure P3.2 as EINT0 pin and enable interrupt by rising  edge trigger */
    GPIO_SetMode(P3, BIT2, GPIO_PMD_INPUT);
    GPIO_EnableEINT0(P3, 2, GPIO_INT_RISING);
    NVIC_EnableIRQ(EINT0_IRQn);

    /* Configure P3.3 as EINT1 pin and enable interrupt by rising  edge trigger */
    GPIO_SetMode(P3, BIT3, GPIO_PMD_INPUT);
    GPIO_EnableEINT1(P3, 3, GPIO_INT_RISING);
    NVIC_EnableIRQ(EINT1_IRQn);
		
    /* Enable interrupt de-bounce function and select de-bounce sampling cycle time is 1024 * 10 KHz clock */
    GPIO_SET_DEBOUNCE_TIME(GPIO_DBCLKSRC_LIRC, GPIO_DBCLKSEL_4);
    GPIO_ENABLE_DEBOUNCE(P3, BIT2 | BIT3);

	 /* Configure P3.3 as EINT1 pin and enable interrupt by rising  edge trigger */
    GPIO_SetMode(P4, BIT3, GPIO_PMD_INPUT);
    GPIO_EnableEINT1(P4, 3, GPIO_INT_RISING);
    NVIC_EnableIRQ(GPIO_P2P3P4_IRQn);
    GPIO_ENABLE_DEBOUNCE(P4, BIT3);




 }

 void CountPoll(void)
 {

 uint8_t g,s,b,q;
   

	  if(TimeCount>5999)
	   {
	   TimeCount=5999;
	   }
	   if(TimeCount<0)
	   {
	   	TimeCount=0;
	   }

	  q=(TimeCount/60)/10;

	  b=(TimeCount/60)%10;

	  s=TimeCount%60/10;
	  g=TimeCount%60%10;
	 
	 SetNumber(q,b,s,g);


 }


















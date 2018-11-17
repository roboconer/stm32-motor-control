#include "sys.h"
#include "delay.h"
#include "usart.h"
#include "led.h"
#include "pwm.h"
#include "icapture.h"
#include "math.h"
#include "can1.h"
#include "usart.h"

u16 signal[4];
extern u16 temp[4];
extern unsigned char Uart1_Buffer[];
extern unsigned int Uart1_Rx;
int main(void)
{ 
	
		NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);//设置系统中断优先级分组2
		delay_init(168);  //初始化延时函数
		uart_init(115200);//初始化串口波特率为115200
	  LED_Init();	 
	  GPIO_Digital_Init();
		TIM4_Cap_Init(0XFFFF,84-1);
		TIM14_PWM_Init(500-1,84-1);	//84M/84=1Mhz的计数频率,重装载值500，所以PWM频率为 1M/500=2Khz.     
		TIM3_PWM_Init(500-1,84-1);
	
//CAN初始化
	  CAN1_Configuration(); 
	  delay_ms(500);                                      //刚开始要有足够的延时，确保驱动器已经初始化完成 
    CAN_RoboModule_DRV_Reset(0,0);                      //对0组驱动器全部复位 
	  delay_ms(500);
    CAN_RoboModule_DRV_Config(0,1,1,0);               //1号驱动器配置为1ms传回一次数据
    delay_us(100);                                      //此处延时为了不让传回数据时候4个不一起传
    CAN_RoboModule_DRV_Config(0,2,1,0);               //2号驱动器配置为1ms传回一次数据
    delay_us(100);
    CAN_RoboModule_DRV_Mode_Choice(0,0,Velocity_Position_Mode);     //选择进入速度位置模式
    delay_ms(500); 
	
	while(1)
	{
		input_capture(signal);
		//printf("%d\t%d\t%d\t%d\r\n",signal[0],signal[1],signal[2],signal[3]);
		TIM_SetCompare1(TIM14,100+3.125*(signal[1]-1000)); 	                    //singal[1]_PD15_CH6
//		for(int so=0;so<100;so++){
//		printf("%c\r\n",Res);} 		
		printf("%s\r\n",Uart1_Buffer);
		if((signal[2]-1590)>0){
		PEout(0)=0;
		PEout(1)=1;
    TIM_SetCompare1(TIM3,(int)(100+(signal[2]-1590)/2)); //1590+
    //printf("电机1输出PWM值%d\r\n",(int)(100+(signal[2]-1590)/2));			
		}	
		else if((abs(signal[2]-1015))<20)
		{
			TIM_SetCompare1(TIM3,0);
		//	printf("电机1输出PWM值0");
		}
		else{
     PEout(0)=1;
		 PEout(1)=0;			
		 TIM_SetCompare1(TIM3,(int)(75+(1590-signal[2])*0.8));
		 //printf("电机1输出PWM值%d\r\n",(int)(75+(1590-signal[2])*0.8));
		}
	}	
 
}

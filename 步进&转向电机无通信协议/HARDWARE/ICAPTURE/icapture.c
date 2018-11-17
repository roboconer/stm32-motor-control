#include "icapture.h"
#include "led.h"
#include "usart.h"
//#include "includes.h"


//定时器5通道1输入捕获配置
//arr：自动重装值(TIM2,TIM5是32位的!!)
//psc：时钟预分频数
void TIM4_Cap_Init(u16 arr,u16 psc)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	TIM_ICInitTypeDef  TIM4_ICInitStructure;

	
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4,ENABLE);  	//TIM4时钟使能    
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD, ENABLE); 	//使能PORTA时钟	
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12|GPIO_Pin_13|GPIO_Pin_14|GPIO_Pin_15; //GPIOA0
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;//复用功能
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;	//速度100MHz
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP; //推挽复用输出
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_DOWN; //下拉
	GPIO_Init(GPIOD,&GPIO_InitStructure); //初始化PA0

	GPIO_PinAFConfig(GPIOD,GPIO_PinSource12,GPIO_AF_TIM4); //PA0复用位定时器5
	GPIO_PinAFConfig(GPIOD,GPIO_PinSource13,GPIO_AF_TIM4);
	GPIO_PinAFConfig(GPIOD,GPIO_PinSource14,GPIO_AF_TIM4);
	GPIO_PinAFConfig(GPIOD,GPIO_PinSource15,GPIO_AF_TIM4);
  
	TIM_TimeBaseStructure.TIM_Prescaler=psc;  //定时器分频
	TIM_TimeBaseStructure.TIM_CounterMode=TIM_CounterMode_Up; //向上计数模式
	TIM_TimeBaseStructure.TIM_Period=arr;   //自动重装载值
	TIM_TimeBaseStructure.TIM_ClockDivision=TIM_CKD_DIV1; 
	
	TIM_TimeBaseInit(TIM4,&TIM_TimeBaseStructure);
	

	//初始化TIM4输入捕获参数
	TIM4_ICInitStructure.TIM_Channel = TIM_Channel_1; //CC1S=01 	选择输入端 IC1映射到TI1上
  TIM4_ICInitStructure.TIM_ICPolarity = TIM_ICPolarity_Rising;	//上升沿捕获
  TIM4_ICInitStructure.TIM_ICSelection = TIM_ICSelection_DirectTI; //映射到TI1上
  TIM4_ICInitStructure.TIM_ICPrescaler = TIM_ICPSC_DIV1;	 //配置输入分频,不分频 
  TIM4_ICInitStructure.TIM_ICFilter = 0x00;//IC1F=0000 配置输入滤波器 不滤波
  TIM_ICInit(TIM4, &TIM4_ICInitStructure);
	TIM4_ICInitStructure.TIM_Channel = TIM_Channel_2;
	TIM_ICInit(TIM4, &TIM4_ICInitStructure);
	TIM4_ICInitStructure.TIM_Channel = TIM_Channel_3;
	TIM_ICInit(TIM4, &TIM4_ICInitStructure);
	TIM4_ICInitStructure.TIM_Channel = TIM_Channel_4;
	TIM_ICInit(TIM4, &TIM4_ICInitStructure);

  NVIC_InitStructure.NVIC_IRQChannel = TIM4_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=2;//抢占优先级3
	NVIC_InitStructure.NVIC_IRQChannelSubPriority =0;		//子优先级3
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			//IRQ通道使能
	NVIC_Init(&NVIC_InitStructure);	//根据指定的参数初始化VIC寄存器、
	
	
	TIM_ITConfig(TIM4,TIM_IT_Update|TIM_IT_CC1|TIM_IT_CC2|TIM_IT_CC3|TIM_IT_CC4 ,ENABLE);//允许更新中断 ,允许CC1IE捕获中断	    
	
	TIM_Cmd(TIM4,ENABLE ); 	//使能定时器5
	
}
//捕获状态
//[7]:0,没有成功的捕获;1,成功捕获到一次.
//[6]:0,还没捕获到低电平;1,已经捕获到低电平了.
//[5:0]:捕获低电平后溢出的次数(对于32位定时器来说,1us计数器加1,溢出时间:4294秒)
u8  TIM4CH1_CAPTURE_STA=0,TIM4CH2_CAPTURE_STA = 0,TIM4CH3_CAPTURE_STA = 0,TIM4CH4_CAPTURE_STA = 0;	//输入捕获状态		    				
u32	TIM4CH1_CAPTURE_VAL,TIM4CH2_CAPTURE_VAL,TIM4CH3_CAPTURE_VAL,TIM4CH4_CAPTURE_VAL;	//输入捕获值(TIM2/TIM4是32位)
//定时器4中断服务程序	 
void TIM4_IRQHandler(void)
{ 		    
	//OSIntEnter();
 	if((TIM4CH1_CAPTURE_STA&0X80)==0)//还未成功捕获	
	{	  
		if (TIM_GetITStatus(TIM4, TIM_IT_Update) != RESET)
		 
		{	    
			if(TIM4CH1_CAPTURE_STA&0X40)//已经捕获到高电平了
			{
				if((TIM4CH1_CAPTURE_STA&0X3F)==0X3F)//高电平太长了
				{
					TIM4CH1_CAPTURE_STA|=0X80;//标记成功捕获了一次
					TIM4CH1_CAPTURE_VAL=0XFFFF;
				}else TIM4CH1_CAPTURE_STA++;
			}	 
		}
	if (TIM_GetITStatus(TIM4, TIM_IT_CC1) != RESET)//捕获1发生捕获事件
		{	
			if(TIM4CH1_CAPTURE_STA&0X40)		//捕获到一个下降沿 		
			{	  			
				TIM4CH1_CAPTURE_STA|=0X80;		//标记成功捕获到一次上升沿
				TIM4CH1_CAPTURE_VAL=TIM_GetCapture1(TIM4);
		   		TIM_OC1PolarityConfig(TIM4,TIM_ICPolarity_Rising); //CC1P=0 设置为上升沿捕获
			}else  								//还未开始,第一次捕获上升沿
			{
				TIM4CH1_CAPTURE_STA=0;			//清空
				TIM4CH1_CAPTURE_VAL=0;
	 			TIM_SetCounter(TIM4,0);
				TIM4CH1_CAPTURE_STA|=0X40;		//标记捕获到了上升沿
		   		TIM_OC1PolarityConfig(TIM4,TIM_ICPolarity_Falling);		//CC1P=1 设置为下降沿捕获
			}		    
		}			     	    					   
 	}
	//////////////////////////////////////////CH2/////////////////////////////////////////////////////////
	
	if((TIM4CH2_CAPTURE_STA&0X80)==0)//还未成功捕获	
	{	  
		if (TIM_GetITStatus(TIM4, TIM_IT_Update) != RESET)
		 
		{	    
			if(TIM4CH2_CAPTURE_STA&0X40)//已经捕获到高电平了
			{
				if((TIM4CH2_CAPTURE_STA&0X3F)==0X3F)//高电平太长了
				{
					TIM4CH2_CAPTURE_STA|=0X80;//标记成功捕获了一次
					TIM4CH2_CAPTURE_VAL=0XFFFF;
				}else TIM4CH2_CAPTURE_STA++;
			}	 
		}
	if (TIM_GetITStatus(TIM4, TIM_IT_CC2) != RESET)//捕获2发生捕获事件
		{	
			if(TIM4CH2_CAPTURE_STA&0X40)		//捕获到一个下降沿 		
			{	  			
				TIM4CH2_CAPTURE_STA|=0X80;		//标记成功捕获到一次上升沿
				TIM4CH2_CAPTURE_VAL=TIM_GetCapture2(TIM4);
		   		TIM_OC2PolarityConfig(TIM4,TIM_ICPolarity_Rising); //CC2P=0 设置为上升沿捕获
			}else  								//还未开始,第一次捕获上升沿
			{
				TIM4CH2_CAPTURE_STA=0;			//清空
				TIM4CH2_CAPTURE_VAL=0;
	 			TIM_SetCounter(TIM4,0);
				TIM4CH2_CAPTURE_STA|=0X40;		//标记捕获到了上升沿
		   		TIM_OC2PolarityConfig(TIM4,TIM_ICPolarity_Falling);		//CC2P=2 设置为下降沿捕获
			}		    
		}			     	    					   
 	}
	
	////////////////////////////////////////CH3/////////////////////////////////////////////////////////
	if((TIM4CH3_CAPTURE_STA&0X80)==0)//还未成功捕获	
	{	  
		if (TIM_GetITStatus(TIM4, TIM_IT_Update) != RESET)
		 
		{	    
			if(TIM4CH3_CAPTURE_STA&0X40)//已经捕获到高电平了
			{
				if((TIM4CH3_CAPTURE_STA&0X3F)==0X3F)//高电平太长了
				{
					TIM4CH3_CAPTURE_STA|=0X80;//标记成功捕获了一次
					TIM4CH3_CAPTURE_VAL=0XFFFF;
				}else TIM4CH3_CAPTURE_STA++;
			}	 
		}
	if (TIM_GetITStatus(TIM4, TIM_IT_CC3) != RESET)//捕获3发生捕获事件
		{	
			if(TIM4CH3_CAPTURE_STA&0X40)		//捕获到一个下降沿 		
			{	  			
				TIM4CH3_CAPTURE_STA|=0X80;		//标记成功捕获到一次上升沿
				TIM4CH3_CAPTURE_VAL=TIM_GetCapture3(TIM4);
		   		TIM_OC3PolarityConfig(TIM4,TIM_ICPolarity_Rising); //CC3P=0 设置为上升沿捕获
			}else  								//还未开始,第一次捕获上升沿
			{
				TIM4CH3_CAPTURE_STA=0;			//清空
				TIM4CH3_CAPTURE_VAL=0;
	 			TIM_SetCounter(TIM4,0);
				TIM4CH3_CAPTURE_STA|=0X40;		//标记捕获到了上升沿
		   		TIM_OC3PolarityConfig(TIM4,TIM_ICPolarity_Falling);		//CC3P=3 设置为下降沿捕获
			}		    
		}			     	  
	
 	}
//	////////////////////////////////////////CH4////////////////////////////////////////////////////////
	if((TIM4CH4_CAPTURE_STA&0X80)==0)//还未成功捕获	
	{	  
		if (TIM_GetITStatus(TIM4, TIM_IT_Update) != RESET)
		 
		{	    
			if(TIM4CH4_CAPTURE_STA&0X40)//已经捕获到高电平了
			{
				if((TIM4CH4_CAPTURE_STA&0X3F)==0X3F)//高电平太长了
				{
					TIM4CH4_CAPTURE_STA|=0X80;//标记成功捕获了一次
					TIM4CH4_CAPTURE_VAL=0XFFFF;
				}else TIM4CH4_CAPTURE_STA++;
			}	 
		}
	if (TIM_GetITStatus(TIM4, TIM_IT_CC4) != RESET)//捕获3发生捕获事件
		{	
			if(TIM4CH4_CAPTURE_STA&0X40)		//捕获到一个下降沿 		
			{	  			
				TIM4CH4_CAPTURE_STA|=0X80;		//标记成功捕获到一次上升沿
				TIM4CH4_CAPTURE_VAL=TIM_GetCapture4(TIM4);
		   		TIM_OC4PolarityConfig(TIM4,TIM_ICPolarity_Rising); //CC4P=0 设置为上升沿捕获
			}else  								//还未开始,第一次捕获上升沿
			{
				TIM4CH4_CAPTURE_STA=0;			//清空
				TIM4CH4_CAPTURE_VAL=0;
	 			TIM_SetCounter(TIM4,0);
				TIM4CH4_CAPTURE_STA|=0X40;		//标记捕获到了上升沿
		   		TIM_OC4PolarityConfig(TIM4,TIM_ICPolarity_Falling);		//CC4P=3 设置为下降沿捕获
			}		    
		}			     	    					   
 	}
	TIM_ClearITPendingBit(TIM4,TIM_IT_CC2|TIM_IT_CC3|TIM_IT_CC4|TIM_IT_CC1|TIM_IT_Update); //清除中断标志位
	//OSIntExit();		
}

void input_capture(u16 temp[4]){
static u16 temp_f[4]={1517,1517,1517,1517};
		temp[0] = 0;
		temp[1] = 0;
		temp[2] = 0;
		temp[3] = 0;

	 		if(TIM4CH1_CAPTURE_STA&0X80)        //成功捕获到了一次高电平
		{
			temp[2]=TIM4CH1_CAPTURE_STA&0X3F; 
			temp[2]*=0XFFFFFFFF;		 		         //溢出时间总和
			temp[2]+=TIM4CH1_CAPTURE_VAL;		   //得到总的高电平时间
			TIM4CH1_CAPTURE_STA=0;			     //开启下一次捕获
		}
		if(TIM4CH2_CAPTURE_STA&0X80)        //成功捕获到了一次高电平
		{
			temp[3]=TIM4CH2_CAPTURE_STA&0X3F; 
			temp[3]*=0XFFFFFFFF;		 		         //溢出时间总和
			temp[3]+=TIM4CH2_CAPTURE_VAL;		   //得到总的高电平时间
			TIM4CH2_CAPTURE_STA=0;			     //开启下一次捕获
		}
		if(TIM4CH3_CAPTURE_STA&0X80)        //成功捕获到了一次高电平
		{
			temp[0]=TIM4CH3_CAPTURE_STA&0X3F; 
			temp[0]*=0XFFFFFFFF;		 		         //溢出时间总和
			temp[0]+=TIM4CH3_CAPTURE_VAL;		   //得到总的高电平时间
			TIM4CH3_CAPTURE_STA=0;			     //开启下一次捕获
		}
		if(TIM4CH4_CAPTURE_STA&0X80)        //成功捕获到了一次高电平
		{
			temp[1]=TIM4CH4_CAPTURE_STA&0X3F; 
			temp[1]*=0XFFFFFFFF;		 		         //溢出时间总和
			temp[1]+=TIM4CH4_CAPTURE_VAL;		   //得到总的高电平时间

			TIM4CH4_CAPTURE_STA=0;			     //开启下一次捕获
		}
		if(temp[0]>2050||temp[0]<1000)
			temp[0]=temp_f[0];
		if(temp[1]>2050||temp[1]<1000)
			temp[1]=temp_f[1];
		if(temp[2]>2050||temp[2]<1000)
			temp[2]=temp_f[2];
		if(temp[3]>2050||temp[3]<1000)
			temp[3]=temp_f[3];
		temp_f[0]=temp[0];
		temp_f[1]=temp[1];
		temp_f[2]=temp[2];
		temp_f[3]=temp[3];
}




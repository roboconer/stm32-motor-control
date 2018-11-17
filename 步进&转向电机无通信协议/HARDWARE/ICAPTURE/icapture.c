#include "icapture.h"
#include "led.h"
#include "usart.h"
//#include "includes.h"


//��ʱ��5ͨ��1���벶������
//arr���Զ���װֵ(TIM2,TIM5��32λ��!!)
//psc��ʱ��Ԥ��Ƶ��
void TIM4_Cap_Init(u16 arr,u16 psc)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	TIM_ICInitTypeDef  TIM4_ICInitStructure;

	
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4,ENABLE);  	//TIM4ʱ��ʹ��    
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD, ENABLE); 	//ʹ��PORTAʱ��	
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12|GPIO_Pin_13|GPIO_Pin_14|GPIO_Pin_15; //GPIOA0
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;//���ù���
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;	//�ٶ�100MHz
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP; //���츴�����
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_DOWN; //����
	GPIO_Init(GPIOD,&GPIO_InitStructure); //��ʼ��PA0

	GPIO_PinAFConfig(GPIOD,GPIO_PinSource12,GPIO_AF_TIM4); //PA0����λ��ʱ��5
	GPIO_PinAFConfig(GPIOD,GPIO_PinSource13,GPIO_AF_TIM4);
	GPIO_PinAFConfig(GPIOD,GPIO_PinSource14,GPIO_AF_TIM4);
	GPIO_PinAFConfig(GPIOD,GPIO_PinSource15,GPIO_AF_TIM4);
  
	TIM_TimeBaseStructure.TIM_Prescaler=psc;  //��ʱ����Ƶ
	TIM_TimeBaseStructure.TIM_CounterMode=TIM_CounterMode_Up; //���ϼ���ģʽ
	TIM_TimeBaseStructure.TIM_Period=arr;   //�Զ���װ��ֵ
	TIM_TimeBaseStructure.TIM_ClockDivision=TIM_CKD_DIV1; 
	
	TIM_TimeBaseInit(TIM4,&TIM_TimeBaseStructure);
	

	//��ʼ��TIM4���벶�����
	TIM4_ICInitStructure.TIM_Channel = TIM_Channel_1; //CC1S=01 	ѡ������� IC1ӳ�䵽TI1��
  TIM4_ICInitStructure.TIM_ICPolarity = TIM_ICPolarity_Rising;	//�����ز���
  TIM4_ICInitStructure.TIM_ICSelection = TIM_ICSelection_DirectTI; //ӳ�䵽TI1��
  TIM4_ICInitStructure.TIM_ICPrescaler = TIM_ICPSC_DIV1;	 //���������Ƶ,����Ƶ 
  TIM4_ICInitStructure.TIM_ICFilter = 0x00;//IC1F=0000 ���������˲��� ���˲�
  TIM_ICInit(TIM4, &TIM4_ICInitStructure);
	TIM4_ICInitStructure.TIM_Channel = TIM_Channel_2;
	TIM_ICInit(TIM4, &TIM4_ICInitStructure);
	TIM4_ICInitStructure.TIM_Channel = TIM_Channel_3;
	TIM_ICInit(TIM4, &TIM4_ICInitStructure);
	TIM4_ICInitStructure.TIM_Channel = TIM_Channel_4;
	TIM_ICInit(TIM4, &TIM4_ICInitStructure);

  NVIC_InitStructure.NVIC_IRQChannel = TIM4_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=2;//��ռ���ȼ�3
	NVIC_InitStructure.NVIC_IRQChannelSubPriority =0;		//�����ȼ�3
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			//IRQͨ��ʹ��
	NVIC_Init(&NVIC_InitStructure);	//����ָ���Ĳ�����ʼ��VIC�Ĵ�����
	
	
	TIM_ITConfig(TIM4,TIM_IT_Update|TIM_IT_CC1|TIM_IT_CC2|TIM_IT_CC3|TIM_IT_CC4 ,ENABLE);//��������ж� ,����CC1IE�����ж�	    
	
	TIM_Cmd(TIM4,ENABLE ); 	//ʹ�ܶ�ʱ��5
	
}
//����״̬
//[7]:0,û�гɹ��Ĳ���;1,�ɹ�����һ��.
//[6]:0,��û���񵽵͵�ƽ;1,�Ѿ����񵽵͵�ƽ��.
//[5:0]:����͵�ƽ������Ĵ���(����32λ��ʱ����˵,1us��������1,���ʱ��:4294��)
u8  TIM4CH1_CAPTURE_STA=0,TIM4CH2_CAPTURE_STA = 0,TIM4CH3_CAPTURE_STA = 0,TIM4CH4_CAPTURE_STA = 0;	//���벶��״̬		    				
u32	TIM4CH1_CAPTURE_VAL,TIM4CH2_CAPTURE_VAL,TIM4CH3_CAPTURE_VAL,TIM4CH4_CAPTURE_VAL;	//���벶��ֵ(TIM2/TIM4��32λ)
//��ʱ��4�жϷ������	 
void TIM4_IRQHandler(void)
{ 		    
	//OSIntEnter();
 	if((TIM4CH1_CAPTURE_STA&0X80)==0)//��δ�ɹ�����	
	{	  
		if (TIM_GetITStatus(TIM4, TIM_IT_Update) != RESET)
		 
		{	    
			if(TIM4CH1_CAPTURE_STA&0X40)//�Ѿ����񵽸ߵ�ƽ��
			{
				if((TIM4CH1_CAPTURE_STA&0X3F)==0X3F)//�ߵ�ƽ̫����
				{
					TIM4CH1_CAPTURE_STA|=0X80;//��ǳɹ�������һ��
					TIM4CH1_CAPTURE_VAL=0XFFFF;
				}else TIM4CH1_CAPTURE_STA++;
			}	 
		}
	if (TIM_GetITStatus(TIM4, TIM_IT_CC1) != RESET)//����1���������¼�
		{	
			if(TIM4CH1_CAPTURE_STA&0X40)		//����һ���½��� 		
			{	  			
				TIM4CH1_CAPTURE_STA|=0X80;		//��ǳɹ�����һ��������
				TIM4CH1_CAPTURE_VAL=TIM_GetCapture1(TIM4);
		   		TIM_OC1PolarityConfig(TIM4,TIM_ICPolarity_Rising); //CC1P=0 ����Ϊ�����ز���
			}else  								//��δ��ʼ,��һ�β���������
			{
				TIM4CH1_CAPTURE_STA=0;			//���
				TIM4CH1_CAPTURE_VAL=0;
	 			TIM_SetCounter(TIM4,0);
				TIM4CH1_CAPTURE_STA|=0X40;		//��ǲ�����������
		   		TIM_OC1PolarityConfig(TIM4,TIM_ICPolarity_Falling);		//CC1P=1 ����Ϊ�½��ز���
			}		    
		}			     	    					   
 	}
	//////////////////////////////////////////CH2/////////////////////////////////////////////////////////
	
	if((TIM4CH2_CAPTURE_STA&0X80)==0)//��δ�ɹ�����	
	{	  
		if (TIM_GetITStatus(TIM4, TIM_IT_Update) != RESET)
		 
		{	    
			if(TIM4CH2_CAPTURE_STA&0X40)//�Ѿ����񵽸ߵ�ƽ��
			{
				if((TIM4CH2_CAPTURE_STA&0X3F)==0X3F)//�ߵ�ƽ̫����
				{
					TIM4CH2_CAPTURE_STA|=0X80;//��ǳɹ�������һ��
					TIM4CH2_CAPTURE_VAL=0XFFFF;
				}else TIM4CH2_CAPTURE_STA++;
			}	 
		}
	if (TIM_GetITStatus(TIM4, TIM_IT_CC2) != RESET)//����2���������¼�
		{	
			if(TIM4CH2_CAPTURE_STA&0X40)		//����һ���½��� 		
			{	  			
				TIM4CH2_CAPTURE_STA|=0X80;		//��ǳɹ�����һ��������
				TIM4CH2_CAPTURE_VAL=TIM_GetCapture2(TIM4);
		   		TIM_OC2PolarityConfig(TIM4,TIM_ICPolarity_Rising); //CC2P=0 ����Ϊ�����ز���
			}else  								//��δ��ʼ,��һ�β���������
			{
				TIM4CH2_CAPTURE_STA=0;			//���
				TIM4CH2_CAPTURE_VAL=0;
	 			TIM_SetCounter(TIM4,0);
				TIM4CH2_CAPTURE_STA|=0X40;		//��ǲ�����������
		   		TIM_OC2PolarityConfig(TIM4,TIM_ICPolarity_Falling);		//CC2P=2 ����Ϊ�½��ز���
			}		    
		}			     	    					   
 	}
	
	////////////////////////////////////////CH3/////////////////////////////////////////////////////////
	if((TIM4CH3_CAPTURE_STA&0X80)==0)//��δ�ɹ�����	
	{	  
		if (TIM_GetITStatus(TIM4, TIM_IT_Update) != RESET)
		 
		{	    
			if(TIM4CH3_CAPTURE_STA&0X40)//�Ѿ����񵽸ߵ�ƽ��
			{
				if((TIM4CH3_CAPTURE_STA&0X3F)==0X3F)//�ߵ�ƽ̫����
				{
					TIM4CH3_CAPTURE_STA|=0X80;//��ǳɹ�������һ��
					TIM4CH3_CAPTURE_VAL=0XFFFF;
				}else TIM4CH3_CAPTURE_STA++;
			}	 
		}
	if (TIM_GetITStatus(TIM4, TIM_IT_CC3) != RESET)//����3���������¼�
		{	
			if(TIM4CH3_CAPTURE_STA&0X40)		//����һ���½��� 		
			{	  			
				TIM4CH3_CAPTURE_STA|=0X80;		//��ǳɹ�����һ��������
				TIM4CH3_CAPTURE_VAL=TIM_GetCapture3(TIM4);
		   		TIM_OC3PolarityConfig(TIM4,TIM_ICPolarity_Rising); //CC3P=0 ����Ϊ�����ز���
			}else  								//��δ��ʼ,��һ�β���������
			{
				TIM4CH3_CAPTURE_STA=0;			//���
				TIM4CH3_CAPTURE_VAL=0;
	 			TIM_SetCounter(TIM4,0);
				TIM4CH3_CAPTURE_STA|=0X40;		//��ǲ�����������
		   		TIM_OC3PolarityConfig(TIM4,TIM_ICPolarity_Falling);		//CC3P=3 ����Ϊ�½��ز���
			}		    
		}			     	  
	
 	}
//	////////////////////////////////////////CH4////////////////////////////////////////////////////////
	if((TIM4CH4_CAPTURE_STA&0X80)==0)//��δ�ɹ�����	
	{	  
		if (TIM_GetITStatus(TIM4, TIM_IT_Update) != RESET)
		 
		{	    
			if(TIM4CH4_CAPTURE_STA&0X40)//�Ѿ����񵽸ߵ�ƽ��
			{
				if((TIM4CH4_CAPTURE_STA&0X3F)==0X3F)//�ߵ�ƽ̫����
				{
					TIM4CH4_CAPTURE_STA|=0X80;//��ǳɹ�������һ��
					TIM4CH4_CAPTURE_VAL=0XFFFF;
				}else TIM4CH4_CAPTURE_STA++;
			}	 
		}
	if (TIM_GetITStatus(TIM4, TIM_IT_CC4) != RESET)//����3���������¼�
		{	
			if(TIM4CH4_CAPTURE_STA&0X40)		//����һ���½��� 		
			{	  			
				TIM4CH4_CAPTURE_STA|=0X80;		//��ǳɹ�����һ��������
				TIM4CH4_CAPTURE_VAL=TIM_GetCapture4(TIM4);
		   		TIM_OC4PolarityConfig(TIM4,TIM_ICPolarity_Rising); //CC4P=0 ����Ϊ�����ز���
			}else  								//��δ��ʼ,��һ�β���������
			{
				TIM4CH4_CAPTURE_STA=0;			//���
				TIM4CH4_CAPTURE_VAL=0;
	 			TIM_SetCounter(TIM4,0);
				TIM4CH4_CAPTURE_STA|=0X40;		//��ǲ�����������
		   		TIM_OC4PolarityConfig(TIM4,TIM_ICPolarity_Falling);		//CC4P=3 ����Ϊ�½��ز���
			}		    
		}			     	    					   
 	}
	TIM_ClearITPendingBit(TIM4,TIM_IT_CC2|TIM_IT_CC3|TIM_IT_CC4|TIM_IT_CC1|TIM_IT_Update); //����жϱ�־λ
	//OSIntExit();		
}

void input_capture(u16 temp[4]){
static u16 temp_f[4]={1517,1517,1517,1517};
		temp[0] = 0;
		temp[1] = 0;
		temp[2] = 0;
		temp[3] = 0;

	 		if(TIM4CH1_CAPTURE_STA&0X80)        //�ɹ�������һ�θߵ�ƽ
		{
			temp[2]=TIM4CH1_CAPTURE_STA&0X3F; 
			temp[2]*=0XFFFFFFFF;		 		         //���ʱ���ܺ�
			temp[2]+=TIM4CH1_CAPTURE_VAL;		   //�õ��ܵĸߵ�ƽʱ��
			TIM4CH1_CAPTURE_STA=0;			     //������һ�β���
		}
		if(TIM4CH2_CAPTURE_STA&0X80)        //�ɹ�������һ�θߵ�ƽ
		{
			temp[3]=TIM4CH2_CAPTURE_STA&0X3F; 
			temp[3]*=0XFFFFFFFF;		 		         //���ʱ���ܺ�
			temp[3]+=TIM4CH2_CAPTURE_VAL;		   //�õ��ܵĸߵ�ƽʱ��
			TIM4CH2_CAPTURE_STA=0;			     //������һ�β���
		}
		if(TIM4CH3_CAPTURE_STA&0X80)        //�ɹ�������һ�θߵ�ƽ
		{
			temp[0]=TIM4CH3_CAPTURE_STA&0X3F; 
			temp[0]*=0XFFFFFFFF;		 		         //���ʱ���ܺ�
			temp[0]+=TIM4CH3_CAPTURE_VAL;		   //�õ��ܵĸߵ�ƽʱ��
			TIM4CH3_CAPTURE_STA=0;			     //������һ�β���
		}
		if(TIM4CH4_CAPTURE_STA&0X80)        //�ɹ�������һ�θߵ�ƽ
		{
			temp[1]=TIM4CH4_CAPTURE_STA&0X3F; 
			temp[1]*=0XFFFFFFFF;		 		         //���ʱ���ܺ�
			temp[1]+=TIM4CH4_CAPTURE_VAL;		   //�õ��ܵĸߵ�ƽʱ��

			TIM4CH4_CAPTURE_STA=0;			     //������һ�β���
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




#include "pwm.h"

GPIO_InitTypeDef GPIO_InitStructure;
NVIC_InitTypeDef NVIC_InitStructure;
TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
TIM_OCInitTypeDef  TIM_OCInitStructure;
USART_InitTypeDef  USART_InitStructure;

static uint32_t g_pwm_cnt=0;

#define SG		PBout(7)



void sg_init(void)
{
	//ʹ�ܶ˿�Bʱ��
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);
	
	/* TIM4 clock enable ����ʱ��14��ʱ��ʹ��*/
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4, ENABLE);
	
	/* ����PB7 ΪPWM���ģʽ */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7;					//��7������
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;				//����Ϊ���ù���ģʽ
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;				//����ģʽ��������������
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;			//����IO���ٶ�Ϊ100MHz��Ƶ��Խ������Խ�ã�Ƶ��Խ�ͣ�����Խ��
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;			//����Ҫ��������
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	
	GPIO_PinAFConfig(GPIOB, GPIO_PinSource7, GPIO_AF_TIM4);

	/* Time base configuration����ʱ���Ļ������ã��������ö�ʱ������������Ƶ��Ϊ50Hz */
	TIM_TimeBaseStructure.TIM_Period = 10000/50;						//���ö�ʱ��Ƶ��Ϊ50Hz
	
	g_pwm_cnt=TIM_TimeBaseStructure.TIM_Period;
	
	TIM_TimeBaseStructure.TIM_Prescaler = 8400-1;						//��һ�η�Ƶ�����ΪԤ��Ƶ
	TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;				//�ڶ��η�Ƶ,��ǰʵ��1��Ƶ��Ҳ���ǲ���Ƶ
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
	
	TIM_TimeBaseInit(TIM4, &TIM_TimeBaseStructure);


	/* PWM1 Mode configuration: Channel2 ����PWM��ͨ��2������ģʽ1*/
	TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;				//PWMģʽ1���ڵ���ģʽ�£�ֻҪ TIMx_CNT < TIMx_CCR1��ͨ�� 1 ��Ϊ��Ч״̬���ߵ�ƽ��������Ϊ��Ч״̬���͵�ƽ����

	TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;	//�������

	TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High;		//��Ч��ʱ������ߵ�ƽ

	TIM_OC2Init(TIM4, &TIM_OCInitStructure);						//��ʱ��4ͨ��2��ʼ��

	TIM_OC2PreloadConfig(TIM4, TIM_OCPreload_Enable);				//��ʱ��ͨ��2�Զ����س�ֵ���������PWM����

	TIM_ARRPreloadConfig(TIM4, ENABLE);							//�Զ����س�ֵʹ��

	/* TIM4 enable counter��ʹ�ܶ�ʱ��4���� */
	TIM_Cmd(TIM4, ENABLE);
}

void sg_angle(uint32_t angle)
{
	if(angle==0)
		TIM_SetCompare2(TIM4,(uint32_t)(0.5 * g_pwm_cnt/20));

	if(angle==45)
		TIM_SetCompare2(TIM4,(uint32_t)(g_pwm_cnt/20));


	if(angle==90)
		TIM_SetCompare2(TIM4,(uint32_t)(1.5*g_pwm_cnt/20));
	
	if(angle==135)
		TIM_SetCompare2(TIM4,(uint32_t)(2*g_pwm_cnt/20));
	
	if(angle==180)
		TIM_SetCompare2(TIM4,(uint32_t)(2.5*g_pwm_cnt/20));		
}




//void sg_init(void)
//{

//	/* GPIOBʱ��*/
//	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);

//	
//	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7;					//��8������
//	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;				//����Ϊ���ģʽ
//	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;				//����ģʽ��������������
//	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;			//����IO���ٶ�Ϊ100MHz��Ƶ��Խ������Խ�ã�Ƶ��Խ�ͣ�����Խ��
//	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;			//����Ҫ��������
//	GPIO_Init(GPIOB, &GPIO_InitStructure);
//}

//void sg_angle(uint32_t angle)
//{
//	if(angle==0)
//	{
//		while(1)
//		{
//		SG=1;
//		delay_us(500);
//		
//		SG=0;
//		delay_us(500);
//		delay_ms(19);
//		
//		}

//	}

//	if(angle==45)
//	{
//		SG=1;
//		delay_ms(1);
//		
//		SG=0;
//		delay_ms(19);
//	}


//	if(angle==90)
//	{
//		SG=1;
//		delay_ms(1);
//		delay_us(500);
//		
//		SG=0;
//		delay_ms(18);
//		delay_us(500);
//	}
//	
//	if(angle==135)
//	{
//		SG=1;
//		delay_ms(2);
//		
//		SG=0;
//		delay_ms(18);
//	}	
//	
//	if(angle==180)
//	{
//		while(1)
//		{
//		SG=1;
//		delay_ms(2);
//		delay_us(500);
//		
//		SG=0;
//		delay_ms(17);
//		delay_us(500);
//		
//		}
//	}		
//}



#include "includes.h"

/*------------------------------------����ռ�����ʼ��----------------------------------------*/
/*������ƿ�ָ��*/
static TaskHandle_t app_task_init_handle      = NULL;			
static TaskHandle_t g_app_task_key_handle     = NULL;
static TaskHandle_t g_app_task_mqtt_handle 	  = NULL;
static TaskHandle_t g_app_task_esp8266_handle = NULL;
static TaskHandle_t g_app_task_monitor_handle = NULL;
static TaskHandle_t g_app_task_oled_handle    = NULL;
static TaskHandle_t g_app_task_steer_handle   = NULL;
static TaskHandle_t g_app_task_key16_handle   = NULL;
static TaskHandle_t g_app_task_finger_handle  = NULL;
static TaskHandle_t g_app_task_flash_handle   = NULL; 
 
/* ����1:��ʼ�� */ 
static void app_task_init(void* pvParameters);  

/* ����2:���� */  
static void app_task_key(void* pvParameters); 

/* ����3:mqtt���Ʒ��� */  
static void app_task_mqtt(void* pvParameters); 

/* ����4:����WiFiģ��-esp8266*/  
static void app_task_esp8266(void* pvParameters); 

/* ����5:�������*/  
static void app_task_monitor(void* pvParameters); 

/* ����6:OLED����*/  
static void app_task_oled(void* pvParameters); 

/* ����7:���������*/  
static void app_task_steer(void* pvParameters); 

/* ����8:�������4x4����*/  
static void app_task_key16(void* pvParameters); 

/* ����9:ָ��ģ������*/  
static void app_task_finger(void* pvParameters); 

/* ����10:flash��д����*/  
static void app_task_flash(void* pvParameters); 


/*-----------------------------------------ȫ�ֱ�����ʼ��-----------------------------------------*/
/* �������ź������ */
static SemaphoreHandle_t g_mutex_printf;

/* �¼���־���� */
EventGroupHandle_t g_event_group;	

/* ��Ϣ���о�� */
QueueHandle_t g_queue_esp8266;			

/*�ź���*/
SemaphoreHandle_t g_sem_binary;						

static volatile uint32_t g_esp8266_init=0;			//��ʼ������
volatile float g_temp=0.0;							//�¶�
volatile float g_humi=0.0;							//ʪ�ȱ���
volatile int g_door=0;								//��״̬
volatile int g_flag_connect_server=0;				//���ӷ���������
volatile int g_flag_in=0;							//��¼ģʽ
volatile int g_flag_password_re=0;					//�޸�����
volatile int g_flag_password_look=0;				//�鿴����
uint8_t tmp=0;										//��ʱ��������
char  password[18]={0};							    //���̵�¼����
char flash_buf[18]={0};							    //flash�Ľ���buf


/*----------------------------------------�����ļ���������----------------------------------------*/
/*ͼ��BMP���ֿ�*/
extern unsigned char BMP1[];
extern unsigned char BMP2[];
extern unsigned char BMP3[];
extern unsigned char BMP4[];
extern unsigned char BMP5[];
extern unsigned char BMP6[];
extern unsigned char BMP7[];
extern const unsigned char F6x8[][6];
extern const unsigned char F8X16[];
extern char Hzk[][32];							

/*-------------------------------------------�궨��-----------------------------------------------*/
#define PASSWORD_INIT 				0  				//�����ʼ��
#define DEBUG_dgb_printf_safe_EN	1  				//������ӡ����
#define PASSWORD_ADDR 				0x000000		//����λ��
#define PASSWORD_PRIMARY  			"123456#"		//ԭ������
#define PASSWORD_PASSWORD_LOOK      1				//��λ��ʾ���뿪��	
#define STREE_OPEN                  1               //�������
#define STREE_LED_OPEN				0				//�ƹ⿪��
#define KEY_EXIT					1				//������ʱ�жϿ���	
#define BEER						1				//����������		��������
//configUSE_TIMERS 									/*��ʱ������*/
//configSUPPORT_DYNAMIC_ALLOCATION 


/*-----------------------------------------�ض�����---------------------------------------------*/
void dgb_printf_safe(const char *format, ...)
{
#if DEBUG_dgb_printf_safe_EN	

	va_list args;
	va_start(args, format);
	
	/* ��ȡ�����ź��� */
	xSemaphoreTake(g_mutex_printf,portMAX_DELAY);
	
	vprintf(format, args);
			
	/* �ͷŻ����ź��� */
	xSemaphoreGive(g_mutex_printf);	

	va_end(args);
#else
	(void)0;										//��ֹ����
#endif
}

/*------------------------------------------������------------------------------------------------*/
int main(void)
{
		
	/* �����ź��� */
	vSemaphoreCreateBinary(g_sem_binary);
	
	/* ����ϵͳ�ж����ȼ�����4 */
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);
	
	/* ϵͳ��ʱ���ж�Ƶ��ΪconfigTICK_RATE_HZ */
	SysTick_Config(SystemCoreClock/configTICK_RATE_HZ);										
	
	/* ��ʼ������1 */
	usart1_init(9600);    

	/* ����app_task_init���� */
	xTaskCreate((TaskFunction_t )app_task_init,  		/* ������ں��� */
			  (const char*    )"app_task_init",			/* �������� */
			  (uint16_t       )512,  					/* ����ջ��С */
			  (void*          )NULL,					/* ������ں������� */
			  (UBaseType_t    )5, 						/* ��������ȼ� */
			  (TaskHandle_t*  )&app_task_init_handle);	/* ������ƿ�ָ�� */ 

	
	/* ����������� */
	vTaskStartScheduler(); 
	
	while(1);
}


/*----------------------------------------���񴴽�����--------------------------------------------*/
static void app_task_init(void* pvParameters)
{
	/* led��ʼ�� */
	led_init();
	
	#if	BEER
	/* ��������ʼ�� */	
	beep_init();
	#endif
	
	/* ������ʼ�� */
	key_init();
	
	/*��ʼ��OLED*/
	OLED_Init();
	
	/*��ʼ����ʪ��*/
	dht11_init();
	
	/*pwm��ʼ��*/
	sg_init();
	
	/*��ʼ������*/
	key_board_init();
	
	/*��ʼ����ʪ��*/
	dht11_init();
	
	/*FLASH��ʼ��*/
	Flash_config();
	
	/*�������Ź� */
	IWDG_Start(); 

	/* ���������� */	  
	g_mutex_printf=xSemaphoreCreateMutex();
				  
	/* �����¼���־�� */
	g_event_group=xEventGroupCreate();

	/* ������Ϣ���� */
	g_queue_esp8266=xQueueCreate(3, sizeof(g_esp8266_rx_buf));	
	
	/* ����app_task_key���� */		  
	xTaskCreate((TaskFunction_t )app_task_key,  		/* ������ں��� */
			  (const char*    )"app_task_key",			/* �������� */
			  (uint16_t       )128,  					/* ����ջ��С */
			  (void*          )NULL,					/* ������ں������� */
			  (UBaseType_t    )5, 						/* ��������ȼ� */
			  (TaskHandle_t*  )&g_app_task_key_handle);	/* ������ƿ�ָ�� */
			  
	/* ����app_task_mqtt���� */		  
	xTaskCreate((TaskFunction_t )app_task_mqtt,  		/* ������ں��� */
			  (const char*    )"app_task_mqtt",		/* �������� */
			  (uint16_t       )512,  					/* ����ջ��С */
			  (void*          )NULL,					/* ������ں������� */
			  (UBaseType_t    )5, 						/* ��������ȼ� */
			  (TaskHandle_t*  )&g_app_task_mqtt_handle);	/* ������ƿ�ָ�� */	

	/* ����app_task_oled���� */		  
	xTaskCreate((TaskFunction_t )app_task_oled,  		/* ������ں��� */
			  (const char*    )"app_task_oled",			/* �������� */
			  (uint16_t       )128,  						/* ����ջ��С */
			  (void*          )NULL,						/* ������ں������� */
			  (UBaseType_t    )5, 							/* ��������ȼ� */
			  (TaskHandle_t*  )&g_app_task_oled_handle);		/* ������ƿ�ָ�� */			  	  
			  
	/* ����app_task_esp8266���� */		  
	xTaskCreate((TaskFunction_t )app_task_esp8266,  		/* ������ں��� */
			  (const char*    )"app_task_esp8266",		/* �������� */
			  (uint16_t       )1024,  					/* ����ջ��С */
			  (void*          )NULL,					/* ������ں������� */
			  (UBaseType_t    )5, 						/* ��������ȼ� */
			  (TaskHandle_t*  )&g_app_task_esp8266_handle);	/* ������ƿ�ָ�� */				  
			  
	/* ����app_task_monitor���� */		  
	xTaskCreate((TaskFunction_t )app_task_monitor,  		/* ������ں��� */
			  (const char*    )"app_task_monitor",			/* �������� */
			  (uint16_t       )512, 						/* ����ջ��С */
			  (void*          )NULL,						/* ������ں������� */
			  (UBaseType_t    )5, 							/* ��������ȼ� */
			  (TaskHandle_t*  )&g_app_task_monitor_handle);		/* ������ƿ�ָ�� */	

	/* ����app_task_steer��ι������ */		  
	xTaskCreate((TaskFunction_t )app_task_steer,  		/* ������ں��� */
			  (const char*    )"app_task_steer",			/* �������� */
			  (uint16_t       )128,  						/* ����ջ��С */
			  (void*          )NULL,						/* ������ں������� */
			  (UBaseType_t    )10, 							/* ��������ȼ� */			//��Ϊ����ι�����������ȼ���һ�㣬һ�д��벻�ÿ�������
			  (TaskHandle_t*  )&g_app_task_steer_handle);		/* ������ƿ�ָ�� */	
	  		  
	/* ����app_task_key16���� */		  
	xTaskCreate((TaskFunction_t )app_task_key16,  		/* ������ں��� */
			  (const char*    )"app_task_key16",			/* �������� */
			  (uint16_t       )128,  						/* ����ջ��С */
			  (void*          )NULL,						/* ������ں������� */
			  (UBaseType_t    )5, 							/* ��������ȼ� */
			  (TaskHandle_t*  )&g_app_task_key16_handle);		/* ������ƿ�ָ�� */	
			  
	/* ����app_task_finger���� */		  
	xTaskCreate((TaskFunction_t )app_task_finger,  		/* ������ں��� */
			  (const char*    )"app_task_finger",			/* �������� */
			  (uint16_t       )128,  						/* ����ջ��С */
			  (void*          )NULL,						/* ������ں������� */
			  (UBaseType_t    )5, 							/* ��������ȼ� */
			  (TaskHandle_t*  )&g_app_task_finger_handle);		/* ������ƿ�ָ�� */	
	
	/* ����app_task_flash���� */		  
	xTaskCreate((TaskFunction_t )app_task_flash,  		/* ������ں��� */
			  (const char*    )"app_task_flash",			/* �������� */
			  (uint16_t       )128,  						/* ����ջ��С */
			  (void*          )NULL,						/* ������ں������� */
			  (UBaseType_t    )5, 							/* ��������ȼ� */
			  (TaskHandle_t*  )&g_app_task_flash_handle);		/* ������ƿ�ָ�� */	
			  
	vTaskDelete(NULL);		  
}   



/*----------------------------------------����������--------------------------------------------*/
static void app_task_key(void* pvParameters)
{
	EventBits_t EventValue=0;								//��ֵ
	
	dgb_printf_safe("app_task_key create success\r\n");		

	for(;;)
	{
		#if KEY_EXIT	
		//�ȴ��¼����е���Ӧ�¼�λ����ͬ��
		EventValue=xEventGroupWaitBits((EventGroupHandle_t	)g_event_group,		//ȷ����������ͬʱ����
									   (EventBits_t			)0x0F,
									   (BaseType_t			)pdTRUE,				
									   (BaseType_t			)pdFALSE,
									   (TickType_t			)portMAX_DELAY);
		//��ʱ����
		vTaskDelay(50);		
		if(EventValue & EVENT_GROUP_KEY1_DOWN)
		{
			//��ֹEXTI0�����ж�
			NVIC_DisableIRQ(EXTI0_IRQn);					//ȷ���ж�ִ�в������
			
			//ȷ���ǰ���
			if(PAin(0) == 0)
			{
				dgb_printf_safe("S1 Press\r\n");
				#if	BEER
				beep_on();  PFout(9)=0;delay_ms(100);		//������Чʱ����
				beep_off(); PFout(9)=1;
				#endif
				//xSemaphoreTake(g_sem_binary,portMAX_DELAY);
				g_flag_in = 0;								//�޸���״̬�͵�¼��ʽ
				g_door=0;
				//xSemaphoreGive(g_sem_binary);
				//�ȴ������ͷ�
				while(PAin(0)==0)
					vTaskDelay(20);	
				
			}
				
			//����EXTI0�����ж�
			NVIC_EnableIRQ(EXTI0_IRQn);						//����жϱ���
		}
		
		if(EventValue & EVENT_GROUP_KEY2_DOWN)
		{
			//��ֹEXTI2�����ж�
			NVIC_DisableIRQ(EXTI2_IRQn);

				
			if(PEin(2) == 0)
			{
				dgb_printf_safe("S2 Press\r\n");
				#if	BEER
				beep_on();  PFout(9)=0;delay_ms(100);		//������Чʱ����
				beep_off(); PFout(9)=1;
				#endif
				//xSemaphoreTake(g_sem_binary,portMAX_DELAY);
				g_flag_in = 1;
				PAout(4)=1;
				//g_door=0;
				//xSemaphoreGive(g_sem_binary);
				//�ȴ������ͷ�
				while(PEin(2)==0)
					vTaskDelay(20);
			}

			//����EXTI2�����ж�
			NVIC_EnableIRQ(EXTI2_IRQn);	
		}	
		
		if(EventValue & EVENT_GROUP_KEY3_DOWN)
		{
			//��ֹEXTI3�����ж�
			NVIC_DisableIRQ(EXTI3_IRQn);
			
				
			if(PEin(3) == 0)	
			{
				dgb_printf_safe("S3 Press\r\n");
				#if	BEER
				beep_on();  PFout(9)=0;delay_ms(100);		//������Чʱ����
				beep_off(); PFout(9)=1;
				#endif
				//xSemaphoreTake(g_sem_binary,portMAX_DELAY);
				g_flag_in = 2;
				g_flag_password_re=1;
				//g_door=1;
				//xSemaphoreGive(g_sem_binary);	
				//�ȴ������ͷ�
				while(PEin(3)==0)
					vTaskDelay(20);
			}
				
			//����EXTI3�����ж�
			NVIC_EnableIRQ(EXTI3_IRQn);	
		}
		
		if(EventValue & EVENT_GROUP_KEY4_DOWN)
		{
			//��ֹEXTI4�����ж�
			NVIC_DisableIRQ(EXTI4_IRQn);
				
			if(PEin(4) == 0)	
			{
				dgb_printf_safe("S4 Press\r\n");
				#if	BEER
				beep_on();  PFout(9)=0;delay_ms(100);		//������Чʱ����
				beep_off(); PFout(9)=1;
				#endif				
				//xSemaphoreTake(g_sem_binary,portMAX_DELAY);
				g_flag_in = 3;
				g_flag_password_look=1;
				//xSemaphoreGive(g_sem_binary);
				//�ȴ������ͷ�
				while(PEin(4)==0)
					vTaskDelay(20);	
			}
			//����EXTI4�����ж�
			NVIC_EnableIRQ(EXTI4_IRQn);	
		}
		#else
			vTaskDelay(1000);	
		#endif
	}
} 

static void app_task_mqtt(void* pvParameters)
{
	uint32_t 	delay_1s_cnt=0;									//ʱ�����
	uint8_t		buf[5]={20,05,56,8,20};							//��������
	
	dgb_printf_safe("app_task_mqtt create success\r\n");		//��ʾ����״̬
	
	dgb_printf_safe("app_task_mqtt suspend\r\n");

	vTaskSuspend(NULL);
	
	dgb_printf_safe("app_task_mqtt resume\r\n");
	
	vTaskDelay(1000);
	
	for(;;)
	{
		//����������
		mqtt_send_heart();							//��ֹ����
		
		//�ϱ��豸״̬
		mqtt_report_devices_status();				//���ϻ㱨
		
		delay_ms(1000);								//һ��һ��
		
		delay_1s_cnt++;							
		
			
		if(delay_1s_cnt >= 3 )						//����һ��
		{	
			delay_1s_cnt=0;

			if(0 == dht11_read(buf))				//�������ȷ
			{
			
				g_temp = (float)buf[2]+(float)buf[3]/10;		//��ȡ�������ݣ��޸�ȫ�ֱ����¶Ⱥ�ʪ�ȣ���Ϊû������λ�ø�ֵ������Ҫ����
				g_humi = (float)buf[0]+(float)buf[1]/10;
			
				printf("Temperature=%.1f Humidity=%.1f\r\n",g_temp,g_humi);
			}

		}
		 
	}
}

static void app_task_monitor(void* pvParameters)
{
	uint32_t esp8266_rx_cnt=0;
	
	BaseType_t xReturn = pdFALSE;					//��ʼ����Ϣ����
	
	dgb_printf_safe("app_task_monitor create success \r\n");
	
	for(;;)
	{	
		esp8266_rx_cnt = g_esp8266_rx_cnt;
		
		delay_ms(10);
		
		/* n����󣬷���g_esp8266_rx_cntû�б仯������Ϊ�������ݽ��� */
		if(g_esp8266_init && esp8266_rx_cnt && (esp8266_rx_cnt == g_esp8266_rx_cnt))
		{
			/* ������Ϣ������������ˣ���ʱʱ��Ϊ1000�����ģ����1000�����Ķ�����ʧ�ܣ�����ֱ�ӷ��� */
			xReturn = xQueueSend(g_queue_esp8266,(void *)g_esp8266_rx_buf,1000);		
			
			if (xReturn != pdPASS)
				dgb_printf_safe("[app_task_monitor] xQueueSend g_queue_esp8266 error code is %d\r\n", xReturn);
			
			g_esp8266_rx_cnt=0;
			memset((void *)g_esp8266_rx_buf,0,sizeof(g_esp8266_rx_buf));
		
		}	
	}
}

static void app_task_esp8266(void* pvParameters)
{
	
	/*�����˰�����ƽ̨������ϴ�����Ҳ��������ô����Ҫ�޸��ж��߼�����Ϊid�����������Ҳ���̶ܹ�*/
	
	uint8_t buf[512];								//��ʼ�����տռ�
	BaseType_t xReturn = pdFALSE;					//��Ϣ���г�ʼ��
	uint32_t i;										//ѭ��������ʼ��
	
	dgb_printf_safe("app_task_esp8266 create success\r\n");
	
	while(esp8266_mqtt_init())
	{
		dgb_printf_safe("esp8266_mqtt_init ...");
		
		delay_ms(1000);
	}
	
	#if BEER
	//��������������D1����˸���Σ�ʾ�����ӳɹ�
	beep_on();  PFout(9)=0;delay_ms(100);
	beep_off(); PFout(9)=1;delay_ms(100);	
	beep_on();  PFout(9)=0;delay_ms(100);
	beep_off(); PFout(9)=1;delay_ms(100);
	#endif
	
	//g_flag_connect_server=1;
	
	printf("esp8266 connect aliyun with mqtt success\r\n");	
	
	vTaskResume(g_app_task_mqtt_handle);
	
	g_esp8266_init=1;
	
	for(;;)
	{	
		xReturn = xQueueReceive(g_queue_esp8266,	/* ��Ϣ���еľ�� */
								buf,				/* �õ�����Ϣ���� */
								portMAX_DELAY); 	/* �ȴ�ʱ��һֱ�� */
		if (xReturn != pdPASS)
		{
			dgb_printf_safe("[app_task_esp8266] xQueueReceive error code is %d\r\n", xReturn);
			continue;
		}	

		for(i=0;i<sizeof(buf);i++)
		{
			//�жϵĹؼ��ַ��Ƿ�Ϊ 1"
			//�������ݣ���{"switch_led_1":1}�еġ�1��
			if((buf[i]==0x31) && (buf[i+1]==0x22))
			{
					//�жϿ��Ʊ���
					if( buf[i+3]=='1' )
						PFout(9)=0;//���Ƶ���
					else
						PFout(9)=1;//���Ƶ���
			}	

			//�жϵĹؼ��ַ��Ƿ�Ϊ 2"
			//�������ݣ���{"switch_led_2":1}�еġ�1��
			if((buf[i]==0x32) && (buf[i+1]==0x22))
			{
					//�жϿ��Ʊ���
					if( buf[i+3]=='1' )
						PFout(10)=0;//���Ƶ���
					else
						PFout(10)=1;//���Ƶ���
			}

			//�жϵĹؼ��ַ��Ƿ�Ϊ 3"
			//�������ݣ���{"switch_led_3":1}�еġ�1��
			if(buf[i]==0x33 && buf[i+1]==0x22)
			{
					//�жϿ��Ʊ���
					if( buf[i+3]=='1' )
						PEout(13)=0;//���Ƶ���
					else
						PEout(13)=1;//���Ƶ���
			}	

			//�жϵĹؼ��ַ��Ƿ�Ϊ 4"
			//�������ݣ���{"switch_led_4":1}�еġ�1��
			if(buf[i]==0x34 && buf[i+1]==0x22)
			{
					//�жϿ��Ʊ���
					if( buf[i+3]=='1' )
						PEout(14)=0;//���Ƶ���
					else
						PEout(14)=1;//���Ƶ���
			}	

			
			//�жϵĹؼ��ַ��Ƿ�Ϊ 5"
			//�������ݣ���{"switch_door_5":1}�еġ�1��
			if(buf[i]==0x35 && buf[i+1]==0x22)
			{
					//�жϿ��Ʊ���
					if( buf[i+3]=='1')
						//TIM_SetCompare2(TIM4,(uint32_t)(2.5*180/20));	
						g_door=1;
					else
						//TIM_SetCompare2(TIM4,(uint32_t)(0));		
						g_door=0;
			}	

		}

	}
}

static void app_task_oled(void* pvParameters)
{
	int tmp_flag=0;								//��һ�ε�״̬
	int i = 0;									//���ҹ���
	
	OLED_Clear();					
	OLED_DrawBMP(0,0,56,8,BMP4);				//��ʼ������
	OLED_DrawBMP(64,0,120,8,BMP5);
	for(;;)
	{
		if(tmp_flag!=g_flag_in)					//ģʽ�л���ʱ�������
		{
			if(tmp_flag==0)
				OLED_WR_Byte(0x2E,OLED_CMD);    //�رչ���
			vTaskDelay(10);
			tmp_flag=g_flag_in;					//����״̬
			OLED_Clear();						//����
			vTaskDelay(10);
			if(g_flag_in==0)
			{
				i=!i;						//�л����ҹ���
				OLED_DrawBMP(0,0,56,8,BMP4);		
				OLED_DrawBMP(64,0,120,8,BMP5);
				vTaskDelay(10);
			}
		}

		if(g_flag_in==0)					//ģʽ0
		{
			if(i==1)
			{
				//�������ƶ�
				OLED_WR_Byte(0x2E,OLED_CMD);        //�رչ���
				OLED_WR_Byte(0x26,OLED_CMD);        //ˮƽ��������ҹ��� 26/27
				OLED_WR_Byte(0x00,OLED_CMD);        //�����ֽ�
				OLED_WR_Byte(0x00,OLED_CMD);        //��ʼҳ 0
				OLED_WR_Byte(0x07,OLED_CMD);        //����ʱ����
				OLED_WR_Byte(0x07,OLED_CMD);        //��ֹҳ 7
				OLED_WR_Byte(0x00,OLED_CMD);        //�����ֽ�
				OLED_WR_Byte(0xFF,OLED_CMD);        //�����ֽ�
				OLED_WR_Byte(0x2F,OLED_CMD);        //��������	
			}
			else
			{
						//���ҵ����ƶ�
				OLED_WR_Byte(0x2E,OLED_CMD);        //�رչ���
				OLED_WR_Byte(0x27,OLED_CMD);        //ˮƽ��������ҹ��� 26/27
				OLED_WR_Byte(0x00,OLED_CMD);        //�����ֽ�
				OLED_WR_Byte(0x00,OLED_CMD);        //��ʼҳ 0
				OLED_WR_Byte(0x07,OLED_CMD);        //����ʱ����
				OLED_WR_Byte(0x07,OLED_CMD);        //��ֹҳ 7
				OLED_WR_Byte(0x00,OLED_CMD);        //�����ֽ�
				OLED_WR_Byte(0xFF,OLED_CMD);        //�����ֽ�
				OLED_WR_Byte(0x2F,OLED_CMD);        //��������
			
			
			}
			vTaskDelay(100);
		}
		

		else if(g_flag_in==1)				//ģʽ1
		{
			OLED_DrawBMP(32,0,95,8,BMP6);
			
		}

		else if(g_flag_in==2)				//ģʽ2
		{
			OLED_DrawBMP(32,0,96,8,BMP7);
			
		}

		else if(g_flag_in==3)				//ģʽ3
		{
			OLED_DrawBMP(0,0,128,8,BMP1);
			vTaskDelay(100);
			OLED_DrawBMP(0,0,128,8,BMP2);
							
		}
		vTaskDelay(100);
		OLED_WR_Byte(0x2E,OLED_CMD);        //�رչ���	Ҳ����������Ĺ�����һ��flag,ִֻ��һ�Σ����Ƕ���һ������û��Ҫ,���ӵĻ����ͻ����ͼ���쳣ƽ��.
	}
}


static void app_task_steer(void* pvParameters)
{
	//sg_angle(0);
	int i =0;                  					 //ι��ʱ��
	for(;;)										//ͨ����־λ�����ƶ��
	{
		
#if STREE_OPEN
	
		if(g_door==1)
		{
			sg_angle(0);
			
	#if STREE_LED_OPEN 
			PFout(9)=0;
			PFout(10)=0;
			PEout(13)=0;
			PEout(14)=0;
			//printf("%d\n",g_door);
	#endif
		}
		else if(g_door==0)
		{
			sg_angle(180);
	#if STREE_LED_OPEN 		
			PFout(9)=1;
			PFout(10)=1;
			PEout(13)=1;
			PEout(14)=1;
	#endif
			//printf("%d\n",g_door);			
		}
#endif
		
	delay_ms(100);

		if(i==2)
		{
			
			NVIC_DisableIRQ(EXTI0_IRQn);		//��ֹEXTI0�����ж�
			
			IWDG_ReloadCounter();				//ι�������жϣ�����Ҫ����
			
			NVIC_EnableIRQ(EXTI0_IRQn);			//����EXTI0�����ж�
			
			i=0;								//���¼���
		}
		i++;
	}
}


static void app_task_key16(void* pvParameters)
{
	char data_buf[18]={0};						//��ʱ����õ����¼�ֵ������������
	int i=0;	
	printf("Please enter a password with 18 characters or less\r\n");	//�����±�
	for(;;)
	{

		tmp = get_key_board();					//��ȡ����ֵchar����
		
		if(g_flag_password_re==0)	
		{
			
			if(tmp != 'N')						//��ΪN����Ч
			{
				#if BEER
				beep_on();  PFout(9)=0;delay_ms(100);
				beep_off(); PFout(9)=1;
				#endif
				printf("%c\n",tmp);
				data_buf[i]=tmp;				//��������
				i++;
				if(tmp=='#')
				{
					i=0;
					printf("%s\n\r",data_buf);
					if(strcmp(data_buf,password)==0)
					{
						g_door=1;								//������״̬
						printf("CXNB\r\n");
					}
					if(strcmp(data_buf,(const char*)flash_buf)==0)
					{
						g_door=1;
						printf("CXNB\r\n");
					}					
					memset(data_buf,0,sizeof(data_buf));		//���,������һ���������
				}
			}
		}
		vTaskDelay(200);	
	}	
}

static void app_task_finger(void* pvParameters)
{
	int32_t rt;							//��������ֵ����
	uint32_t key_sta;					//��ֵ���ƹ���ѡ��
	uint16_t id=1;						//�û���
	uint32_t timeout=0;					//��ʱ�������
	uint16_t user_total;				//��ȡ�û���
	uint8_t switch_flag=0;				//���ر�־λ
	
	for(;;)
	{
		if(g_flag_in==1)
		{

			delay_ms(1000);
			
			dgb_printf_safe("ָ�Ƶ�¼\r\n");
			
			/* ������ָ��ģ��������� */
			while(SFM_ACK_SUCCESS!=sfm_init(115200))
			{
				
				delay_ms(500);
			
				dgb_printf_safe("����ָ��ģ�������� ...\r\n");		
			}
			
			dgb_printf_safe("����ָ��ģ����������\r\n");
			
			/* �ɹ����֣���������һ��ʾ�� */
			#if BEER
			beep_on();delay_ms(50);beep_off();
			#endif
			/* ��ȡ�û����� */
			sfm_get_user_total(&user_total);
			
			dgb_printf_safe("����ָ��ģ���û����� = %d \r\n",user_total);
			
			for(;;)
			{
				key_sta=key_sta_get(tmp);
				
				/* ���ָ�� */
				if(key_sta & 0x01)
				{
					dgb_printf_safe("\r\n\r\n=====================================\r\n\r\n");
					dgb_printf_safe("ִ�����ָ�Ʋ���,�뽫��ָ�ŵ�ָ��ģ�鴥����Ӧ��\r\n");
					timeout=0;
					
					/* ��ʾ��ɫ */
					sfm_ctrl_led(0x06,0x06,0x32);
					
					while((sfm_touch_check()!=SFM_ACK_SUCCESS) && (timeout<10))
					{
						timeout++;
					}
					
					if(timeout>=10)
					{
						dgb_printf_safe("û�м�⵽��ָ�������²���!\r\n");
						
						/* �ָ���Ȧȫ��->ȫ������2�� */
						sfm_ctrl_led(0x00,0x07,0xC8);
						
						continue;
					}
					
					dgb_printf_safe("��⵽��ָ�����ڿ�ʼ���ָ��...\r\n");
					
					/* ��ȡδʹ�õ��û�id */
					rt = sfm_get_unused_id(&id);
					
					if(rt != SFM_ACK_SUCCESS)
					{
						dgb_printf_safe("��ȡδʹ�õ��û�id %s\r\n",sfm_error_code(rt));
					
						/* �ָ���Ȧȫ��->ȫ������2�� */
						sfm_ctrl_led(0x00,0x07,0xC8);				
					
						continue;
					
					}
					
					dgb_printf_safe("��ʹ�õ��û�idΪ%d\r\n",id);	
					
					rt=sfm_reg_user(id);
					
					if(rt == SFM_ACK_SUCCESS)
					{
						/* �ɹ�:��Ȧ��ʾ��ɫ */
						sfm_ctrl_led(0x05,0x05,0x32);			
						delay_ms(1000);
						
						/* �ɹ�����������һ��ʾ�� */
						#if BEER
						beep_on();delay_ms(50);beep_off();					
						#endif
					}
					else
					{
						/* ʧ��:��Ȧ��ʾ��ɫ */
						sfm_ctrl_led(0x03,0x03,0x32);
						
						delay_ms(1000);			
					}
					
					
					dgb_printf_safe("���ָ�� %s\r\n",sfm_error_code(rt));
					
					/* �ָ���Ȧȫ��->ȫ������2�� */
					sfm_ctrl_led(0x00,0x07,0xC8);		
				}
				
				/* ˢָ�� */
				if(key_sta & 0x02)
				{
					dgb_printf_safe("\r\n\r\n=====================================\r\n\r\n");
					dgb_printf_safe("ִ��ˢָ�Ʋ���,�뽫��ָ�ŵ�ָ��ģ�鴥����Ӧ��\r\n");
					timeout=0;
					
					/* ��ʾ��ɫ */
					sfm_ctrl_led(0x06,0x06,0x32);
					
					while((sfm_touch_check()!=SFM_ACK_SUCCESS) && (timeout<10))
					{
						timeout++;
					}
					
					if(timeout>=10)
					{
						dgb_printf_safe("û�м�⵽��ָ�������²���!\r\n");
						
						/* �ָ���Ȧȫ��->ȫ������2�� */
						sfm_ctrl_led(0x00,0x07,0xC8);
						
						continue;
					}
					
					dgb_printf_safe("��⵽��ָ�����ڿ�ʼˢָ��...\r\n");
					
					rt=sfm_compare_users(&id);
					
					if(rt == SFM_ACK_SUCCESS)
					{
						/* �ɹ�:��Ȧ��ʾ��ɫ */
						sfm_ctrl_led(0x05,0x05,0x32);
						
						delay_ms(1000);
						
						/* �ɹ�����������һ��ʾ�� */
						#if BEER
						beep_on();delay_ms(50);beep_off();	
						#endif
						switch_flag=1;
						break;
					}
					else
					{
						/* ʧ��:��Ȧ��ʾ��ɫ */
						sfm_ctrl_led(0x03,0x03,0x32);
						
						delay_ms(1000);			
					}
					
					/* ��idΪ0����ȶԲ��ɹ���*/
					dgb_printf_safe("ˢָ�� %s ʶ��id=%d\r\n",sfm_error_code(rt),id);
					
					/* �ָ���Ȧȫ��->ȫ������2�� */
					sfm_ctrl_led(0x00,0x07,0xC8);
					
				}
				
				/* ��ȡ�û����� */
				if(key_sta & 0x04)
				{
					printf("\r\n\r\n=====================================\r\n\r\n");
			
					rt=sfm_get_user_total(&user_total);
			
					dgb_printf_safe("����ָ��ģ���û����� %s %d \r\n",sfm_error_code(rt),user_total);
				
					if(rt == SFM_ACK_SUCCESS)
					{
						/* �ɹ�����������һ��ʾ�� */
						#if BEER
						beep_on();delay_ms(50);beep_off();				
						#endif
					}		
				}	

				
				/* ɾ������ָ�� */
				if(key_sta & 0x08)
				{
					printf("\r\n\r\n=====================================\r\n\r\n");

					rt = sfm_del_user_all();

					dgb_printf_safe("ɾ�������û� %s\r\n",sfm_error_code(rt));
					
					if(rt == SFM_ACK_SUCCESS)
					{
						/* �ɹ�����������һ��ʾ�� */
						#if BEER
						beep_on();delay_ms(50);beep_off();
						#endif
					}			
				}
				vTaskDelay(100);				
				
			}
			if(switch_flag==1)
			{
				g_flag_in = 0;							//�˳�ָ��ģʽ
				
				/* �ָ���Ȧȫ��->ȫ������2�� */
				sfm_ctrl_led(0x00,0x07,0xC8);		
				
				PAout(4)=0;								//�̵�������
				g_door=1;								//����
			}
			
		}
	
		vTaskDelay(100);
	}

}

static void app_task_flash(void* pvParameters)
{
	uint8_t  data=0;												//��������
	uint16_t W25ID=0;												//�豸ID
	int i=0;														//��ʼ�������±�
	uint8_t data_buf[18]={0};										//��ʼ���������ݿռ�
	
	/*���Զ����ʼ����*/	
#if PASSWORD_INIT 
	
	//�Ȳ�����д
	Flash_SectorErase(PASSWORD_ADDR);
	delay_ms(500);
	//д������
	Flash_PageProgram(PASSWORD_ADDR, 10,(uint8_t *)PASSWORD_PRIMARY);
	
#endif	
	
	/*���Զ����ʼ����*/	
	
	W25ID = Flash_ReadID();											//��ȡ�豸ID
	printf("W25ID=%x\r\n",W25ID);
	delay_ms(500);	
	Flash_ReadData(PASSWORD_ADDR, 10,(uint8_t *)flash_buf);				//��ȡ����
	
#if PASSWORD_PASSWORD_LOOK 	
	
	printf("rdatabuf=%s\r\n",flash_buf);							//ʵ��ʱ���޸ĺ궨�弴��
	
#endif	
	
	delay_ms(500);
	memcpy(password,flash_buf,sizeof(flash_buf));					//��ս�����������,׼���鿴������
	for(;;)
	{
		if(g_flag_password_re==1)
		{
			printf("The password change mode is displayed\r\n");	//�����޸�ģʽ
			
			for(;;)
			{
				data=tmp; 											//��ȡ����ֵchar��������
			
				if(tmp != 'N')
				{
					#if BEER
					beep_on();delay_ms(100);beep_off();
					#endif
					printf("change : %c\n",data);	
					data_buf[i]=data;								//���ռ��̵���ʱ����,һ��ѭ������ʱ��,���ݱ��'N',����ûӰ��.ͬʱ���α������������뼴��.
					i++;
					if(data=='#')
					{
						printf("%s\n",data_buf);					//������ϣ���ӡ�鿴
						i=0;									
						//�Ȳ�����д
						Flash_SectorErase(PASSWORD_ADDR);				
						delay_ms(500);
						//д������
						Flash_PageProgram(PASSWORD_ADDR,sizeof(data_buf),data_buf);
						memset(data_buf,0,sizeof(data_buf));		//��յȴ���һ���޸�
						g_flag_password_look=1;						//��������鿴
						g_flag_password_re=0;						//�ر��޸�����ģʽ
						printf("Please enter a password with 18 characters or less\r\n");
						break;
					}
					data='N';
					
				}
				delay_ms(200);
			}
			printf("Password changed successfully\r\n");			//��ӡ�޸ĳɹ�
		}
		if(g_flag_password_look==1)
		{
			printf("Enter the password viewing mode\r\n");			//����鿴ģʽ
			W25ID = Flash_ReadID();
			printf("W25ID=%x\r\n",W25ID);
			delay_ms(500);
			Flash_ReadData(PASSWORD_ADDR,sizeof(data_buf),(uint8_t *)flash_buf);
			printf("rdatabuf=%s\r\n",flash_buf);
			g_flag_password_look=0;
			memcpy(password,flash_buf,sizeof(flash_buf));			//��ս�����������,׼���鿴������
		}
		delay_ms(1000);
		
	}

}

/*-----------------------------------------------------------------------------------------------*/

void vApplicationMallocFailedHook( void )
{
	/* vApplicationMallocFailedHook() will only be called if
	configUSE_MALLOC_FAILED_HOOK is set to 1 in FreeRTOSConfig.h.  It is a hook
	function that will get called if a call to pvPortMalloc() fails.
	pvPortMalloc() is called internally by the kernel whenever a task, queue,
	timer or semaphore is created.  It is also called by various parts of the
	demo application.  If heap_1.c or heap_2.c are used, then the size of the
	heap available to pvPortMalloc() is defined by configTOTAL_HEAP_SIZE in
	FreeRTOSConfig.h, and the xPortGetFreeHeapSize() API function can be used
	to query the size of free heap space that remains (although it does not
	provide information on how the remaining heap might be fragmented). */
	taskDISABLE_INTERRUPTS();
	for( ;; );
}
/*-----------------------------------------------------------------------------------------------*/

void vApplicationIdleHook( void )
{
	/* vApplicationIdleHook() will only be called if configUSE_IDLE_HOOK is set
	to 1 in FreeRTOSConfig.h.  It will be called on each iteration of the idle
	task.  It is essential that code added to this hook function never attempts
	to block in any way (for example, call xQueueReceive() with a block time
	specified, or call vTaskDelay()).  If the application makes use of the
	vTaskDelete() API function (as this demo application does) then it is also
	important that vApplicationIdleHook() is permitted to return to its calling
	function, because it is the responsibility of the idle task to clean up
	memory allocated by the kernel to any task that has since been deleted. */
}
/*-----------------------------------------------------------------------------------------------*/

void vApplicationStackOverflowHook( TaskHandle_t pxTask, char *pcTaskName )
{
	( void ) pcTaskName;
	( void ) pxTask;

	/* Run time stack overflow checking is performed if
	configCHECK_FOR_STACK_OVERFLOW is defined to 1 or 2.  This hook
	function is called if a stack overflow is detected. */
	taskDISABLE_INTERRUPTS();
	for( ;; );
}


void vApplicationTickHook( void )
{

}

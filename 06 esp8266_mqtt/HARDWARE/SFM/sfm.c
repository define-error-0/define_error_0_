#include "sfm.h"

uint8_t bcc_check(uint8_t *buf,uint32_t len)
{
	uint8_t s=0;
	uint8_t i=0;
	uint8_t *p = buf;
	
	for(i=0; i<len; i++)
		s = s^p[i];

	return s;
}
void sfm_touch_init(void)
{
	GPIO_InitTypeDef 		GPIO_InitStructure;

	//�򿪶˿�E��Ӳ��ʱ�ӣ����ǶԶ˿�E����
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOE,ENABLE);
	
	//����GPIOA�ĵ�0������
	GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_6;
	GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_Speed = GPIO_High_Speed;
	GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_DOWN;
	
	//��ʼ��
	GPIO_Init(GPIOE,&GPIO_InitStructure);	
	GPIO_SetBits(GPIOE, GPIO_Pin_6);
}

uint32_t sfm_touch_sta(void)
{
	return (PEin(6)==0);
}

int32_t sfm_init(uint32_t baud)
{
	int32_t rt=0;
	
	/* sfm����������ų�ʼ�� */
	sfm_touch_init();
	
	/* ����2 ��ʼ�� */
	usart2_init(baud);

	/* ��Ȧ����:ȫ��->ȫ������2�� */
	rt= sfm_ctrl_led(0x00,0x07,0xC8);
	if(rt != SFM_ACK_SUCCESS)	
		return rt;

	return SFM_ACK_SUCCESS;
}


int32_t sfm_ctrl_led(uint8_t led_start,uint8_t led_end,uint8_t period)
{
	uint8_t buf_tx[8]={0};
	
	memset((void *)g_usart2_rx_buf,0,sizeof g_usart2_rx_buf);
	g_usart2_rx_cnt=0;
	g_usart2_rx_end=0;
	
	/* �������ù�Ȧ�������� */
	buf_tx[0]=0xF5;
	buf_tx[1]=0xC3;	
	buf_tx[2]=led_start;	
	buf_tx[3]=led_end;	
	buf_tx[4]=period;		
	buf_tx[5]=0;
	buf_tx[6]=bcc_check(&buf_tx[1],5);
	buf_tx[7]=0xF5;
	
	usart_send_bytes(USART2,buf_tx,8);
	
	delay_ms(1000);
	
	if((g_usart2_rx_buf[0] == 0xF5) && (g_usart2_rx_buf[1] == 0xC3)&& (g_usart2_rx_buf[2] == SFM_ACK_SUCCESS))
	{
		return SFM_ACK_SUCCESS;
	}
	
	return SFM_ACK_FAIL;
}

int32_t sfm_reg_user(uint16_t id)
{
	uint8_t buf_tx[8]={0};
	
	memset((void *)g_usart2_rx_buf,0,sizeof g_usart2_rx_buf);
	g_usart2_rx_cnt=0;
	g_usart2_rx_end=0;
	
	/* ע���û����� */
	buf_tx[0]=0xF5;
	buf_tx[1]=0x01;	
	buf_tx[2]=(uint8_t)(id>>8);	
	buf_tx[3]=(uint8_t)(id&0x00FF);	
	buf_tx[4]=0x01;		
	buf_tx[5]=0x00;
	buf_tx[6]=bcc_check(&buf_tx[1],5);
	buf_tx[7]=0xF5;
	
	usart_send_bytes(USART2,buf_tx,8);
	
	delay_ms(1000);
	
	if(!((g_usart2_rx_buf[0] == 0xF5) && (g_usart2_rx_buf[1] == 0x01) && (g_usart2_rx_buf[4] == SFM_ACK_SUCCESS)))
	{
		return g_usart2_rx_buf[4];
	}
	
	memset((void *)g_usart2_rx_buf,0,sizeof g_usart2_rx_buf);
	g_usart2_rx_cnt=0;
	g_usart2_rx_end=0;
	
	/* ���͵�2�� */
	buf_tx[0]=0xF5;
	buf_tx[1]=0x02;	
	buf_tx[2]=0x00;	
	buf_tx[3]=0x00;	
	buf_tx[4]=0x00;		
	buf_tx[5]=0x00;
	buf_tx[6]=bcc_check(&buf_tx[1],5);
	buf_tx[7]=0xF5;
	
	usart_send_bytes(USART2,buf_tx,8);
	
	delay_ms(1000);
	
	if(!((g_usart2_rx_buf[0] == 0xF5) && (g_usart2_rx_buf[1] == 0x02) && (g_usart2_rx_buf[4] == SFM_ACK_SUCCESS)))
	{
		return g_usart2_rx_buf[4];
	}	
	
	memset((void *)g_usart2_rx_buf,0,sizeof g_usart2_rx_buf);
	g_usart2_rx_cnt=0;
	g_usart2_rx_end=0;	
	

	/* ���͵�3�� */
	buf_tx[0]=0xF5;
	buf_tx[1]=0x03;	
	buf_tx[2]=0x00;	
	buf_tx[3]=0x00;	
	buf_tx[4]=0x00;		
	buf_tx[5]=0x00;
	buf_tx[6]=bcc_check(&buf_tx[1],5);
	buf_tx[7]=0xF5;
	
	usart_send_bytes(USART2,buf_tx,8);
	
	delay_ms(1000);
	
	if(!((g_usart2_rx_buf[0] == 0xF5) && (g_usart2_rx_buf[1] == 0x03) && (g_usart2_rx_buf[4] == SFM_ACK_SUCCESS)))
	{
		return g_usart2_rx_buf[4];
	}
	
	if(g_usart2_rx_buf[2] == ((uint8_t)(id>>8)))
		if(g_usart2_rx_buf[3] == ((uint8_t)(id&0x00FF)))
			return SFM_ACK_SUCCESS;
		
	return g_usart2_rx_buf[2];
}

int32_t sfm_compare_users(uint16_t *id)
{
	uint8_t buf_tx[8]={0};
	
	memset((void *)g_usart2_rx_buf,0,sizeof g_usart2_rx_buf);
	g_usart2_rx_cnt=0;
	g_usart2_rx_end=0;
	
	/* 1:N�ȶ� */
	buf_tx[0]=0xF5;
	buf_tx[1]=0x0C;	
	buf_tx[2]=0x00;	
	buf_tx[3]=0x00;	
	buf_tx[4]=0x00;		
	buf_tx[5]=0x00;
	buf_tx[6]=bcc_check(&buf_tx[1],5);
	buf_tx[7]=0xF5;
	
	usart_send_bytes(USART2,buf_tx,8);
	
	delay_ms(1000);
	
	if(!((g_usart2_rx_buf[0] == 0xF5) && (g_usart2_rx_buf[1] == 0x0C)))
	{
		return SFM_ACK_FAIL;
	}	
	
	/* �����û�id */
	*id = (g_usart2_rx_buf[2]<<8)|g_usart2_rx_buf[3];
	
	/* ��id = 0x0000����ʾ�ȶԲ��ɹ� */
	if(*id == 0x0000)
		return SFM_ACK_NOUSER;
	
	return SFM_ACK_SUCCESS;
}


int32_t sfm_get_unused_id(uint16_t *id)
{
	uint8_t buf_tx[8]={0};
	
	memset((void *)g_usart2_rx_buf,0,sizeof g_usart2_rx_buf);
	g_usart2_rx_cnt=0;
	g_usart2_rx_end=0;
	
	/* ��ȡδʹ�õ��û����� */
	buf_tx[0]=0xF5;
	buf_tx[1]=0x0D;	
	buf_tx[2]=0x00;	
	buf_tx[3]=0x00;	
	buf_tx[4]=0x00;		
	buf_tx[5]=0x00;
	buf_tx[6]=bcc_check(&buf_tx[1],5);
	buf_tx[7]=0xF5;
	
	usart_send_bytes(USART2,buf_tx,8);
	
	delay_ms(1000);
	
	*id = (g_usart2_rx_buf[2]<<8)|g_usart2_rx_buf[3];
	
	if((g_usart2_rx_buf[0] == 0xF5) && (g_usart2_rx_buf[1] == 0x0D) && (g_usart2_rx_buf[4] == SFM_ACK_SUCCESS))
	{
		return SFM_ACK_SUCCESS;
	}

	return SFM_ACK_FAIL;
}

int32_t sfm_del_user(uint16_t id)
{
	uint8_t buf_tx[8]={0};
	
	memset((void *)g_usart2_rx_buf,0,sizeof g_usart2_rx_buf);
	g_usart2_rx_cnt=0;
	g_usart2_rx_end=0;
	
	/* ɾ���û����� */
	buf_tx[0]=0xF5;
	buf_tx[1]=0x04;	
	buf_tx[2]=(uint8_t)(id>>8);	
	buf_tx[3]=(uint8_t)(id&0x00FF);	
	buf_tx[4]=0x01;		
	buf_tx[5]=0x00;
	buf_tx[6]=bcc_check(&buf_tx[1],5);
	buf_tx[7]=0xF5;
	
	usart_send_bytes(USART2,buf_tx,8);
	
	delay_ms(1000);
	
	if((g_usart2_rx_buf[0] == 0xF5) && (g_usart2_rx_buf[1] == 0x04) && (g_usart2_rx_buf[4] == SFM_ACK_SUCCESS))
	{
		return SFM_ACK_SUCCESS;
	}

	return SFM_ACK_FAIL;
}


int32_t sfm_del_user_all(void)
{
	uint8_t buf_tx[8]={0};
	
	memset((void *)g_usart2_rx_buf,0,sizeof g_usart2_rx_buf);
	g_usart2_rx_cnt=0;
	g_usart2_rx_end=0;
	
	/* ɾ�������û����� */
	buf_tx[0]=0xF5;
	buf_tx[1]=0x05;	
	buf_tx[2]=0x00;	
	buf_tx[3]=0x00;	
	buf_tx[4]=0x00;		
	buf_tx[5]=0x00;
	buf_tx[6]=bcc_check(&buf_tx[1],5);
	buf_tx[7]=0xF5;
	
	usart_send_bytes(USART2,buf_tx,8);
	
	delay_ms(1000);
	
	if((g_usart2_rx_buf[0] == 0xF5) && (g_usart2_rx_buf[1] == 0x05) && (g_usart2_rx_buf[4] == SFM_ACK_SUCCESS))
	{
		return SFM_ACK_SUCCESS;
	}

	return SFM_ACK_FAIL;
}

int32_t sfm_get_user_total(uint16_t *user_total)
{
	uint8_t buf_tx[8]={0};
	
	memset((void *)g_usart2_rx_buf,0,sizeof g_usart2_rx_buf);
	g_usart2_rx_cnt=0;
	g_usart2_rx_end=0;
	
	/* ɾ���û����� */
	buf_tx[0]=0xF5;
	buf_tx[1]=0x09;	
	buf_tx[2]=0x00;	
	buf_tx[3]=0x00;	
	buf_tx[4]=0x00;		
	buf_tx[5]=0x00;
	buf_tx[6]=bcc_check(&buf_tx[1],5);
	buf_tx[7]=0xF5;
	
	usart_send_bytes(USART2,buf_tx,8);
	
	delay_ms(1000);
	
	if(!((g_usart2_rx_buf[0] == 0xF5) && (g_usart2_rx_buf[1] == 0x09)))
	{
		return SFM_ACK_FAIL;
	}	
	
	/* �����û����� */
	*user_total = (g_usart2_rx_buf[2]<<8)|g_usart2_rx_buf[3];
	
	return SFM_ACK_SUCCESS;
}

int32_t sfm_touch_check(void)
{
	uint8_t buf_tx[8]={0};
	
	memset((void *)g_usart2_rx_buf,0,sizeof g_usart2_rx_buf);
	g_usart2_rx_cnt=0;
	g_usart2_rx_end=0;
	
	/* ��ⴥ����Ӧ���� */
	buf_tx[0]=0xF5;
	buf_tx[1]=0x30;	
	buf_tx[2]=0x00;	
	buf_tx[3]=0x00;	
	buf_tx[4]=0x00;		
	buf_tx[5]=0x00;
	buf_tx[6]=bcc_check(&buf_tx[1],5);
	buf_tx[7]=0xF5;
	
	usart_send_bytes(USART2,buf_tx,8);
	
	delay_ms(1000);
	
	if((g_usart2_rx_buf[0] == 0xF5) && (g_usart2_rx_buf[1] == 0x30) && (g_usart2_rx_buf[4] == SFM_ACK_SUCCESS))
	{
		return SFM_ACK_SUCCESS;
	}	
	
	return SFM_ACK_FAIL;
}

const char *sfm_error_code(uint8_t error_code)
{
	const char *p;
	
	switch(error_code)
	{
		case SFM_ACK_SUCCESS:p="ִ�гɹ�";
		break;
		
		case SFM_ACK_FAIL:p="ִ��ʧ��";
		break;	

		case SFM_ACK_FULL:p="���ݿ���";
		break;		

		case SFM_ACK_NOUSER:p="û������û�";
		break;		

		case SFM_ACK_USER_EXIST:p="�û��Ѵ���";
		break;	
		
		case SFM_ACK_TIMEOUT:p="ͼ��ɼ���ʱ";
		break;		
	
		case SFM_ACK_HARDWAREERROR:p="Ӳ������";
		break;	
		
		case SFM_ACK_IMAGEERROR:p="ͼ�����";
		break;	

		case SFM_ACK_BREAK:p="��ֹ��ǰָ��";
		break;	

		case SFM_ACK_ALGORITHMFAIL:p="��Ĥ�������";
		break;	
		
		case SFM_ACK_HOMOLOGYFAIL:p="ͬԴ��У�����";
		break;

		default :
			p="ģ�鷵��ȷ��������";break;
	}

	return p;


}

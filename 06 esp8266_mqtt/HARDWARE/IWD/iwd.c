#include "iwd.h"

//�������Ź�
void IWDG_Start(void)
{
	//1.���д����
	IWDG_WriteAccessCmd(IWDG_WriteAccess_Enable);
	//IWDG->KR = 0x5555; //�Ĵ���д��
	
	//2.���÷�Ƶֵ 32KHz/32=1000Hz  1s��1000��
	IWDG_SetPrescaler(IWDG_Prescaler_32);

	//3.����ֵ ��ʱ3s   Ҫ��3����ι��
	IWDG_SetReload(4000);

	//4.�������Ź�
	IWDG_Enable();
	IWDG_ReloadCounter(); //��ιһ�ι�
	//IWDG->KR = 0xCCCC;
}

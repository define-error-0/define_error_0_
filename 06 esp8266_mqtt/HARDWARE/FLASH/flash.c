#include "flash.h"

#define W25Q_CS  PBout(14)


/*
CS--PB14 ---IO����  ѡ��������
SCK--PB3 ---���� ---������Ҫ���ݹ���ģʽ�ļ��� 
MISO--PB4
MOSI--PB5
*/

void Flash_config(void)
{
	GPIO_InitTypeDef  GPIO_InitStructure;
	SPI_InitTypeDef  SPI_InitStruct;

	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB,ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_SPI1, ENABLE);
	
	GPIO_InitStructure.GPIO_Pin=GPIO_Pin_14;//CS
	GPIO_InitStructure.GPIO_Mode=GPIO_Mode_OUT;//���ģʽ
	GPIO_InitStructure.GPIO_OType=GPIO_OType_PP;//���� Push Pull
	GPIO_InitStructure.GPIO_PuPd=GPIO_PuPd_UP;//
	GPIO_InitStructure.GPIO_Speed=GPIO_High_Speed;//����(�ٶ�Խ�ߣ�����Խ�ߣ�Ҳ�����ײ�����Ÿ���)
	GPIO_Init(GPIOB,&GPIO_InitStructure);
	
	//Ĭ�ϸ��ߵ�ƽ
	PBout(14)=1;

	GPIO_InitStructure.GPIO_Pin=GPIO_Pin_4|GPIO_Pin_5;//CS
	GPIO_InitStructure.GPIO_Mode=GPIO_Mode_AF;//����ģʽ
	GPIO_InitStructure.GPIO_OType=GPIO_OType_PP;//���� Push Pull
	GPIO_InitStructure.GPIO_PuPd=GPIO_PuPd_UP;//
	GPIO_InitStructure.GPIO_Speed=GPIO_High_Speed;
	GPIO_Init(GPIOB,&GPIO_InitStructure);
	
	GPIO_InitStructure.GPIO_Pin=GPIO_Pin_3;//CS
	GPIO_InitStructure.GPIO_Mode=GPIO_Mode_AF;//����ģʽ
	GPIO_InitStructure.GPIO_OType=GPIO_OType_PP;//���� Push Pull
	GPIO_InitStructure.GPIO_PuPd=GPIO_PuPd_DOWN;//ģʽ0
	GPIO_InitStructure.GPIO_Speed=GPIO_High_Speed;
	GPIO_Init(GPIOB,&GPIO_InitStructure);
	
	GPIO_PinAFConfig(GPIOB, GPIO_PinSource3,GPIO_AF_SPI1);
	GPIO_PinAFConfig(GPIOB, GPIO_PinSource4,GPIO_AF_SPI1);
	GPIO_PinAFConfig(GPIOB, GPIO_PinSource5,GPIO_AF_SPI1);
	
	SPI_InitStruct.SPI_Direction=SPI_Direction_2Lines_FullDuplex;
	SPI_InitStruct.SPI_Mode=SPI_Mode_Master;
	SPI_InitStruct.SPI_DataSize=SPI_DataSize_8b;
	SPI_InitStruct.SPI_CPOL=SPI_CPOL_Low;
	SPI_InitStruct.SPI_CPHA=SPI_CPHA_1Edge;
	SPI_InitStruct.SPI_NSS=SPI_NSS_Soft;
	SPI_InitStruct.SPI_BaudRatePrescaler=SPI_BaudRatePrescaler_16;
	SPI_InitStruct.SPI_FirstBit=SPI_FirstBit_MSB;
	SPI_InitStruct.SPI_CRCPolynomial=7;
	
	SPI_Init(SPI1,&SPI_InitStruct);
	
	SPI_Cmd(SPI1, ENABLE);
}


//�շ�һ��
uint8_t FLASH_SendByte(uint8_t byte)
{
  /*!< Loop while DR register in not emplty */
  while (SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_TXE) == RESET);

  /*!< Send byte through the SPI1 peripheral */
  SPI_I2S_SendData(SPI1, byte);

  /*!< Wait to receive a byte */
  while (SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_RXNE) == RESET);

  /*!< Return the byte read from the SPI bus */
  return SPI_I2S_ReceiveData(SPI1);
}

//��ȡID
uint16_t Flash_ReadID(void)
{
//	uint8_t ID1=0;
//	uint8_t ID2=0;
	uint16_t ID=0;
	//����ʹ��Ƭѡ
	W25Q_CS=0;
	
	//����ָ��(0x90)
	FLASH_SendByte(0x90);

	//���͵�ַ
	FLASH_SendByte(0x00); //�ȷ����Ǹ�λ FLASH_SendByte(Address>>16);
	FLASH_SendByte(0x00);
	FLASH_SendByte(0x00);
	
//	ID1=FLASH_SendByte(0xFF);
//	ID2=FLASH_SendByte(0xFF);
	
	ID=FLASH_SendByte(0xFF)<<8;//���ճ���ID���ڵ�8λ
	ID|=FLASH_SendByte(0xFF);
	W25Q_CS=1;//�������
	
	return ID;
}

void  Flash_ReadData(uint32_t Address, uint32_t datalen,uint8_t *rdatabuf)
{
	
//	uint32_t len=0;
	//����ʹ��Ƭѡ
	W25Q_CS=0;

	//����ָ��(0x03)
	FLASH_SendByte(0x03);

	//����24λ��ַ
	FLASH_SendByte((Address & 0xFF0000)>>16);
	FLASH_SendByte((Address & 0xFF00)>>8);
	FLASH_SendByte((Address & 0xFF));
	
	//��������
//	for(len=0;len<datalen;len++)
//	{
//	
//		rdatabuf[len]=FLASH_SendByte(0xFF);

//	}
	while(datalen--)
	{
		*rdatabuf++=FLASH_SendByte(0xFF);
	}
	
	
	W25Q_CS=1;//�������
}

//ʹ��д��
void  Flash_WriteEnable(void)
{
	//����ʹ��Ƭѡ
	W25Q_CS=0;

	//����ָ��(0x06)
	FLASH_SendByte(0x06);
	
	W25Q_CS=1;//�������
}

//��ֹд��
void  Flash_WriteDisable(void)
{
	//����ʹ��Ƭѡ
	W25Q_CS=0;

	//����ָ��(0x04)
	FLASH_SendByte(0x04);
	
	W25Q_CS=1;//�������
}

//��ȡ״̬�Ĵ���1
uint8_t Flash_ReadStatusRegister1(void)
{
	uint8_t Status=0;
	//����ʹ��Ƭѡ
	W25Q_CS=0;

	//����ָ��(0x05)
	FLASH_SendByte(0x05);
	
	Status=FLASH_SendByte(0xFF);
	
	W25Q_CS=1;//�������
	
	return  Status;
}

//��������  Address--��ʼ��ַ
void Flash_SectorErase(uint32_t Address)
{
	//Ҫ��дʹ��
	Flash_WriteEnable();
	
	//����ʹ��Ƭѡ
	W25Q_CS=0;
	
	//����ָ��(0x20)
	FLASH_SendByte(0x20);
	
	//����24λ��ַ
	FLASH_SendByte((Address & 0xFF0000)>>16);
	FLASH_SendByte((Address & 0xFF00)>>8);
	FLASH_SendByte((Address & 0xFF));
	
	W25Q_CS=1;//�������
	
	// xxxx xxxx & 0000 0001 �ж����λ
	while(Flash_ReadStatusRegister1() &0x01); //�ж��Ƿ�æ
	
	Flash_WriteDisable();//��ֹд��
}

//ҳд
void  Flash_PageProgram(uint32_t Address, uint32_t datalen,uint8_t *rdatabuf)
{

	//uint32_t len=0;
	//Ҫ��дʹ��
	Flash_WriteEnable();
	//����ʹ��Ƭѡ
	W25Q_CS=0;

	//����ָ��(0x02)
	FLASH_SendByte(0x02);

	//����24λ��ַ
	FLASH_SendByte((Address & 0xFF0000)>>16);
	FLASH_SendByte((Address & 0xFF00)>>8);
	FLASH_SendByte((Address & 0xFF));
	
	//��������
//	for(len=0;len<datalen;len++)
//	{
//	
//		rdatabuf[len]=FLASH_SendByte(0xFF);

//	}
	while(datalen--)
	{
		FLASH_SendByte(*rdatabuf++); //д���ݾͲ��ý�����
	}
	
	
	W25Q_CS=1;//�������
	
	while(Flash_ReadStatusRegister1() &0x01); //�ж��Ƿ�æ
	
	Flash_WriteDisable();//��ֹд��
}






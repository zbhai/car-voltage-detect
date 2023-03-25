#include "sys.h"
#include "usart.h"	  
#include "timer.h"
#include "bc26.h"
#include "led.h"
//////////////////////////////////////////////////////////////////
//�������´���,֧��printf����,������Ҫѡ��use MicroLIB	  
#if 1
#pragma import(__use_no_semihosting)             
//��׼����Ҫ��֧�ֺ���                 
struct __FILE 
{ 
	int handle; 
}; 

FILE __stdout;       
//����_sys_exit()�Ա���ʹ�ð�����ģʽ    
void _sys_exit(int x) 
{ 
	x = x; 
} 
//�ض���fputc���� 
int fputc(int ch, FILE *f)
{      
	while((USART1->SR&0X40)==0) {};//ѭ������,ֱ���������   
    USART1->DR = (uint8_t) ch;      
	return ch;
}
#endif 


#if EN_USART1_RX   //���ʹ���˽���
//����1�жϷ������
//ע��,��ȡUSARTx->SR�ܱ���Ī������Ĵ���   	
uint8_t USART_RX_BUF[USART_MAX_RECV_LEN] = {0};     //���ջ���,USART_MAX_RECV_LEN���ֽ�
//����״̬
//bit15��	������ɱ�־
//bit14��	���յ�0x0d
//bit13~0��	���յ�����Ч�ֽ���Ŀ
u16 USART_RX_STA=0;       //����״̬���	  
  
void uart_init(u32 bound){
    //GPIO�˿�����
    GPIO_InitTypeDef GPIO_InitStructure;
    USART_InitTypeDef USART_InitStructure;
    NVIC_InitTypeDef NVIC_InitStructure;
     
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1|RCC_APB2Periph_GPIOA, ENABLE);	//ʹ��USART1��GPIOAʱ��

    //USART1_TX   GPIOA.9
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;                           //PA.9
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;	                    //�����������
    GPIO_Init(GPIOA, &GPIO_InitStructure);                              //��ʼ��GPIOA.9

    //USART1_RX	  GPIOA.10��ʼ��
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;                          //PA10
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;               //��������
    GPIO_Init(GPIOA, &GPIO_InitStructure);                              //��ʼ��GPIOA.10  

    //Usart1 NVIC ����
    NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=3 ;            //��ռ���ȼ�3
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;		            //�����ȼ�3
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			            //IRQͨ��ʹ��
    NVIC_Init(&NVIC_InitStructure);	                                    //����ָ���Ĳ�����ʼ��VIC�Ĵ���

    //USART ��ʼ������

    USART_InitStructure.USART_BaudRate = bound;                         //���ڲ�����
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;         //�ֳ�Ϊ8λ���ݸ�ʽ
    USART_InitStructure.USART_StopBits = USART_StopBits_1;              //һ��ֹͣλ
    USART_InitStructure.USART_Parity = USART_Parity_No;                 //����żУ��λ
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;//��Ӳ������������
    USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;	                //�շ�ģʽ

    USART_Init(USART1, &USART_InitStructure);                           //��ʼ������1
    USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);                      //�������ڽ����ж�
    USART_Cmd(USART1, ENABLE);                                          //ʹ�ܴ���1 

    TIM3_Init(1000-1,7200-1);		                                    //10ms�ж�
    USART_RX_STA=0;		                                                //����
    TIM_Cmd(TIM3,DISABLE);			                                    //�رն�ʱ��3
}

void USART1_IRQHandler(void)                	                        //����1�жϷ������
{
    uint8_t res;	
    uint8_t *p = 0;
    
    if(USART_GetITStatus(USART1, USART_IT_ORE) != RESET)    //����USART_SR 
    {
          USART_ReceiveData(USART1);                        //����USART_DR 
    }
    
	if(USART_GetITStatus(USART1, USART_IT_RXNE) != RESET)//���յ�����
	{	 
		res = USART_ReceiveData(USART1);		 
		if((USART_RX_STA&(1<<15))==0)                           //�������һ������,��û�б�����,���ٽ�����������
		{ 
			if(USART_RX_STA < USART_MAX_RECV_LEN)	            //�����Խ�������
			{
				TIM_SetCounter(TIM3,0);                         //���������      
				if(USART_RX_STA==0) 				            //ʹ�ܶ�ʱ��3���ж� 
				{
					TIM_Cmd(TIM3,ENABLE);                       //ʹ�ܶ�ʱ��3
				}
				USART_RX_BUF[USART_RX_STA++]=res;	            //��¼���յ���ֵ
			}else 
			{
				USART_RX_STA|=1<<15;				            //ǿ�Ʊ�ǽ������
                p = (uint8_t *)strstr((char *)USART_RX_BUF,"Check_Mode");
                if(p)
                {
//                    USART_RX_STA = 0;
                    Check_Mode = !Check_Mode;
                    memset(USART_RX_BUF, 0, 200);
                }
			} 
		}
	}  
} 
#endif	


#ifndef __USART_H
#define __USART_H
#include "stdio.h"	
#include "sys.h" 

#define USART_MAX_RECV_LEN  			200  	//�����������ֽ��� 200
#define EN_USART1_RX 			        1		//ʹ�ܣ�1��/��ֹ��0������1����
	  	
extern u8 USART_RX_BUF[USART_MAX_RECV_LEN];     //���ջ���,���USART_REC_LEN���ֽ�.
extern u16 USART_RX_STA;       //����״̬���	  
//����봮���жϽ��գ��벻Ҫע�����º궨��
void uart_init(u32 bound);

#endif



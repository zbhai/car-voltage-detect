#include "sys.h"
#include "usart.h"	  
#include "timer.h"
#include "bc26.h"
#include "led.h"
//////////////////////////////////////////////////////////////////
//加入以下代码,支持printf函数,而不需要选择use MicroLIB	  
#if 1
#pragma import(__use_no_semihosting)             
//标准库需要的支持函数                 
struct __FILE 
{ 
	int handle; 
}; 

FILE __stdout;       
//定义_sys_exit()以避免使用半主机模式    
void _sys_exit(int x) 
{ 
	x = x; 
} 
//重定义fputc函数 
int fputc(int ch, FILE *f)
{      
	while((USART1->SR&0X40)==0) {};//循环发送,直到发送完毕   
    USART1->DR = (uint8_t) ch;      
	return ch;
}
#endif 


#if EN_USART1_RX   //如果使能了接收
//串口1中断服务程序
//注意,读取USARTx->SR能避免莫名其妙的错误   	
uint8_t USART_RX_BUF[USART_MAX_RECV_LEN] = {0};     //接收缓冲,USART_MAX_RECV_LEN个字节
//接收状态
//bit15，	接收完成标志
//bit14，	接收到0x0d
//bit13~0，	接收到的有效字节数目
u16 USART_RX_STA=0;       //接收状态标记	  
  
void uart_init(u32 bound){
    //GPIO端口设置
    GPIO_InitTypeDef GPIO_InitStructure;
    USART_InitTypeDef USART_InitStructure;
    NVIC_InitTypeDef NVIC_InitStructure;
     
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1|RCC_APB2Periph_GPIOA, ENABLE);	//使能USART1，GPIOA时钟

    //USART1_TX   GPIOA.9
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;                           //PA.9
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;	                    //复用推挽输出
    GPIO_Init(GPIOA, &GPIO_InitStructure);                              //初始化GPIOA.9

    //USART1_RX	  GPIOA.10初始化
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;                          //PA10
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;               //浮空输入
    GPIO_Init(GPIOA, &GPIO_InitStructure);                              //初始化GPIOA.10  

    //Usart1 NVIC 配置
    NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=3 ;            //抢占优先级3
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;		            //子优先级3
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			            //IRQ通道使能
    NVIC_Init(&NVIC_InitStructure);	                                    //根据指定的参数初始化VIC寄存器

    //USART 初始化设置

    USART_InitStructure.USART_BaudRate = bound;                         //串口波特率
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;         //字长为8位数据格式
    USART_InitStructure.USART_StopBits = USART_StopBits_1;              //一个停止位
    USART_InitStructure.USART_Parity = USART_Parity_No;                 //无奇偶校验位
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;//无硬件数据流控制
    USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;	                //收发模式

    USART_Init(USART1, &USART_InitStructure);                           //初始化串口1
    USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);                      //开启串口接受中断
    USART_Cmd(USART1, ENABLE);                                          //使能串口1 

    TIM3_Init(1000-1,7200-1);		                                    //10ms中断
    USART_RX_STA=0;		                                                //清零
    TIM_Cmd(TIM3,DISABLE);			                                    //关闭定时器3
}

void USART1_IRQHandler(void)                	                        //串口1中断服务程序
{
    uint8_t res;	
    uint8_t *p = 0;
    
    if(USART_GetITStatus(USART1, USART_IT_ORE) != RESET)    //读入USART_SR 
    {
          USART_ReceiveData(USART1);                        //读入USART_DR 
    }
    
	if(USART_GetITStatus(USART1, USART_IT_RXNE) != RESET)//接收到数据
	{	 
		res = USART_ReceiveData(USART1);		 
		if((USART_RX_STA&(1<<15))==0)                           //接收完的一批数据,还没有被处理,则不再接收其他数据
		{ 
			if(USART_RX_STA < USART_MAX_RECV_LEN)	            //还可以接收数据
			{
				TIM_SetCounter(TIM3,0);                         //计数器清空      
				if(USART_RX_STA==0) 				            //使能定时器3的中断 
				{
					TIM_Cmd(TIM3,ENABLE);                       //使能定时器3
				}
				USART_RX_BUF[USART_RX_STA++]=res;	            //记录接收到的值
			}else 
			{
				USART_RX_STA|=1<<15;				            //强制标记接收完成
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


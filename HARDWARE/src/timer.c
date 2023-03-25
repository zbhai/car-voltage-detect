#include "timer.h"
#include "key.h"
#include "bc26.h"
#include "usart3.h"
#include "adc.h"
#include "stm32f10x.h"
#include "usart.h"
#include "led.h"

uint8_t post_flag = 0;	  // ��������������ݱ�־λ
uint8_t ping_flag = 0;	  // ��������־λ
uint8_t ten_min_flag = 0; // ʮ���ӵ���

// ͨ�ö�ʱ��3�жϳ�ʼ��
// ����ʱ��ѡ��ΪAPB1��2������APB1Ϊ36M
// arr���Զ���װֵ��
// psc��ʱ��Ԥ��Ƶ��
void TIM3_Init(u16 arr, u16 psc)
{
	TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
	NVIC_InitTypeDef NVIC_InitStructure;

	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE); // ʱ��ʹ��

	// ��ʱ��TIM3��ʼ��
	TIM_TimeBaseStructure.TIM_Period = arr;						// ��������һ�������¼�װ�����Զ���װ�ؼĴ������ڵ�ֵ
	TIM_TimeBaseStructure.TIM_Prescaler = psc;					// ����������ΪTIMxʱ��Ƶ�ʳ�����Ԥ��Ƶֵ
	TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;		// ����ʱ�ӷָ�:TDTS = Tck_tim
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up; // TIM���ϼ���ģʽ
	TIM_TimeBaseInit(TIM3, &TIM_TimeBaseStructure);				// ����ָ���Ĳ�����ʼ��TIMx��ʱ�������λ

	TIM_ITConfig(TIM3, TIM_IT_Update, ENABLE); // ʹ��ָ����TIM3�ж�,��������ж�

	// �ж����ȼ�NVIC����
	NVIC_InitStructure.NVIC_IRQChannel = TIM3_IRQn;			  // TIM3�ж�
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 4; // ��ռ���ȼ�4��
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;		  // �����ȼ�0��
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			  // IRQͨ����ʹ��
	NVIC_Init(&NVIC_InitStructure);							  // ��ʼ��NVIC�Ĵ���
}

// ͨ�ö�ʱ���жϳ�ʼ��
// ����ʱ��ѡ��ΪAPB1��2������APB1Ϊ36M
// arr���Զ���װֵ��
// psc��ʱ��Ԥ��Ƶ��
void TIM2_Init(u16 arr, u16 psc)
{
	TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
	NVIC_InitTypeDef NVIC_InitStructure;

	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE); // ʱ��ʹ��

	TIM_TimeBaseStructure.TIM_Period = arr - 1;					// ��������һ�������¼�װ�����Զ���װ�ؼĴ������ڵ�ֵ
	TIM_TimeBaseStructure.TIM_Prescaler = psc - 1;				// ����������ΪTIMxʱ��Ƶ�ʳ�����Ԥ��Ƶֵ
	TIM_TimeBaseStructure.TIM_ClockDivision = 0;				// ����ʱ�ӷָ�:TDTS = Tck_tim
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up; // TIM���ϼ���ģʽ
	TIM_TimeBaseInit(TIM2, &TIM_TimeBaseStructure);				// ����TIM_TimeBaseInitStruct��ָ���Ĳ�����ʼ��TIMx��ʱ�������λ

	TIM_ITConfig(TIM2, TIM_IT_Update, ENABLE); // TIM2

	NVIC_InitStructure.NVIC_IRQChannel = TIM2_IRQn;			  // TIM2�ж�
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0; // ��ռ���ȼ�0��
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;		  // �����ȼ�3��
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			  // IRQͨ����ʹ��
	NVIC_Init(&NVIC_InitStructure);							  // ����NVIC_InitStruct��ָ���Ĳ�����ʼ������NVIC�Ĵ���

	TIM_Cmd(TIM2, ENABLE); // ʹ��TIMx����
}

void TIM2_IRQHandler(void) // TIM2�ж�
{
	static uint32_t heartbeat_count = 0, post_sec_count = 0, ten_minCount = 0;
	if (TIM_GetITStatus(TIM2, TIM_IT_Update) != RESET) // ���ָ����TIM�жϷ������:TIM �ж�Դ
	{
		TIM_ClearITPendingBit(TIM2, TIM_IT_Update); // ���TIMx���жϴ�����λ:TIM �ж�Դ
		heartbeat_count++;							// ��ʼ��ʱ
		post_sec_count++;							// ���͵�����ʱ
		if (!Connect_flag)
			ten_minCount++; // ������ʱ
		else
			ten_minCount = 0;

		//        switch(Check_Mode)
		//        {
		//            case 0:if(post_sec_count >= 600)       {post_flag = 1; post_sec_count = 0;} break;   // 1����
		//            case 1:if(post_sec_count >= 36000)  {post_flag = 1; post_sec_count = 0;} break;   // 1Сʱ
		//            case 2:if(post_sec_count >= 72000)  {post_flag = 1; post_sec_count = 0;} break;   // 2Сʱ
		//            case 3:if(post_sec_count >= 108000) {post_flag = 1; post_sec_count = 0;} break;   // 3Сʱ
		//            case 4:if(post_sec_count >= 216000) {post_flag = 1; post_sec_count = 0;} break;   // 6Сʱ
		//            case 5:if(post_sec_count >= 432000) {post_flag = 1; post_sec_count = 0;} break;   // 12Сʱ
		//            default: if(post_sec_count >= 600)       {post_flag = 1; post_sec_count = 0;} break;   // 5S
		//        }
		// if(post_sec_count >= 36000)     // 1Сʱ
		// {
		//     post_flag = 1; post_sec_count = 0;
		// }
		if (post_sec_count >= 1800) // 1Сʱ
		{
			post_flag = 1;
			post_sec_count = 0;
		}
		if (heartbeat_count >= 9000) // �Ű���
		{
			ping_flag = 1;
			heartbeat_count = 0;
		}
		if (ten_minCount >= 6000) // 10����
		{
			ten_min_flag = 1;
			ten_minCount = 0;
		}
	}
}

// ��ʱ��3�жϷ�����
void TIM3_IRQHandler(void)
{
	uint8_t *p = 0;
	if (TIM_GetITStatus(TIM3, TIM_IT_Update) == SET) // �����ж�
	{
		USART_RX_STA |= 1 << 15;					// ��ǽ������
		TIM_ClearITPendingBit(TIM3, TIM_IT_Update); // ���TIM3�����жϱ�־
		p = (uint8_t *)strstr((char *)USART_RX_BUF, "Check_Mode");
		if (p)
		{
			//            USART_RX_STA = 0;
			Check_Mode = !Check_Mode; // *(p+12) - '0'
			memset(USART_RX_BUF, 0, 200);
		}
		TIM_Cmd(TIM3, DISABLE); // �ر�TIM3
	}
}

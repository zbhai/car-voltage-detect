#include "led.h"
#include "key.h"
#include "delay.h"
#include "sys.h"
#include "usart.h"
#include "usart3.h"
#include "adc.h"
#include "bc26.h"
#include "timer.h"
#include "string.h"

uint8_t BACK_UP_CHECKk_MODE = 0; // ��¼δ���ĵ�Mode

// ���շ���������������ݷ���ģʽ
void mode_change_by_sever(void)
{
    if (BACK_UP_CHECKk_MODE != Check_Mode)
    {
        USART_RX_STA = 0;
        BACK_UP_CHECKk_MODE = Check_Mode;
        Post_Send(); // Asso_QMTPUB_event();
        RS_INFO("���յ�״̬ת��\r\n");
    }
    //    if(strstr((char *)USART_RX_BUF,"+QMTSTAT")) Connect_flag = 0;       //���յ������������쳣�Ͽ���Ϣ��Connect_flag��0���������ӷ�����
    //    else Connect_flag = 1;
}

int main(void)
{
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);
    delay_init();                  // ��ʱ��ʼ��
    uart_init(115200);             // ����1��ʼ��Ϊ115200
    Usart3_Init(115200);           // ����3��ʼ��Ϊ115200
    LED_Init();                    // ��ʼ����LED���ӵ�Ӳ���ӿ�
                                   //    KEY_Init();          	            // ��ʼ���밴�����ӵ�Ӳ���ӿ�, ����δʹ��
    Adc_Init();                    // ADC��ʼ��
    TIM2_Init(1000 - 1, 7200 - 1); // 10KHz�ļ���Ƶ�ʣ�������1000Ϊ100ms
    while (WiFi_Connect_IoTServer())
        ; // ��ʼ����

    while (1)
    {
        if (Connect_flag || !ten_min_flag)
        {
            mode_change_by_sever();
            if (post_flag) // ÿ��ָ��ʱ�������������һ�ε�ѹ�ɼ�ֵ
            {
                Post_Send();
                post_flag = 0;
            }
            if (ping_flag) // ����������
            {
                Asso_QMTPUB_event();
                ping_flag = 0;
            }
        }
        else
        {
            ten_min_flag = 0;
            while (WiFi_Connect_IoTServer())
                ;
        }
    }
}

#include "bc26.h"
#include "led.h"
#include "string.h"
#include "delay.h"
#include "usart.h"
#include "usart3.h"
#include "adc.h"
#include "math.h"

char Value[10];              // �洢��ѹֵ������
unsigned int Check_Mode = 0; // ���ģʽ

char *Receive_Data;            // �洢BC26�ź�ǿ�Ȼ�����
BC26 BC26_Status;              // BC26�ź�״̬
unsigned int Connect_flag = 0; // ���ӱ�־λ

/*
 * У�鷵������
 * ����ֵ: 0:    û���ҵ���Ӧ��Ӧ��
 *         ����: Ӧ�����ڵ�λ��
 */
uint8_t *BC26_check_cmd(uint8_t *str)
{
    char *strx = 0;
    if (USART_RX_STA & 0X8000) // ���յ�һ��������
    {
        USART_RX_BUF[USART_RX_STA & 0X7FFF] = 0; // ��ӽ�����
        strx = strstr((const char *)USART_RX_BUF, (const char *)str);
    }
    return (uint8_t *)strx;
}

/*
 * ��BC26����ָ��
 * cmd:       ATָ��
 * ack:       �ڴ��Ļظ�
 * waittime:  ��Ӧ��ѯ����, ��λms
 * retry_num: ��Ӧ��ѯ����
 *************waittime * retry_num = ÿ����Ϣ��ȴ�ʱ��, ��λms
 */
uint8_t BC26_send_cmd(uint8_t *cmd, uint8_t *ack, uint16_t waittime, uint16_t retry_num)
{
    uint8_t res = 0;              // ����ֵ�洢����
    USART_RX_STA = 0;             // �������1���ձ�־
    memset(USART_RX_BUF, 0, 200); // ��ս��տռ�

    if (retry_num)
    {
        while (--retry_num) // �ȴ�����ʱ
        {
            RS_INFO("%d ", retry_num);
            printf("%s\r\n", cmd);
            RS_INFO("CMD : %s\r\n", cmd);
            delay_ms(waittime);        // �ȴ���ʱ,300ms
            if (USART_RX_STA & 0X8000) // ���յ��ڴ���Ӧ����
            {
                if (BC26_check_cmd(ack))
                {
                    RS_INFO("received %s \r\n", USART_RX_BUF);
                    USART_RX_STA = 0;
                    break; // �õ���Ч����
                }
                USART_RX_STA = 0; // ���״̬��־
            }
        }
        if (retry_num == 0)
        {
            res = 1;
        }
    }
    return res;
}

/*
 * BC26���ͺͷ���������ָ��
 * cmd:       ATָ��
 * ack:       �ڴ��Ļظ�
 * waittime:  ��Ӧ��ѯ����, ��λms
 * retry_num: ��Ӧ��ѯ����
 *************waittime * retry_num = ÿ����Ϣ��ȴ�ʱ��, ��λms
 */
uint8_t BC26_send_cmd2_sever(uint8_t *cmd, uint8_t *ack, uint16_t waittime, uint16_t retry_num)
{
    uint8_t res = 0;              // ����ֵ�洢����
    uint8_t num = 0;              // ������Դ���
    USART_RX_STA = 0;             // �������1���ձ�־
    memset(USART_RX_BUF, 0, 200); // ��ս��տռ�

    for (; num < 5; num++) // ������Դ���
    {
        printf("%s\r\n", cmd);
        if (retry_num)
        {
            while (--retry_num) // �ȴ�����ʱ
            {
                delay_ms(waittime);        // �ȴ���ʱ,300ms
                if (USART_RX_STA & 0X8000) // ���յ��ڴ���Ӧ����
                {
                    if (BC26_check_cmd(ack))
                    {
                        RS_INFO("received %s \r\n", USART_RX_BUF);
                        USART_RX_STA = 0;
                        goto exit; // �õ���Ч����
                    }
                    USART_RX_STA = 0; // ���״̬��־
                }
            }
        }
    }
exit:
    if ((retry_num == 0) && (num >= 5))
    {
        res = 1;
    } // �������Դ���ʹ����,����û��������
    return res;
}

// ��ʼ��BC26
uint8_t BC26_Init(void)
{
    RS_INFO("��ʼ��������\r\n");

    if (BC26_send_cmd((uint8_t *)"AT+QRST=1", (uint8_t *)"BROM", 300, 60))
    {
        return 1;
    }
    RS_INFO("�����ɹ�\r\n"); // ����Ƿ������
    if (BC26_send_cmd((uint8_t *)"ATE0", (uint8_t *)"OK", 300, 60))
    {
        return 1;
    }
    RS_INFO("�رջ��Գɹ�\r\n"); // ����Ƿ������
    if (BC26_send_cmd((uint8_t *)"AT", (uint8_t *)"OK", 300, 60))
    {
        return 1;
    }
    RS_INFO("���ӳɹ�\r\n"); // ����Ƿ������
    if (BC26_send_cmd((uint8_t *)"AT+CFUN=1", (uint8_t *)"OK", 300, 60))
    {
        return 2;
    }
    RS_INFO("MT���óɹ�\r\n"); // ����MT��������
    if (BC26_send_cmd((uint8_t *)"AT+CIMI", (uint8_t *)"460", 300, 60))
    {
        return 1;
    }
    RS_INFO("��ȡ���ųɹ�\r\n"); // ��ȡ����
    if (BC26_send_cmd((uint8_t *)"AT+CGATT=1", (uint8_t *)"OK", 300, 60))
    {
        return 1;
    }
    RS_INFO("��������ɹ�\r\n"); // ��������, PDP
    if (BC26_send_cmd((uint8_t *)"AT+CGATT?", (uint8_t *)"+CGATT: 1", 300, 60))
    {
        return 1;
    }
    RS_INFO("���缤���ѯ�ɹ�\r\n"); // ��������
    if (BC26_send_cmd((uint8_t *)"AT+CESQ", (uint8_t *)"OK", 300, 60))
    {
        return 1;
    }
    RS_INFO("�鿴�ɹ�\r\n"); // �鿴ESQ

    RS_INFO("BC26��ʼ���ɹ�\r\n");
    return 0;
}

/*MQTT��ʼ��*/
uint8_t MQTT_Init(void)
{
    uint8_t buff[200] = {0};

    // ���Ͱ�������Ԫ��
    RS_INFO("׼�����Ͱ�������Ԫ��\r\n");
    sprintf((char *)buff, "AT+QMTCFG=\"ALIAUTH\",0,\"%s\",\"%s\",\"%s\"", ProductKey, DeviceName, DeviceSecret);
    if (BC26_send_cmd2_sever(buff, (uint8_t *)"OK", 100, 100))
    {
        RS_INFO("���Ͱ�������Ԫ��ʧ�ܣ�׼������\r\n");
        return 1;
    }
    RS_INFO("���Ͱ�������Ԫ��ɹ�\r\n");

    // ���Ӱ����Ʒ�����
    RS_INFO("׼�����Ӱ����Ʒ�����\r\n");
    memset(buff, 0, 200); // �����ʱ�洢�ռ�
    sprintf((char *)buff, "AT+QMTOPEN=0,\"%s\",%s", Domain, Port);
    RS_INFO("����������: %s\r\n", buff);
    if (BC26_send_cmd2_sever(buff, (uint8_t *)"+QMTOPEN: 0,0", 1000, 200))
    {
        RS_INFO("���Ӱ����Ʒ�����ʧ�ܣ�׼������\r\n");
        return 1;
    }
    RS_INFO("���Ӱ����Ʒ������ɹ�\r\n");

    // �����豸
    RS_INFO("׼�������豸\r\n");
    memset(buff, 0, 200); // �����ʱ�洢�ռ�
    sprintf((char *)buff, "AT+QMTCONN=0,\"%s\"", ConnectID);
    RS_INFO("�豸����: %s\r\n", buff);
    if (BC26_send_cmd2_sever(buff, (uint8_t *)"+QMTCONN: 0,0,0", 100, 100))
    {
        RS_INFO("�����豸ʧ�ܣ�׼������\r\n");
        return 1;
    }
    RS_INFO("�����豸�ɹ�\r\n");

    // ��������
    RS_INFO("׼����������\r\n");
    memset(buff, 0, 200); // �����ʱ�洢�ռ�
    sprintf((char *)buff, "AT+QMTSUB=0,1,\"%s\",0", SubTopic);
    if (BC26_send_cmd2_sever(buff, (uint8_t *)"+QMTSUB: 0,1,0,1", 100, 100))
    {
        RS_INFO("����ʧ�ܣ�׼������\r\n");
        return 1;
    }
    RS_INFO("���ĳɹ�\r\n");

    return 0;
}

// ���ӷ�����
uint8_t WiFi_Connect_IoTServer(void)
{
    LED0 = 0; // ָʾ��������ʾ��������Ͽ�

    TIM_Cmd(TIM2, DISABLE); // �ر�TIM2

    RS_INFO("׼����ʼ��BC26\r\n");

    if (BC26_Init())
    {
        RS_INFO("BC26��ʼ��ʧ�ܣ�׼������\r\n");
        return 1;
    }
    RS_INFO("BC26��ʼ���ɹ�\r\n");

    RS_INFO("׼�����ӷ�����\r\n");
    if (MQTT_Init())
    {
        RS_INFO("���ӷ�����ʧ�ܣ�׼������\r\n");
        return 1;
    }
    RS_INFO("���ӷ������ɹ�\r\n");

    Connect_flag = 1; // ���ӳɹ�

    Post_Send();         // ���͵�ѹֵ��������
    Asso_QMTPUB_event(); // ���ͼ��ģʽ��������

    TIM_Cmd(TIM2, ENABLE); // ��TIM2
    LED0 = 1;              // ָʾ���𣬱�ʾ�����������
    return 0;
}

// ������������, ��������㱨��ѹ
void Asso_QMTPUB(void)
{
    uint8_t buff[200] = {0};

    sprintf((char *)buff, "AT+QMTPUB=0,1,1,0,\"%s\",\"{\"params\":{\"%s\":%s}}\"\r\n", PubTopic, Param_Post, Value);

    //    RS_INFO( "�����������⣺AT+QMTPUB=0,1,1,0,\"%s\",\"{\"params\":{\"%s\":%s}}\"\r\n", PubTopic, Param_Post, Value );

    if (BC26_send_cmd2_sever(buff, (uint8_t *)"+QMTPUB: 0,1,0", 100, 300))
    {
        Connect_flag = 0;
    } // ���Ӷ���
    else
        Connect_flag = 1;
}

// �����¼�����, ��������ϱ����ģʽ,ͬʱ����������
// �����¼�����
void Asso_QMTPUB_event(void)
{
    uint8_t buff[200] = {0};

    sprintf((char *)buff, "AT+QMTPUB=0,1,1,0,\"%s\",\"{\"params\":{\"%s\":%d}}\"\r\n", PubTopic, Param_Set, Check_Mode);
    RS_INFO("�����������⣺AT+QMTPUB=0,1,1,0,\"%s\",\"{\"params\":{\"%s\":%d}}\"\r\n", PubTopic, Param_Set, Check_Mode);

    if (BC26_send_cmd2_sever(buff, (uint8_t *)"+QMTPUB: 0,1,0", 100, 300))
    {
        Connect_flag = 0;
    } // ���Ӷ���
    else
        Connect_flag = 1;
}

// ���͵͵�ѹ����
void Asso_QMTPUB_remind(void)
{
    uint8_t buff[200] = {0};

    sprintf((char *)buff, "AT+QMTPUB=0,1,1,0,\"%s\",\"{\"params\":{\"error_info\":1}}\"\r\n", Remind_Topic);
    //    RS_INFO( "�����������⣺AT+QMTPUB=0,1,1,0,\"%s\",\"{params:{Error:1}}\"\r\n",Remind_Topic );

    if (BC26_send_cmd2_sever(buff, (uint8_t *)"+QMTPUB: 0,1,0", 100, 300))
    {
        Connect_flag = 0;
    } // ���Ӷ���
    else
        Connect_flag = 1;
}

// �������ݸ�������
void Post_Send(void)
{
    u16 adcx;
    float temp;
    static float buff = 0; // ��¼��һ������

    adcx = Get_Adc_Average(ADC_Channel_1, 10); // �ɼ���ѹֵ
    temp = (float)(adcx * (3.3 / 4096)) * 10;
    sprintf(Value, "%.2f", temp);
    Asso_QMTPUB(); // ���͵�ѹֵ��������

    if (fabs(temp - buff) >= 0.05)
    {
        buff = temp;
        RS_INFO("�ϱ�����\r\n");
        if (temp < 11.5)
            Asso_QMTPUB_remind(); // ��ѹ����,���͵�ѹ����
    }
}

#ifndef __BC26_H
#define __BC26_H
#include "sys.h"
// e1c8b1319197a20926965e701ba02449     Device_001
// d137af895c4d8efaf876c2b463d37b91     Device_002
// cb1727085ace4eebddb7eeea0cc88f59     Device_003
// 3ee2d52215e30855a07a3df2431f7cee     Device_004

#define ProductKey "gtua5daFrI6"						// ��Ʒ��Կ
#define DeviceName "voltage_device_001"					// �豸��
#define DeviceSecret "df2c8974886ee687faef95edb9107b8c" // �豸��Կ

#define Domain "iot-06z00bp0nwmb9tp.mqtt.iothub.aliyuncs.com"			  // ����a188XTcAC3R.iot-as-mqtt.cn-shanghai.aliyuncs.com    101.133.196.24
#define Port "1883"														  // �˿ں�
#define ConnectID "voltage_device_001&gtua5daFrI6"						  // �豸��
#define PubTopic "/gtua5daFrI6/voltage_device_001/user/update"			  // ��������
#define SubTopic "/gtua5daFrI6/voltage_device_001/user/update" // ��������
#define Remind_Topic "/gtua5daFrI6/voltage_device_001/user/update"		  // �¼�����
#define Param_Post "voltage"											  // ����������
#define Param_Set "check_mode"											  // ����������

//* ��������Ԫ�� */
// #define ProductKey      "a188XTcAC3R"                                    			 	//��Ʒ��Կ
// #define DeviceName   	"test2"                                                      	//�豸��
// #define DeviceSecret    "e0b839e5265af926fccd0f2e97bc5937"                              //�豸��Կ

// #define Domain          "a188XTcAC3R.iot-as-mqtt.cn-shanghai.aliyuncs.com"           	//���� 47.102.164.191
// #define Port            "1883"                                            			 	//�˿ں�
// #define ConnectID       "test2&a188XTcAC3R"                                             //�豸��
// #define PubTopic	    "/sys/a188XTcAC3R/test2/thing/event/property/post" 	 	        //��������
// #define SubTopic	    "/sys/a188XTcAC3R/test2/thing/service/property/set"	 	//��������
// #define Remind_Topic	"/sys/a188XTcAC3R/test2/thing/event/LowVoltageAlarm/post"	    //�¼�����
// #define Param_Post      "voltage"                                  					    //����������
// #define Param_Set       "Check_Mode"                                  				 	//����������
///* ��������Ԫ�� */
// #define ProductKey      "a1JvuQO1Mb0"                                    			 	//��Ʒ��Կ
// #define DeviceName   	"Device_001"                                                      	//�豸��
// #define DeviceSecret    "e1c8b1319197a20926965e701ba02449"                              //�豸��Կ

// #define Domain          "a1JvuQO1Mb0.iot-as-mqtt.cn-shanghai.aliyuncs.com"           	            //����a188XTcAC3R.iot-as-mqtt.cn-shanghai.aliyuncs.com    101.133.196.24
// #define Port            "1883"                                            			 	//�˿ں�
// #define ConnectID       "Device_001&a1JvuQO1Mb0"                                                 	//�豸��
// #define PubTopic	    "/sys/a1JvuQO1Mb0/Device_001/thing/event/property/post" 	 	//��������
// #define SubTopic	    "/sys/a1JvuQO1Mb0/Device_001/thing/service/property/set"	 	//��������
// #define Remind_Topic	"/sys/a1JvuQO1Mb0/Device_001/thing/event/LowVoltageAlarm/post"	//�¼�����
// #define Param_Post      "voltage"                                  					 	//����������
// #define Param_Set       "Check_Mode"                                      				 	//����������
// extern char Value[10];                                                         			 //��ѹֵ

#define IP

extern char Value[10];
extern unsigned int Connect_flag;
extern unsigned int Check_Mode;

uint8_t BC26_Init(void);
uint8_t WiFi_Connect_IoTServer(void);
void Post_Send(void);
void Asso_QMTPUB_event(void);

typedef struct
{
	u8 CSQ;		  // �����ź�ֵ
	u8 NetStatus; // ����״ֵ̬
} BC26;

#endif

//------------------------------------------------------------
#include <stdio.h>
#include <string.h>
#include "CH563SFR.H"
#include "SYSFREQ.H"
#include "CH563NET.H"
#include "CH563.H"
#include "Clouds_Protocal.h"
#include "UartData.h"
#include "SubMQ_Server.h"
#include "DNS.h"
#include "Encrypt.h"

extern void Get_Device_Ver_Num(void);
UINT8 SubMQ_Topic_Flag=0;
UINT8 Sub_MQ_DNS_Parse_Flag=0;//子服务局解析标志，在云端下发写时为1
UINT8 GateWay_Link_Sub_MQ_Cnt=0;
UINT8 Link_Sub_MQ_Start_Flag=0;	//连接子MQ启动标志

UINT8 url_sub[128];// = "mqtt-cn-mp90q7tra01.mqtt.aliyuncs.com";   
UINT8 TEA_KEY[16]={"EDB8F6180892F7CB"};
uint32_t EN_TEA_KEY[4];
uint32_t UINT8_To_UINT32_C(UINT8 *by)
{
	uint32_t val=0;

	val=by[0]*0x1000000+by[1]*0x10000+by[2]*0x100+by[3];

	return val;
}
void Get_UINT32_Tea_Key(UINT8 *in,uint32_t *out)
{
	UINT8 temp1[4];
	UINT8 temp2[4];
	UINT8 temp3[4];
	UINT8 temp4[4];
	UINT8 i=0;

	for(i=0;i<16;i++)
	{
		if(in[i]>='a'&&in[i]<='z')
		{
			in[i]-=32;
		}
	}
	for(i=0;i<4;i++)
	{
		temp1[i]=in[i];
	}
	out[0]=UINT8_To_UINT32_C(temp1);
	for(i=0;i<4;i++)
	{
		temp2[i]=in[i+4];
	}
	out[1]=UINT8_To_UINT32_C(temp2);
	for(i=0;i<4;i++)
	{
		temp3[i]=in[i+8];
	}
	out[2]=UINT8_To_UINT32_C(temp3);
	for(i=0;i<4;i++)
	{
		temp4[i]=in[i+12];
	}
	out[3]=UINT8_To_UINT32_C(temp4);
}
extern uint32_t M_Tea_Key[4];
void Rsub_Send_Dat_To_Clouds(UINT8 *device_id,UINT8 *FuncType,UINT8 *dat,UINT8 *msg_buf,UINT8 len)
{
	UINT8 send_buf[message_head+message_tail+32];
	UINT16 send_len=0;
	UINT8 i=0;

	send_len=message_head+message_tail+len-12;

	send_buf[0]=0xfd;
	send_buf[1]=send_len>>8;
	send_buf[2]=send_len&0xff;

	for(i=0;i<6;i++)
	{
		send_buf[i+3]=MACAddr[i];	
	}
	
	for(i=0;i<12;i++)
	{
		send_buf[9+i]=device_id[i];
	}
	send_buf[21]=FuncType[0];  //功能类型
	send_buf[22]=FuncType[1];  //功能指令
	//有效数据
	for(i=0;i<len;i++)
	{
		send_buf[23+i]=dat[i];
	}

	send_buf[send_len-32-2]=0x01;//协议版本
	
//	for(i=0;i<32;i++)
//	{
//		send_buf[send_len-32-1+i]=msg_buf[i];
//	}
	for(i=0;i<20;i++)//mark2019_09_16
	{
		send_buf[send_len-20-1+i]=msg_buf[i];
	}

	send_buf[send_len-1]=Clouds_Check_Sum(send_buf,(send_len-1));

	//MQTT_Public_Pack(Push_Topics,send_buf,send_len);
//	if(GateWay_Link_Sub_MQ_Flag==1)
//	{
		send_len=encrypt((unsigned char*)send_buf,send_len,(uint32_t *)M_Tea_Key);	
		MQTT_Public_Pack(Push_Topics,send_buf,send_len);
//	}
//	else
//	{
//		#if TEA_ENCRYPT == 1
//		Clouds_sendPacketBuffer(send_buf,send_len);
//		#else
//		MQTT_Public_Pack(Push_Topics,send_buf,send_len);
//		#endif
//	}
}
//网关推送数据，连接MQ服务器
void Get_MQ_Server_Msg_LinkNet(void)
{
	UINT8 dat_buf[14]={"get mqtt msg"};
	UINT8 dat_len=0;
	UINT8 i=0,j=0;
	UINT8 dev_id[12];
	UINT8 msg_buf[32];
	UINT8 functype[2]; 
	UINT8 tmp1[8];
	UINT8 tmp2[8];
	UINT8 RandomDat[2];
//	UINT8 tbuf[16]={"12345678"};
	UINT8 buflen=0;
	UINT8 temp1[4];
	UINT8 temp2[4];
	UINT8 temp3[4];
	UINT8 temp4[4];

	if(GateWay_Link_Sub_MQ_Flag==1)
	{
		dat_len=14;
		
		for(i=0;i<12;i++)
		{
			dev_id[i]=0;
		}
			
		functype[0]=0x17;	//功能类型
		functype[1]=0x03;	//功能指令	
		
		Back_To_Clouds_Msg_ID(msg_buf);		
		
		for(i=0;i<2;i++)
		RandomDat[i] = rand();	
		
		for(i=0;i<2;i++)
		{
			dat_buf[12+i]=RandomDat[i];		
		}			

		//Send_Dat_To_Clouds(dev_id,functype,dat_buf,msg_buf,dat_len);
		Rsub_Send_Dat_To_Clouds(dev_id,functype,dat_buf,msg_buf,dat_len);
//		USART1_Send_Byte(0xbb);
//		USART1_Send_Byte(0xbb);
//		USART1_Send_Byte(0xbb);
//		Uart1_SendBuffer(dat_buf,dat_len);
		tmp1[0]=0xff-RandomDat[0];
		for(i=0;i<6;i++)
		{
			tmp1[i+1]=MACAddr[i];
		}
		tmp1[7]=0xff-RandomDat[1];

		tmp2[0]=tmp1[0];
		tmp2[1]=tmp1[1]+RandomDat[1];
		tmp2[2]=tmp1[2]+RandomDat[1];
		tmp2[3]=tmp1[3]+RandomDat[1];
		tmp2[4]=tmp1[4]+RandomDat[0];
		tmp2[5]=tmp1[5]+RandomDat[0];
		tmp2[6]=tmp1[6]+RandomDat[0];
		tmp2[7]=tmp1[7];

		//转化为字符串	
		/****************************************************************/
		j=0;
		for(i=0;i<8;i++)
		{
			if(((tmp2[i]&0xf0)>>4)>=0&&((tmp2[i]&0xf0)>>4)<=9)
			{
				TEA_KEY[j]=0x30+((tmp2[i]&0xf0)>>4);	
			}
			else if(((tmp2[i]&0xf0)>>4)>=0x0A&&((tmp2[i]&0xf0)>>4)<=0x0F)
			{
				TEA_KEY[j]='A'+(((tmp2[i]&0xf0)>>4)-0xA);
			}
			//-------------------------------------------------
			if((tmp2[i]&0x0f)>=0&&(tmp2[i]&0x0f)<=9)
			{
				TEA_KEY[j+1]=0x30+(tmp2[i]&0x0f);
			}
			else if((tmp2[i]&0x0f)>=0xA&&(tmp2[i]&0x0f)<=0xF)
			{
				TEA_KEY[j+1]='A'+((tmp2[i]&0x0f)-0xA);		
			}
		
		
			j+=2;
		}																								
		/****************************************************************/
		for(i=0;i<16;i++)
		{
			if(TEA_KEY[i]>='a'&&TEA_KEY[i]<='z')
			{
				TEA_KEY[i]-=32;
			}
		}
		for(i=0;i<4;i++)
		{																	    
			temp1[i]=TEA_KEY[i];
		}
		EN_TEA_KEY[0]=UINT8_To_UINT32_C(temp1);
		for(i=0;i<4;i++)
		{
			temp2[i]=TEA_KEY[i+4];
		}
		EN_TEA_KEY[1]=UINT8_To_UINT32_C(temp2);
		for(i=0;i<4;i++)
		{
			temp3[i]=TEA_KEY[i+8];
		}
		EN_TEA_KEY[2]=UINT8_To_UINT32_C(temp3);
		for(i=0;i<4;i++)
		{
			temp4[i]=TEA_KEY[i+12];
		}
		EN_TEA_KEY[3]=UINT8_To_UINT32_C(temp4);
//		Uart1_SendBuffer(TEA_KEY,16);
//		encrypt(tbuf,8,EN_TEA_KEY);
//		USART1_Send_Byte(0xaa);
//		USART1_Send_Byte(0xaa);
//		USART1_Send_Byte(0xaa);
//		USART1_Send_Byte(0xaa);
//		Uart1_SendBuffer(tbuf,9);
//		USART1_Send_Byte(0xaa);
//		USART1_Send_Byte(0xaa);
//		USART1_Send_Byte(0xaa);
//		USART1_Send_Byte(0xaa);
//		decrypt(tbuf,9,EN_TEA_KEY);
//		Uart1_SendBuffer(tbuf,8);
	}
}
/*******************************************************************************
* Function Name  : Link_SubMQ_Server_Process
* Description    : 连接子MQ服务器
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void Link_SubMQ_Server_Process(void)
{
	UINT8 i=0;
	UINT8 subMQ_IP[4]={0,0,0,0};
	UINT8 Rece=0;

	//获取mq的msg
	if(GateWay_Link_Sub_MQ_Cnt>20)//old=12
	{
		GateWay_Link_Sub_MQ_Cnt=0;
		MQTT_subscrib_Pack(Subscribe_Topics,meassage);
		Delay_ms(30);
		Get_MQ_Server_Msg_LinkNet();
	}	

	if(Sub_MQ_DNS_Parse_Flag==1)//子服务局解析标志，在云端下发写时为1
	{
		//i = DnsQuery(0,url_sub,subMQ_IP);  
		i=1;
		if(i)
		{ 
			Sub_MQ_DNS_Parse_Flag=0;
//			MyAutoConnectDESIP[0]=subMQ_IP[0];
//			MyAutoConnectDESIP[1]=subMQ_IP[1];
//			MyAutoConnectDESIP[2]=subMQ_IP[2];
//			MyAutoConnectDESIP[3]=subMQ_IP[3];
//			MyAutoConnectDESIP[0]=47;
//			MyAutoConnectDESIP[1]=106;
//			MyAutoConnectDESIP[2]=218;
//			MyAutoConnectDESIP[3]=29;
//			USART1_Send_Byte(0xff);
//			USART1_Send_Byte(0xff);
//			USART1_Send_Byte(0x11);
//			USART1_Send_Byte(MyAutoConnectDESIP[0]);
//			USART1_Send_Byte(MyAutoConnectDESIP[1]);
//			USART1_Send_Byte(MyAutoConnectDESIP[2]);
//			USART1_Send_Byte(MyAutoConnectDESIP[3]);

			Link_Sub_MQ_Start_Flag=1; //连接子MQ启动标志
			Heart_Start_Flag=1;	
			
			GateWay_Online_Status_Flag[1]=1; //子MQ服务连上
			SubMQ_Topic_Flag=1;

			CH563NET_SocketClose(TCP_SOCKET_ID,TCP_CLOSE_NORMAL);

//			Get_Device_Ver_Num();
		}		
	}
}
extern char MQTTUserName_bak[30]; //
extern char MQTTPassWord_bak[70];//
extern char ClientID[24]; 
extern UINT8 UserName_Len;
extern UINT8 PassWord_Len;
extern UINT8 My_MACAddr[12];
//重新连接中心服
void Reconnect_MQ_Process(void)
{
	UINT8 i=0;
	UINT8 Temp_Topics[23]="ccc/center/"; 

	for(i=0;i<12;i++)	    
	{
		Temp_Topics[11+i]=My_MACAddr[i];		
	}
	for(i=0;i<32;i++)
	{
		Push_Topics[i]='\0';
	}
	for(i=0;i<23;i++)
	{
		Push_Topics[i]=Temp_Topics[i];
	}
	NVIC_SystemReset();
}

void Get_Mstar_MQ_Topics(void)
{
	UINT8 i=0;
	UINT8 Temp_Topics[23]={"ccc/center/"};
	UINT8 relink_cliendid[10]={"GID_CCC@@@"}; 
	UINT8 relink_username[4]={"gw01"};	  
	UINT8 relink_userpasswd[16]={"FKOt*vXk8x&T#9B0"};
	UINT8 Subscribe_Topics_Head[4]={"ccc/"};

	for(i=0;i<12;i++)
		Temp_Topics[11+i]=My_MACAddr[i];		
	for(i=0;i<32;i++)
		Push_Topics[i]='\0';
	for(i=0;i<23;i++)
		Push_Topics[i]=Temp_Topics[i];

	//主客户端ID
	//ClientID[4]='C';
	for(i=0;i<24;i++)ClientID[i]='\0';	 //GID_MCC@@@
	for(i=0;i<10;i++)
	{
		ClientID[i]=relink_cliendid[i];		
	}
	for(i=0;i<12;i++)
	{
		ClientID[10+i]=My_MACAddr[i];//MACAddr[i];		
	}
	//用户名和密码
	for(i=0;i<30;i++)MQTTUserName[i]='\0';	 //LTAIrRv0erUH18DL
	for(i=0;i<4;i++)
	{
		MQTTUserName[i]=relink_username[i];		
	}
	
	for(i=0;i<70;i++)MQTTPassWord[i]='\0'; //V4PufgNlLziw38V/0cUD1Js4dWg=
	for(i=0;i<16;i++)
	{
		MQTTPassWord[i]=relink_userpasswd[i];			
	}
	UserName_Len=4;
	PassWord_Len=16;
	for(i=0;i<30;i++)
	{
		MQTTUserName_bak[i]=MQTTUserName[i];	
	}
	for(i=0;i<70;i++)
	{
		MQTTPassWord_bak[i]=MQTTPassWord[i];	
	}
	//发布和推送
//	Push_Topics[0]='c';
//	Subscribe_Topics[0]='c';
	for(i=0;i<20;i++)Subscribe_Topics[i]='\0'; 
	for(i=0;i<4;i++)
	{
		Subscribe_Topics[i]=Subscribe_Topics_Head[i];
	}
	for(i=0;i<12;i++)
	{
		Subscribe_Topics[4+i]=My_MACAddr[i];	
	}
	//ip和端口
	MyAutoConnectDESIP[0]=MyAutoConnectDESIP_bak[0];
	MyAutoConnectDESIP[1]=MyAutoConnectDESIP_bak[1];
	MyAutoConnectDESIP[2]=MyAutoConnectDESIP_bak[2];
	MyAutoConnectDESIP[3]=MyAutoConnectDESIP_bak[3];
	MyAutoConnectPort=21883;
	//-------------------------------------------------
	//全局变量初始化
	GateWay_Link_Sub_MQ_Flag=0;
	Link_Sub_MQ_Start_Flag=0; //连接子MQ启动标志
	Heart_Start_Flag=0;	
	Heart_Cnt=0;
	
	for(i=0;i<5;i++)
	{	
		GateWay_Online_Status_Flag[i]=0; //子MQ服务连上
	}
	SubMQ_Topic_Flag=0;
	
	for(i=0;i<10;i++)
	{
		MyBuf[i]=0;	
	}

}












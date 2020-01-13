//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <net/if.h>        //for struct ifreq

#include "Get_NetInfo.h"

unsigned char  MACAddr[6] = {0x02,0x03,0x04,0x05,0x16,0x17};                                                          
unsigned char IPAddr[4] = {0,0,0,0};                                         
unsigned char GWIPAddr[4] = {192,168,1,1};                                             
unsigned char IPMask[4] = {255,255,255,0};     

unsigned char My_MACAddr[12];

int get_mac(char * mac, int len_limit)    //返回值是实际写入char * mac的字符个数（不包括'\0'）
{
    struct ifreq ifreq;
    int sock;
	unsigned char i;

    if ((sock = socket (AF_INET, SOCK_STREAM, 0)) < 0)
    {
        perror ("socket");
        return -1;
    }
    strcpy (ifreq.ifr_name, "eth0");    //Currently, only get eth0

    if (ioctl (sock, SIOCGIFHWADDR, &ifreq) < 0)
    {
        perror ("ioctl");
        return -1;
    }
	for(i=0;i<6;i++)
	MACAddr[i] = ifreq.ifr_hwaddr.sa_data[i];
    
    return snprintf (mac, len_limit, "%X:%X:%X:%X:%X:%X", (unsigned char) ifreq.ifr_hwaddr.sa_data[0], (unsigned char) ifreq.ifr_hwaddr.sa_data[1], (unsigned char) ifreq.ifr_hwaddr.sa_data[2], (unsigned char) ifreq.ifr_hwaddr.sa_data[3], (unsigned char) ifreq.ifr_hwaddr.sa_data[4], (unsigned char) ifreq.ifr_hwaddr.sa_data[5]);
}
//-------------------------------------------------------------------------------
void Get_NetMac(void)
{
    char    szMac[18];
    int     nRtn;
	nRtn = get_mac(szMac, sizeof(szMac));
    if(nRtn > 0)
    {
        fprintf(stderr, "MAC ADDR: %s\n", szMac);
    }
}
void Get_NetIP(void)
{

}
void MacHexToStr(void)
{
	unsigned char i,j;
	unsigned char macbuf[12];
	
	/****************************************************************/
	j=0;
	for(i=0;i<6;i++)
	{
		if(((MACAddr[i]&0xf0)>>4)>=0&&((MACAddr[i]&0xf0)>>4)<=9)
		{
			macbuf[j]=0x30+((MACAddr[i]&0xf0)>>4);	
		}
		else if(((MACAddr[i]&0xf0)>>4)>=0x0A&&((MACAddr[i]&0xf0)>>4)<=0x0F)
		{
			macbuf[j]='A'+(((MACAddr[i]&0xf0)>>4)-0xA);
		}
		//-------------------------------------------------
		if((MACAddr[i]&0x0f)>=0&&(MACAddr[i]&0x0f)<=9)
		{
			macbuf[j+1]=0x30+(MACAddr[i]&0x0f);
		}
		else if((MACAddr[i]&0x0f)>=0xA&&(MACAddr[i]&0x0f)<=0xF)
		{
			macbuf[j+1]='A'+((MACAddr[i]&0x0f)-0xA);		
		}

		j+=2;
	}
	/****************************************************************/
	for(i=0;i<12;i++)
	{
		if(macbuf[i]>='a'&&macbuf[i]<='z')
		{
			macbuf[i]-=32;
		}
	}
	for(i=0;i<12;i++)
	{
		My_MACAddr[i]=macbuf[i];
	}
}

/* 方案一 */
/*
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <fcntl.h>
#include <errno.h>
#include <sys/ioctl.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <net/if.h>
#include <linux/sockios.h>
#include <linux/ethtool.h>
*/

/*
 *if_name like "ath0","eth0".Notice:call this function
 *need root privilege.
 *return value:
 *-1 -- error,details can check errno
 *1 -- interface link up
 *0 -- interface link down.
*/
/*
int get_netlink_status(const char *if_name)
{
	int skfd;
	struct ifreq ifr;
	struct ethtool_value edata;

	edata.cmd = ETHTOOL_GLINK;
	edata.data = 0;

	memset(&ifr, 0,sizeof(ifr));
	strncpy(ifr.ifr_name,if_name,sizeof(ifr.ifr_name)-1);
	ifr.ifr_data = (char *)&edata;

	if ((skfd = socket(AF_INET,SOCK_DGRAM,0))<0){
		return -1;
	}
	if(ioctl(skfd,SIOCETHTOOL,&ifr)==-1){
		close(skfd);
		return -1;
	}

	close(skfd);
	return edata.data;
}
*/
/* 方案二 */
//使用ifconfig命令能很方便的查看网卡与网线是否连通
//#ifconfig eth0

//linux系统提供了popen/pclose进程管道让C和shell很方便的交互，下面C代码结合shell命令检测网卡与网线连通状况；


#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define BUFSIZ 256
int GetNetStaus(void)
{
	char buffer[BUFSIZ];
	FILE *read_fp;
	int  chars_read;
	int  ret;

	memset(buffer,0,BUFSIZ);//swconfig dev switch0 port 0 show
	//read_fp = popen("ifconfig eth0 | grep RUNNING","r");//RUNNING
	read_fp = popen("swconfig dev switch0 port 1 show | grep up","r");

	if(read_fp !=NULL){
		chars_read = fread(buffer,sizeof(char),BUFSIZ-1,read_fp);
		if (chars_read > 0){
			ret = 1;
		}
		else{
			ret = 0;//-1;
		}
		pclose(read_fp);
	}
	else{
		ret = -1;
	}

	return ret;
}
void Plan_a_Ethernet_Cable_Sta(void)
{
	int i;

	i=GetNetStaus();
	printf("Netsta1=:%d\n",i);
	if(i==1){
		printf("网线已连接\n");	
	}
	else{
		printf("网线未连接\n");
	}
	//i=get_netlink_status("eth0");
	//printf("Netsta2=:%d\n",i);
}





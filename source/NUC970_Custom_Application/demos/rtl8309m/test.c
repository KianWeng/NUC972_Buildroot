#include <stdio.h>  
#include <fcntl.h>  
#include <stdlib.h>  
#include <string.h>  
#include <sys/types.h>  
#include <sys/stat.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <rtk_api_ext.h>

#define RTL8309_MAGIC 'R'  //å½“ç„¶æˆ‘ä»¬ä¹Ÿå¯ä»¥å®šä¹‰ä¸€ä¸ªç›¸åº”çš„å¤´æ–‡ä»¶ï¼ŒæŠŠioctlçš„cmdæ”¾è¿›é‡Œé¢ï¼Œç„¶åå†includeè¿› æ¥
#define RTL8309M_GET_REG_DATA   _IO(RTL8309_MAGIC, 0x20)
#define RTL8309M_SET_REG_DATA   _IO(RTL8309_MAGIC, 0x21)
#define RTL8309M_READ_REG       _IO(RTL8309_MAGIC, 0x22)
#define RTL8309M_WRITE_REG      _IO(RTL8309_MAGIC, 0x23)

struct rtl8309m_register_data {
	unsigned int phyad;
	unsigned int regad;
};

int main(void)
{
    int ret;
	rtk_portmask_t mbrmsk1, mbrmsk2, untagmsk1, untagmsk2;
	rtk_fid_t fid1,fid2;

	//³õÊ¼»¯½»»»»ú
    ret = rtk_switch_init();
	if(ret < 0){
		printf("rtk switch init fail,just exit!");
		exit(0);
	}

	//³õÊ¼»¯½»»»vlan¹¦ÄÜ
	rtk_vlan_init();

	mbrmsk1.bits[0] = 0x143;
	untagmsk1.bits[0] = 0x1FF;
	fid1 = 1;
	rtk_vlan_set(1, mbrmsk1, untagmsk1, fid1);

	mbrmsk2.bits[0] = 0x1C;
	untagmsk2.bits[0] = 0x1FF;
	fid2 = 1;
	rtk_vlan_set(2, mbrmsk2, untagmsk2, fid2);
	
	//ÊÍ·Å½»»»»ú×ÊÔ´
	rtk_switch_release();
	
    return 0;
}

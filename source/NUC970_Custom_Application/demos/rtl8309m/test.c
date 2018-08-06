#include <stdio.h>  
#include <fcntl.h>  
#include <stdlib.h>  
#include <string.h>  
#include <sys/types.h>  
#include <sys/stat.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <rtk_api_ext.h>

#define RTL8309_MAGIC 'R'  //当然我们也可以定义一个相应的头文件，把ioctl的cmd放进里面，然后再include进 来
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
    rtk_port_phy_data_t data;

	//��ʼ��������
    ret = rtk_switch_init();
	if(ret < 0){
		printf("rtk switch init fail,just exit!");
		exit(0);
	}

	//��ʼ������vlan����
	rtk_vlan_init();
	rtk_vlan_destroy(1);
	
	mbrmsk1.bits[0] = 0x15E;
	untagmsk1.bits[0] = 0x15E;
	fid1 = 0;
	rtk_vlan_set(100, mbrmsk1, untagmsk1, fid1);

	mbrmsk2.bits[0] = 0x120;
	untagmsk2.bits[0] = 0x120;
	fid2 = 0;
	rtk_vlan_set(101, mbrmsk2, untagmsk2, fid2);
	
	mbrmsk2.bits[0] = 0x1FF;
	untagmsk2.bits[0] = 0x1FF;
	fid2 = 0;
	rtk_vlan_set(102, mbrmsk2, untagmsk2, fid2);
	
	rtk_vlan_portPvid_set(1,100,0);
	rtk_vlan_portPvid_set(2,100,0);
	rtk_vlan_portPvid_set(3,100,0);
	rtk_vlan_portPvid_set(4,100,0);
	rtk_vlan_portPvid_set(6,100,0);
	rtk_vlan_portPvid_set(5,101,0);
	rtk_vlan_portPvid_set(8,102,0);
	
    rtk_port_phyReg_get(1,1,&data);
    printf("port 1 status is 0x%x.\n",data);
	rtk_port_phyReg_get(2,1,&data);
    printf("port 2 status is 0x%x.\n",data);
	rtk_port_phyReg_get(3,1,&data);
    printf("port 3 status is 0x%x.\n",data);
	rtk_port_phyReg_get(4,1,&data);
    printf("port 4 status is 0x%x.\n",data);
	rtk_port_phyReg_get(5,1,&data);
    printf("port 5 status is 0x%x.\n",data);
	rtk_port_phyReg_get(6,1,&data);
    printf("port 6 status is 0x%x.\n",data);
	//�ͷŽ�������Դ
	rtk_switch_release();
	
    return 0;
}

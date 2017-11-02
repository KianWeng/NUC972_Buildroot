#include <stdio.h>
#include <linux/input.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <poll.h>

/*
struct input_event {
        struct timeval time;
        __u16 type;
        __u16 code;
        __s32 value;
};
*/
/*
Event types
#define EV_SYN                  0x00
#define EV_KEY                  0x01
#define EV_REL                  0x02
#define EV_ABS                  0x03
*/
/*
 Relative axes

#define REL_X                   0x00
#define REL_Y                   0x01
#define REL_Z                   0x02
#define REL_RX                  0x03
#define REL_RY                  0x04
#define REL_RZ                  0x05
#define REL_HWHEEL              0x06
#define REL_DIAL                0x07
#define REL_WHEEL               0x08
#define REL_MISC                0x09
#define REL_MAX                 0x0f
#define REL_CNT                 (REL_MAX+1)
*/

#define INFTIM -1

int main(void)
{
	//定义一个结构体变量用来描述input事件
	struct input_event event_key ;
	struct pollfd pfd[1];
	int fd;
	int nfd = 1;
	int len;
	int type ;
	int buffer[10]={0};
	
	//打开input设备的事件节点
	fd = open("/dev/input/event0",O_RDWR);
	if(-1 == fd){
		printf("GPIO_BUTTON:Open gpio key input event fail!\n");
		return -1 ;
	}

	pfd[0].fd = fd;
	pfd[0].events = POLLIN;
	
	while(1){
		
		if(poll(pfd, nfd, INFTIM) > 0){
			//有事件上报，读取事件
			len = read(fd ,&event_key ,sizeof(event_key));
			if(len < 0){
				printf("GPIO_BUTTON:Read input event error\n");
			}else{
				//判断事件类型，并打印键码
				switch(event_key.type){
					case EV_SYN:
						 printf("sync!\n");
						 break ;
					case EV_KEY:
						//按键事件 keycode 249
						if(event_key.code == 249 && event_key.value == 1){
							printf("GPIO_BUTTON:Recovery key pressed,now we will recovery the system.\n");
						}
						break;
					defalut:
						break ;
				}
			}
		}
	}	
	return 0 ;
}
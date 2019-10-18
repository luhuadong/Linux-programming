/*================================================================
*   Copyright (C) 2019 Guangzhou Firefly Ltd. All rights reserved.
*   
*   文件名称：get_display_info.c
*   创 建 者：luhuadong
*   创建日期：2019年10月18日
*   描    述：
*
================================================================*/


#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <linux/fb.h>

int main(void)
{
	int fd;

	/* 打开fb设备文件 */
	fd = open("/dev/fb0", O_RDWR);

	if(-1 == fd) {
		perror("Open framebuffer");
		return -1;
	}

	/* 获取fix屏幕信息：获取命令为FBIOGET_FSCREENINFO */
	struct fb_fix_screeninfo fixInfo;

	if(-1 == ioctl(fd, FBIOGET_FSCREENINFO, &fixInfo)) {
	
		perror("Get fscreeninfo");
		close(fd);
		return -2;
	}

	/* 打印fix信息 */

	/* 厂商id信息 */
	printf("id = %s\n", fixInfo.id); 

	/* 这里获取一行像素所需的空间 
	 * 该空间大小是出厂时就固定的了
	 * 厂商会对一行像素字节进行对齐 */
	printf("line length = %d\n", fixInfo.line_length); 

	/* 获取var屏幕的信息：获取命令为FBIOGET_VSCREENINFO */
	struct fb_var_screeninfo varInfo;

	if(ioctl(fd, FBIOGET_VSCREENINFO, &varInfo) == -1) {
	
		perror("Get var screen failed\n");
		close(fd);
		return -3;
	}

	/* 打印var信息 */
	printf("xres = %d, yres = %d\n", varInfo.xres, varInfo.yres);
	printf("bpp = %d\n", varInfo.bits_per_pixel);
	printf("R: offset = %d, length = %d\n", varInfo.red.offset, varInfo.red.length);
	printf("G: offset = %d, length = %d\n", varInfo.green.offset, varInfo.green.length);
	printf("B: offset = %d, length = %d\n", varInfo.blue.offset, varInfo.blue.length);
	printf("A: offset = %d, length = %d\n", varInfo.transp.offset, varInfo.transp.length);

	close(fd);

	return 0;
}

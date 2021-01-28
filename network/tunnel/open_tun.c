#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <net/if.h>
#include <linux/if_tun.h>
#include <sys/msg.h>

int open_tun (const char *dev, char *actual, int size)
{
    struct ifreq ifr;
    int fd;
    char *device = "/dev/net/tun";
    if ((fd = open (device, O_RDWR)) < 0) //创建描述符
        msg (M_ERR, "Cannot open TUN/TAP dev %s", device);
    
    memset (&ifr, 0, sizeof (ifr));
    ifr.ifr_flags = IFF_NO_PI;
    if (!strncmp (dev, "tun", 3)) {
        ifr.ifr_flags |= IFF_TUN;
    }
    else if (!strncmp (dev, "tap", 3)) {
        ifr.ifr_flags |= IFF_TAP;
    }
    else {
        msg (M_FATAL, "I don't recognize device %s as a TUN or TAP device",dev);
    }
    
    if (strlen (dev) > 3)    /* unit number specified? */
        strncpy (ifr.ifr_name, dev, IFNAMSIZ);
    
    if (ioctl (fd, TUNSETIFF, (void *) &ifr) < 0) //打开虚拟网卡
        msg (M_ERR, "Cannot ioctl TUNSETIFF %s", dev);
    set_nonblock (fd);
    msg (M_INFO, "TUN/TAP device %s opened", ifr.ifr_name);
    strncpynt (actual, ifr.ifr_name, size);
    
    return fd;
}

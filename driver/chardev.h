#ifndef CHARDEV_H
#define CHARDEV_H
#include <linux/ioctl.h>
#include <linux/net.h>

#define DEV_NAME "uglybastard"
#define MAGIC_NUM 69420

// Fetches data for pid and returns array size
#define IOCTL_FETCH_FOR_PID _IOWR(MAGIC_NUM, 0, unsigned long)
#define IOCTL_GET_SIZE _IOR(MAGIC_NUM, 1, unsigned long)
enum mode
{
	Vm,
	Sock
};

#define IOCTL_SET_MODE _IOW(MAGIC_NUM, 2, enum mode)

#define DEVICE_FILE_NAME "uglybastard"
#define DEVICE_FILE_PATH "/dev/uglybastard"
struct vm_area_info
{
	unsigned long vm_start;
	unsigned long vm_end;
	unsigned long vm_flags;
	int inode;
};

struct socket_info
{
	socket_state state;
	short type;
	unsigned long flags;
};
#endif

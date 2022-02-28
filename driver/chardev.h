#ifndef CHARDEV_H
#define CHARDEV_H
#include <linux/ioctl.h>
#include <linux/net.h>
#include <uapi/linux/net.h>

#define MAJOR_NUM 0

#define IOCTL_SET_PARAM _IOR(MAJOR_NUM, 0, unsigned long)

enum mode
{
	Vm,
	Sock
};

#define IOCTL_SET_MODE _IOR(MAJOR_NUM, 1, enum mode)

#define DEVICE_FILE_NAME "uglybastard"

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

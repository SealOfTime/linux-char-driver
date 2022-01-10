#ifndef CHARDEV_H
#define CHARDEV_H
#include <linux/ioctl.h>

#define MAJOR_NUM 228

#define IOCTL_SET_PID _IOR(MAJOR_NUM, 0, unsigned long)
#define IOCTL_GET_PID _IOW(MAJOR_NUM, 1)

enum mode {
	Vm, Sock
}

#define IOCTL_SET_MODE _IOR(MAJOR_NUM, 2, mode)

#define DEVICE_FILE_NAME "lab2_dev"

#endif

#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/fs.h>

#include "chardev.h"

#define DEV_NAME "uglybastard"

static void __user *VmAreaAddr = NULL;
static int SockFd = -1;
static enum mode Mode = Vm;

static ssize_t device_read(
	struct file *file,
	char __user *buf,
	size_t len,
	loff_t *off)
{
	int bytes_read = 0;
	void *transmitted;
	unsigned long trans_sz = 0;

	switch (Mode)
	{
	case Vm:
	{
		if (VmAreaAddr == NULL)
		{
			return 0;
		}

		struct vm_area_struct *vma_tmp = find_vma(current->mm, VmAreaAddr);
		if (vma_tmp == NULL)
		{
			return 0; // todo: err
		}

		if (vma_tmp->vm_start > VmAreaAddr)
		{
			return 0; // todo: err
		}

		struct vm_area_info vmi = {
			.inode = vma_tmp->vm_file->f_inode->i_ino,
			.vm_start = vma_tmp->vm_start,
			.vm_end = vma_tmp->vm_end,
			.vm_flags = vma_tmp->vm_flags,
		};
		break;
	}
	case Sock:
	{
		if (SockFd == -1)
		{
			return 0; // todo: err
		}

		int err = 0;
		struct socket *socket = sockfd_lookup(SockFd, &err);
		if (err != 0 || socket == NULL)
		{
			return 0; // todo: err
		}

		break;
	}
	}

	return bytes_read;
}

int device_ioctl(
	struct file *file,
	unsigned int ioctl_num,
	unsigned long ioctl_param)
{
	switch (ioctl_num)
	{
	case IOCTL_SET_PARAM:
	{
		if (Mode == Vm)
		{
			VmAreaAddr = ioctl_param;
			SockFd = -1;
		}
		else if (Mode == Sock)
		{
			VmAreaAddr = NULL;
			SockFd = (int)ioctl_param;
		}
		break;
	}
	case IOCTL_SET_MODE:
		Mode = (enum mode)ioctl_param;
		break;
	}

	return 0;
}

struct file_operations Fops = {
	.read = device_read,
	.unlocked_ioctl = device_ioctl,
};

int init_module()
{
	int res;

	res = register_chrdev(MAJOR_NUM, DEV_NAME, &Fops);

	if (res < 0)
	{
		pr_alert("Device registration failed with %d\n",
				 res);
		return res;
	}

	return 0;
}

void cleanup_module()
{
	unregister_chrdev(MAJOR_NUM, DEV_NAME);
}
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/fs.h>

#include "chardev.h"
#define OK 0
#define DEV_NAME "lab_dev"

static unsigned long Pid = 0;
static enum mode Mod = Vm;
static int Dev_Open = 0;

static int device_open(struct inode* inode, struct file* file) {
	if(Dev_Open) {
		return -EBUSY;
	}
	Dev_Open++;

	try_module_get(THIS_MODULE);
	return OK;

}

static int device_release(struct inode* inode, struct file* file) {
	Dev_Open--;

	module_put(THIS_MODULE);
	return OK;
}

static ssize_t device_read(
	struct file* file,
	char __user* buf,
	size_t len,
	loff_t* off
) {
	int bytes_read = 0;
	struct task_struct* t = get_pid_task(find_get_pid(Pid), PIDTYPE_PID);
	void* transmitted;
	unsigned long trans_sz = 0;
	switch(Mod) {
	case Vm:
		transmited = t->mm->mmap;
		break;
	case Sock:
		long fds_count = t->count;
		transmited =
		break;
	}
	while(len) {
		put_user();
		len--;
		bytes_read++;
	}

	return bytes_read;
}

static ssize_t device_write(
	struct file* file,
	const char __user* buf,
	size_t len,
	loff_t* off
) {
	//TODO: remove/use
	return 0;
}

int device_ioctl(
	struct inode* inode,
	struct file* file,
	unsigned int ioctl_num
	unsigned long ioctl_param
) {
	int i;
	char* tmp;
	char ch;

	switch (ioctl_num) {
	case IOCTL_SET_PID:
		Pid = ioctl_param;
		break;
	case IOCTL_GET_PID:
		unsigned long * dest = (unsigned long *) ioctl_param;
		put_user(Pid, dest);
		break;
	case IOCTL_SET_MODE:
		Mod = (enum mode) ioctl_param;
		break;
	}

	return OK;
}

struct file_operations Fops = {
	.read = device_read,
	.write = device_write,
	.ioctl = device_ioctl,
	.open = device_open,
	.release = device_release,
};

int init_module() {
	int res;

	res = register_chrdev(MAJOR_NUM, DEV_NAME, &Fops);

	if (res < 0) {
		printk(KERN_ALERT "%s failed with %d\n",
			"Device registration", res);
		return res;
	}

	return OK;
}

void cleanup_module() {
	int res;

	res = unregister_chrdev(MAJOR_NUM, DEV_NAME);

	if (res < 0) {
		printk(KERN_ALERT "Error: unregister_chrdev: %d\n", ret);
	}
}

//pid_task(find_vpid(pid), PIDTYPE_PID);
//get_pid_task(find_get_pid(pid_no), PIDTPYE_PID);
//files -> fd_array 
//struct stat statbuf;
//fstat(fd, &statbuf);
//S_ISSOCK(statbuf.st_mode);
//sock_from_file
//
//get_task_mm



#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/device.h>
#include <linux/fs.h>
#include <linux/fdtable.h> // files_struct

#include "chardev.h"

static int pid = 0;
static void *data;
static size_t size = 0; // In structs
static enum mode Mode = Vm;

static ssize_t device_read(
	struct file *file,
	char __user *buf,
	size_t len,
	loff_t *off)
{
	ssize_t bytes_read;
	const char *msg = (const char*)data;
	switch (Mode)
	{
	case Vm:
		bytes_read = len - copy_to_user(buf, msg + *off, len);
		break;
	case Sock:
		bytes_read = len - copy_to_user(buf, msg + *off, len);
		break;
	default:
		pr_err("[UglyBastard]: bad mode %d\n", Mode);
		return -1;
	}
	*off += bytes_read;
	return bytes_read;
}

static long int fetch_vm_info(struct task_struct *task);
static long int fetch_socket_info(struct task_struct *task);

long int device_ioctl(
	struct file *file,
	unsigned int ioctl_num,
	unsigned long ioctl_param)
{
	switch (ioctl_num)
	{
	case IOCTL_FETCH_FOR_PID:
	{
		pid = (int) ioctl_param;
		pr_info("[UglyBastard] fetching data for pid=%d\n", pid);
		struct task_struct *task = get_pid_task(find_get_pid(pid), PIDTYPE_PID);
		if(task == NULL) {
			pr_err("[UglyBastard] process with that pid does not exist.\n");
			return -1;
		}

		switch(Mode){
		case Vm:
			return fetch_vm_info(task);
		case Sock:
			return fetch_socket_info(task);
		}

		pr_err("[UglyBastard]: Error unknown Mode %d\n", Mode);
		return -1;
	}
	case IOCTL_GET_SIZE:
		return size;
	case IOCTL_SET_MODE:
		Mode = (enum mode)ioctl_param;
		pr_info("[UglyBastard] set mode to %lu\n", ioctl_param);
		return 0;
	}

	pr_err("[UglyBastard]: Error unknown ioctl %u", ioctl_num);
	return -1; 
}

static long int fetch_vm_info(struct task_struct *task) {
	struct vm_area_struct *curr = task->mm->mmap;
	struct vm_area_info *info;
	size_t i;
	size = (size_t) task->mm->map_count;
	data = krealloc_array(data, size, sizeof(struct vm_area_info), GFP_KERNEL);
	info = (struct vm_area_info*) data;
	for(i = 0; curr != NULL; curr = curr->vm_next) {
		info[i] = (struct vm_area_info){
			.vm_start = curr->vm_start,
			.vm_end = curr->vm_end,
			.vm_flags = curr->vm_flags
		};
		if(curr->vm_file != NULL) {
			if(curr->vm_file->f_inode != NULL){
				info[i].inode = curr->vm_file->f_inode->i_ino;
			}
		}
		i++;
	}
	pr_info("[UglyBastard] Successfully fetched vm_area info from process.\n");
	return size;

}

static long int fetch_socket_info(struct task_struct *task) {
	struct socket_info *info;
	struct file **fds = task->files->fd_array;
	size_t fdz_sz = (size_t) atomic_read(&task->files->count);
	struct socket *so;
	size_t sz = 0;
	size_t i = 0;
	size_t j = 0;
	pr_info("[UglyBastard] total fds: %lu\n", fdz_sz);
	for(i = 0; i < fdz_sz; i++) {
		if(fds[i] == NULL) {
			pr_info("[UglyBastard] reached the end.\n");
			break;
		}
		if(fds[i]->f_inode == NULL) {
			pr_err("[UglyBastard] finode null\n");
			continue;
		}
		if(S_ISSOCK(fds[i]->f_inode->i_mode)){
			pr_info("[UglyBastard] found a socket\n");
			sz++;
		}
	}
	size = sz;
	if(size != 0){
		data = krealloc_array(data, size, sizeof(struct socket_info), GFP_KERNEL);
		info = (struct socket_info*) data;
		for(i = 0; i < size; i++) {
			if (S_ISSOCK(fds[i]->f_inode->i_mode)){
				pr_info("[UglyBastard] saving info of a socket\n");
				so = sock_from_file(fds[i]);
				info[j++] = (struct socket_info){
					.state = so->state,
					.type = so->type,
					.flags = so->flags
				};
			}
		}
	}
	return size;
}

struct file_operations Fops = {
	.read = device_read,
	.unlocked_ioctl = device_ioctl,
};

static struct class *cls;

static int major;

static int __init init_uglybastard(void)
{
	major = register_chrdev(0, DEV_NAME, &Fops);

	pr_info("UglyBastard has entered the system.\n");
	if (major < 0)
	{
		pr_alert("Device registration failed with %d\n", major);
		return major;
	}

	cls = class_create(THIS_MODULE, DEV_NAME);
	device_create(cls, NULL, MKDEV(major, 0), NULL, DEV_NAME);
	pr_info("Device created on /dev/%s\n", DEV_NAME);

	return 0;
}

static void __exit cleanup_uglybastard(void)
{
	device_destroy(cls, MKDEV(major, 0));
	class_destroy(cls);

	unregister_chrdev(major, DEV_NAME);
	pr_info("UglyBastard has left the system.\n");
}

module_init(init_uglybastard);
module_exit(cleanup_uglybastard);
MODULE_LICENSE("GPL");

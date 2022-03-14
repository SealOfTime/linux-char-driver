#include <sys/ioctl.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <malloc.h>
#include "./driver/chardev.h"

int fetch_vm(int dev, int pid);
int fetch_socks(int dev, int pid);

int main(int argc, char* argv[]) {
	if(argc < 3) {
		fprintf(stderr, "not enough arguments\n");
		return -1;
	}
	int pid = atoi(argv[1]);

	char* target = argv[2];
	enum mode md;
	char* md_str;
	if(strcmp(target, "vm") == 0) {
		printf("Mode is vm\n");
		md = Vm;
		md_str = "memory areas";
	} else if(strcmp(target, "sock") == 0) {
		printf("Mode is socket\n");
		md = Sock;
		md_str = "sockets";
	} else {
		fprintf(stderr, "Unknown Mode %s\n", target);
		return -1;
	}

	int fild = open("/dev/uglybastard", O_RDONLY);
	long int msg_sz;

	if(fild <= 0) {
		fprintf(stderr, "couldn't open device file %d\n", fild);
		return -1;
	}
	if(md == Vm) {
		return fetch_vm(fild, pid);
	}
	if(md == Sock) {
		return fetch_socks(fild, pid);
	}

	return 0;
}

int fetch_vm(int dev, int pid) {
	long int msg_sz;
	ssize_t readb;
	size_t sz;
	size_t i;
	struct vm_area_info *received;

	if(ioctl(dev, IOCTL_SET_MODE, Vm)) {
		fprintf(stderr, "couldn't set mode to Vm\n");
		return -1;
	}
	printf("Successfully set mode to Vm\n");

	msg_sz = ioctl(dev, IOCTL_FETCH_FOR_PID, pid);
	if(msg_sz < 0) {
		fprintf(stderr, "couldn't fetch memory areas for Process(pid=%d)\n", pid);
		return -1;
	}
	printf("Process(pid=%d) has %lu memory areas.\n", pid, msg_sz);

	received = malloc(sizeof(struct vm_area_info));
	sz = (size_t) msg_sz;
	for(i = 0; i < sz; i++) {
		readb = read(dev, received, sizeof(struct vm_area_info));
		printf("read %ld bytes\n", readb);
		printf("vm_area:\n\tstart: %lu\n\tend: %lu\n\tflags: %lu\n\tinode: %d\n", received->vm_start, received->vm_end, received->vm_flags, received->inode);
	}

	return 0;
}

const char* SOCKET_STATE_NAME[] = {
	"SS_FREE",
	"SS_UNCONNECTED",
	"SS_CONNECTING",
	"SS_CONNECTED",
	"SS_DISCONNECTING"
};

int fetch_socks(int dev, int pid) {
	long int msg_sz;
	ssize_t readb;
	size_t sz;
	size_t i;
	struct socket_info *received;

	if(ioctl(dev, IOCTL_SET_MODE, Sock)) {
		fprintf(stderr, "couldn't tell the mode to Sock\n");
		return -1;
	}
	printf("Successfully set mode\n");

	msg_sz = ioctl(dev, IOCTL_FETCH_FOR_PID, pid);
	if(msg_sz < 0) {
		fprintf(stderr, "couldn't fetch sockets for Process(pid=%d)\n", pid);
		return -1;
	}
	printf("Process(pid=%d) has %lu sockets\n", pid, msg_sz);

	received = malloc(sizeof(struct socket_info));
	sz = (size_t) msg_sz;
	for(i = 0; i < sz; i++) {
		readb = read(dev, received, sizeof(struct socket_info));
		printf("read %ld bytes\n", readb);
		printf("socket:\n\tstate: %d\n\ttype: %d\n\tflags: %lu\n", received->state, received->type, received->flags);
	}

	return 0;
}

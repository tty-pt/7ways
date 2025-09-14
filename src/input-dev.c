#include "../include/input.h"

#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <linux/input.h>
#include <stdio.h>
#include <string.h>
#include <sys/epoll.h>
#include <sys/inotify.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>

#include <qsys.h>

#ifndef EV_BUFSZ
#define EV_BUFSZ 256
#endif

typedef struct {
	int fd;
	dev_t rdev;
	char path[PATH_MAX];
	uint8_t caps_key, caps_rel, caps_abs;
} idev_t;

static int epfd = -1, inofd = -1, grab_all = 0;
static idev_t devs[128];
static int ndev = 0;

static int input_index(dev_t r) {
	for (int i = 0; i < ndev; i++)
		if (devs[i].rdev == r)
			return i;
	return -1;
}

static int has_type(unsigned long *evbits, int t) {
	return !!(evbits[
			t/ (8 * sizeof(long))
	] & (1UL << (t % (8 * sizeof(long)))));
}

static int input_open(const char *path) {
	struct stat st;

	if (stat(path, &st) < 0)
		return -1;

	if (!S_ISCHR(st.st_mode))
		return -1;

	if (input_index(st.st_rdev) >= 0)
		return 0;

	int fd = open(path, O_RDONLY | O_NONBLOCK | O_CLOEXEC);
	if (fd < 0)
		return -1;

	unsigned long evbits[
		(EV_CNT + (8 * sizeof(long)) -1)
			/ (8 * sizeof(long))
	];

	memset(evbits, 0, sizeof(evbits));
	if (ioctl(fd, EVIOCGBIT(0, sizeof(evbits)), evbits) < 0) {
		close(fd);
		return -1;
	}

	uint8_t ckey = has_type(evbits, EV_KEY);
	uint8_t crel = has_type(evbits, EV_REL);
	uint8_t cabs = has_type(evbits, EV_ABS);

	if (!(ckey || crel || cabs)) {
		close(fd);
		return -1;
	}

#ifdef EVIOCSCLOCKID
	int clk = CLOCK_MONOTONIC;
	ioctl(fd, EVIOCSCLOCKID, &clk);
#endif

	if (grab_all) ioctl(fd, EVIOCGRAB, 1);

	struct epoll_event ev = {
		.events = EPOLLIN | EPOLLET,
		.data.fd = fd,
	};

	if (epoll_ctl(epfd, EPOLL_CTL_ADD, fd, &ev) < 0) {
		close(fd);
		return -1;
	}

	idev_t *d = &devs[ndev++];
	d->fd = fd; d->rdev = st.st_rdev;
	d->caps_key = ckey;
	d->caps_rel = crel;
	d->caps_abs = cabs;
	strncpy(d->path, path, sizeof(d->path) - 1);

	d->path[sizeof(d->path) - 1] = 0;
	return 0;
}

static void input_iscan(void) {
	DIR *dir = opendir("/dev/input");
	struct dirent *e;

	if (!dir)
		return;

	while ((e = readdir(dir))) {
		char p[PATH_MAX];

		if (strncmp(e->d_name, "event", 5) != 0)
			continue;

		snprintf(p, sizeof(p), "/dev/input/%s",
				e->d_name);

		input_open(p);
	}

	closedir(dir);
}

void input_init(int grab) {
	grab_all = grab ? 1 : 0;
	epfd = epoll_create1(EPOLL_CLOEXEC);
	inofd = inotify_init1(IN_NONBLOCK | IN_CLOEXEC);
	inotify_add_watch(inofd, "/dev/input",
			IN_CREATE | IN_DELETE);

	struct epoll_event ev = {
		.events = EPOLLIN,
		.data.fd = inofd,
	};

	epoll_ctl(epfd, EPOLL_CTL_ADD, inofd, &ev);
	input_iscan();
}


static void input_close(int idx) {
	if (idx < 0 || idx >= ndev)
		return;

	epoll_ctl(epfd, EPOLL_CTL_DEL, devs[idx].fd, NULL);
	close(devs[idx].fd);
	devs[idx] = devs[--ndev];
}

static void input_inotify(void) {
	char buf[4096] __attribute__((aligned(__alignof__(struct inotify_event))));

	for (;;) {
		ssize_t len = read(inofd, buf, sizeof(buf));

		if (len < 0)
			break;

		for (char *p = buf; p < buf + len; ) {
			struct inotify_event *ie
				= (struct inotify_event *) p;

			if (ie->len && strncmp(ie->name, "event", 5) == 0) {
				char path[PATH_MAX];
				snprintf(path, sizeof(path), "/dev/input/%s", ie->name);
				if (ie->mask & IN_CREATE)
					input_open(path);

				if (ie->mask & IN_DELETE)
					for (int i=0; i<ndev; i++)
						if (!strcmp(devs[i].path, path))
						{
							input_close(i);
							break;
						}
			}

			p += sizeof(*ie) + ie->len;
		}
	}
}

static void input_drain(int fd) {
	struct input_event ev[EV_BUFSZ];
	for (;;) {
		ssize_t r = read(fd, ev, sizeof(ev));

		if (r < 0)
			break;

		// dead device
		if (r == 0) {
			for (int i=0; i<ndev; i++)
				if (devs[i].fd == fd) {
					input_close(i);
					break;
				}
			break;
		}

		int n = r / (int) sizeof(struct input_event);

		for (int i = 0; i < n; i++) {
			WARN("code %d\n", ev[i].code);
			input_cb_t *cb = input_cb(ev[i].code);
			if (cb)
				cb(ev[i].value, ev[i].type);
		}
	}
}

void input_poll(void) {
	struct epoll_event evs[64];
	int n = epoll_wait(epfd, evs, 64, 0);

	if (n < 0)
		return;

	for (int i = 0; i < n; i++) {
		int fd = evs[i].data.fd;
		if (fd == inofd)
			input_inotify();
		else
			input_drain(fd);
	}
}

void input_deinit(void) {
	for (int i = 0; i < ndev; i++)
		close(devs[i].fd);
	ndev = 0;
	if (inofd >= 0)
		close(inofd);
	if (epfd >= 0)
		close(epfd);
	inofd = epfd = -1;
}

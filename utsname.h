#ifndef UTSNAME_H
#define UTSNAME_H

int uname(struct utsname *buf);

struct utsname
{
	char sysname[];
	char nodename[];
	char version[];
	char machine[];
};

#endif

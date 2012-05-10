#ifndef QUEUE_H
#define QUEUE_H

#include <dbus/dbus.h>

typedef struct {
	char name[512];
	dbus_uint32_t value;
} Item;

#endif

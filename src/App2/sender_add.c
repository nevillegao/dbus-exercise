#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dbus/dbus.h>
#include "queue.h"

/*
 * Connect to the DBUS bus and send a signal 
 * to add items to the queue at a random position
 */
void send_signal_add_items(Item *item)
{
	DBusMessage *msg;
	DBusConnection *conn;
	DBusError err;
	int ret;
	dbus_uint32_t serial = 0;

	// initialize the error value
	dbus_error_init(&err);

	// connect to the DBUS system bus, and check for errors
	conn = dbus_bus_get(DBUS_BUS_SESSION, &err);
	if (dbus_error_is_set(&err))
	{ 
		fprintf(stderr, "Connection Error (%s)\n", err.message); 
		dbus_error_free(&err); 
	}
	if (conn == NULL)
	{ 
		exit(1);
	}

	// register our name on the bus, and check for errors
	ret = dbus_bus_request_name(conn, "queue.msg.add", DBUS_NAME_FLAG_REPLACE_EXISTING, &err);
	if (dbus_error_is_set(&err))
	{ 
		fprintf(stderr, "Name Error (%s)\n", err.message); 
		dbus_error_free(&err); 
	}
	if (DBUS_REQUEST_NAME_REPLY_PRIMARY_OWNER != ret)
	{ 
		exit(1);
	}

	// loop listening for signals being emmitted
	while (1)
	{
		// create a signal & check for errors 
		msg = dbus_message_new_signal("/queue/msg/Object",  // object name of the signal
		                              "queue.msg.Handler",  // interface name of the signal
					      "Add");               // name of the signal
		if (msg == NULL) 
		{ 
			fprintf(stderr, "Message Null\n"); 
			exit(1); 
		}

		// append arguments onto signal
		char *s = (char *)malloc(512);
		strcpy(s, item->name);
		dbus_uint32_t v = item->value;
		dbus_message_append_args(msg, 
		                         DBUS_TYPE_STRING, &s,  // the name of the item
					 DBUS_TYPE_UINT32, &v,  // the value of the item
					 DBUS_TYPE_INVALID);
		free(s);
		s = NULL;

		// send the message and flush the connection
		if (!dbus_connection_send(conn, msg, &serial))
		{
			fprintf(stderr, "Out Of Memory!\n"); 
			exit(1);
		}
		dbus_connection_flush(conn);

		printf("Signal sent to add an item with name \"%s\" valued (%d)\n", item->name, item->value);

		// free the message 
		dbus_message_unref(msg);

		// wait for one second
		sleep(1);
	}
}

int main(int argc, char *argv[])
{
	// initialize an item that consists of a name and a value, 
	// and send it to the DBUS
	Item *item = (Item *)malloc(sizeof(Item));
	strcpy(item->name, "add");
	item->value = 1;
	send_signal_add_items(item);
	free(item);
	item = NULL;
	return 0;
}

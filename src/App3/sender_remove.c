#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dbus/dbus.h>
#include "queue.h"

/*
 * Connect to the DBUS bus and send a signal 
 * to remove items from the queue randomly
 */
void send_signal_remove_items()
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
	ret = dbus_bus_request_name(conn, "queue.msg.remove", DBUS_NAME_FLAG_REPLACE_EXISTING, &err);
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
		// create a method cal & check for errors 
		msg = dbus_message_new_method_call("queue.msg.receiver",  // destination name
		                                   "/queue/msg/Object",   // object name of the signal
		                                   "queue.msg.Handler",   // interface name of the signal
						   "Remove");                // name of the method
		if (msg == NULL) 
		{ 
			fprintf(stderr, "Message Null\n"); 
			exit(1); 
		}

		// send the message and flush the connection
		if (!dbus_connection_send(conn, msg, &serial))
		{
			fprintf(stderr, "Out Of Memory!\n"); 
			exit(1);
		}
		dbus_connection_flush(conn);

		printf("Signal sent to remove an item randomly\n");

		// free the message 
		dbus_message_unref(msg);

		// wait for five seconds
		sleep(5);
	}
}

int main(int argc, char *argv[])
{
	send_signal_remove_items();
	return 0;
}

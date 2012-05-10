#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include <dbus/dbus.h>
#include "queue.h"

#define MAXNUM 1024  // the max number of items in the queue

int num = 0;         // current number of items in the queue
Item queue[MAXNUM];  // global queue for the program

/*
 * Randomly adds items to the queue, 
 * and returns the position if success
 */
int random_add_items(Item queue[MAXNUM], Item *item)
{
	if (num == 0)
	{
		strcpy(queue[0].name, item->name);
		queue[0].value = item->value;
		num = 1;
		return 0;
	}
	if (num >= MAXNUM)
	{
		printf("The queue is full!\n");
		return -1;
	}

	// generate a random number between 0 and current number of the queue
	srand((unsigned int)time(NULL));
	int n = rand() % num;
	int i;
	for (i = num - 1; i >= n; --i)
	{
		strcpy(queue[i + 1].name, queue[i].name);
		queue[i + 1].value = queue[i].value;
	}
	strcpy(queue[n].name, item->name);
	queue[n].value = item->value;
	++num;
	return n;
}

/*
 * Randomly removes items from the queue, 
 * and returns the position if success
 */
int random_remove_items(Item queue[MAXNUM])
{
	if (num <= 0)
	{
		printf("The queue is empty!\n");
		return -1;
	}

	// generate a random number between 0 and current number of the queue
	srand((unsigned int)time(NULL));
	int n = rand() % num;
	int i;
	for (i = n; i < num - 1; ++i)
	{
		strcpy(queue[i].name, queue[i + 1].name);
		queue[i].value = queue[i].value;
	}
	--num;
	return n;
}

/*
 * Receive the signals from the bus
 */
void receive()
{
	DBusMessage *msg;
	DBusMessageIter iter;
	DBusConnection *conn;
	DBusError err;
	int ret;

	Item *item = (Item *)malloc(sizeof(Item));
	if (item == NULL)
	{
		exit(1);
	}
	char *s = (char *)malloc(512 * sizeof(char));
	if (s == NULL)
	{
		exit(1);
	}

	printf("Listening for signals\n");

	// initialize the errors
	dbus_error_init(&err);

	// connect to the bus and check for errors
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

	// request our name on the bus and check for errors
	ret = dbus_bus_request_name(conn, "queue.msg.receiver", DBUS_NAME_FLAG_REPLACE_EXISTING , &err);
	if (dbus_error_is_set(&err))
	{ 
		fprintf(stderr, "Name Error (%s)\n", err.message);
		dbus_error_free(&err); 
	}
	if (DBUS_REQUEST_NAME_REPLY_PRIMARY_OWNER != ret)
	{
		exit(1);
	}

	// add a rule for which messages we want to see
	// see signals from the given interface
	dbus_bus_add_match(conn, "interface='queue.msg.Handler'", &err);
	dbus_connection_flush(conn);
	if (dbus_error_is_set(&err))
	{ 
		fprintf(stderr, "Match Error (%s)\n", err.message);
		exit(1); 
	}

	// loop listening for signals being emmitted
	while (1)
	{
		// non blocking read of the next available message
		dbus_connection_read_write(conn, 0);
		msg = dbus_connection_pop_message(conn);

		// loop again if we haven't read a message
		if (msg == NULL)
		{
			sleep(1);
			continue;
		}

		// signals for adding
		// check if the message is a signal from the correct interface and with the correct name
		if (dbus_message_is_signal(msg, "queue.msg.Handler", "Add"))
		{

			// read the parameters
			if (!dbus_message_iter_init(msg, &iter))
			{
				fprintf(stderr, "Message Has No Parameters\n");
			}
			else
			{
				dbus_uint32_t v;
				dbus_message_iter_get_basic(&iter, &s);
				strcpy(item->name, s);
				dbus_message_iter_next(&iter);  // moves the iter to read next param
				dbus_message_iter_get_basic(&iter, &v);
				item->value = v;
			}

			int n = random_add_items(queue, item);
			if (n != -1)
			{
				printf("Added an item with name \"%s\" valued (%d) at position (%d)\n", 
				       item->name, item->value, n);
			}
			else
			{
				exit(1);
			}
		}

		// methods for removing
		// check if the message is a method call from the correct interface and with the correct name
		else if (dbus_message_is_method_call(msg, "queue.msg.Handler", "Remove"))
		{
			int n = random_remove_items(queue);
			if (n != -1)
			{
				printf("Removed an item from position (%d)\n", n);
			}
			else
			{
				exit(1);
			}
		}

		// free the message
		dbus_message_unref(msg);
	}

	// free the item
	free(item);
	item = NULL;
	// free the s
	free(s);
	s = NULL;
}

int main(int argc, char *argv[])
{
	receive();
	return 0;
}

#include <zephyr/net/socket.h>
#include <zephyr/posix/sys/eventfd.h>
#include <zephyr/posix/arpa/inet.h>
#include <zephyr/posix/poll.h>
#include <zephyr/posix/fcntl.h>

#include <zephyr/logging/log.h>

#include <pb_encode.h>
#include <pb_decode.h>
#include "gz_message.pb.h"
#include "pb.h"

#include "gz_link.h"
#include "zephyr/kernel.h"

LOG_MODULE_REGISTER(gz_link, LOG_LEVEL_DBG);

#define ADDR        "127.0.0.1"
#define CLIENT_PORT 0
#define SERVER_PORT 9002

#define SEND_MSGS_QUEUE_SIZE 10

#define GZ_CONN_THREAD_STACK_SIZE 1024

#define RXTX_BUF_SIZE 512

/* One fd is for the socket and one is an eventfd indicating a message in the queue. */
static struct pollfd fd[2];

static int sock = -1;
static int ctrl_fd = -1;

static bool connected;
static gz_link_msg_cb msg_cb;

static struct net_sockaddr_in c_addr;
static struct net_sockaddr_in s_addr;

static uint8_t rx_buf[RXTX_BUF_SIZE];
static uint8_t tx_buf[RXTX_BUF_SIZE];

static struct k_thread gz_thread_data;
K_THREAD_STACK_DEFINE(gz_thread_stack, GZ_CONN_THREAD_STACK_SIZE);

K_MSGQ_DEFINE_STATIC_TYPE(gz_msg_queue, GazeboMessage, SEND_MSGS_QUEUE_SIZE);

static bool process_rx(void)
{
	int recv;
	GazeboMessage msg = GazeboMessage_init_default;
	pb_istream_t stream;

	memset(rx_buf, 0, sizeof(rx_buf));

	/* TODO: There may be more than one message in the socket at a time. */
	recv = zsock_recv(sock, rx_buf, sizeof(rx_buf), 0);
	if (recv <= 0) {
		LOG_WRN("Connection lost!");
		connected = false;
		return false;
	}

	stream = pb_istream_from_buffer(rx_buf, recv);

	if (pb_decode(&stream, GazeboMessage_fields, &msg)) {
		msg_cb(&msg);
	} else {
		LOG_ERR("Failed to decode Gazebo message %s", PB_GET_ERROR(&stream));
		return false;
	}

	return true;
}

static bool process_tx(void)
{
	int ret;
	GazeboMessage rx = GazeboMessage_init_zero;
	pb_ostream_t ostream;

	do {
		ret = k_msgq_get(&gz_msg_queue, &rx, K_NO_WAIT);
		if (ret != 0) {
			LOG_ERR("Failed to get a message from queue");
			return false;
		}
		ostream = pb_ostream_from_buffer(tx_buf, sizeof(tx_buf));

		if (!pb_encode(&ostream, GazeboMessage_fields, &rx)) {
			LOG_ERR("Failed to encode gazebo message");
			return false;
		}

		ret = zsock_send(sock, tx_buf, ostream.bytes_written, 0);

		if (ret != ostream.bytes_written) {
			LOG_ERR("Partial write not supported!");
			return false;
		}
	} while (k_msgq_num_used_get(&gz_msg_queue));

	return true;
}

static bool wait(void)
{
	int ret = zsock_poll(fd, 2, -1);
	if (ret < 0) {
		return false;
	}
	return true;
}

static void gz_link_thread(void *p1, void *p2, void *p3)
{
	do {
		if (!connected) {
			int ret =
				zsock_connect(sock, (struct net_sockaddr *)&s_addr, sizeof(s_addr));

			if (ret != 0) {
				LOG_WRN("Failed to connect, retrying...");
				k_sleep(K_SECONDS(3));
				continue;

			} else if (ret == 0) {
				connected = true;
			}
		} else {
			if (!wait()) {
				continue;
			}

			if (fd[0].revents & POLLIN) {
				process_rx();
			}
			if (fd[1].revents & POLLIN) {
				process_tx();
				/* Clear the event. */
				zvfs_eventfd_t val;
				zvfs_eventfd_read(ctrl_fd, &val);
			}
		}

	} while (true);
}

static bool setup_sock_tcp_v4(const char *addr, uint16_t port, int *sock,
			      struct net_sockaddr_in *sockaddr)
{
	*sock = zsock_socket(NET_AF_INET, NET_SOCK_STREAM, NET_IPPROTO_TCP);

	if (*sock < 0) {
		LOG_ERR("Failed to create socket: %d", -errno);
		return false;
	}

	// set up non-blocking socket
	int fl, res;
	fl = fcntl(*sock, F_GETFL, 0);
	if (fl == -1) {
		LOG_ERR("fcntl F_GETFL failed");
		return false;
	}

	fl |= O_NONBLOCK;
	res = fcntl(*sock, F_SETFL, fl);
	if (fl == -1) {
		LOG_ERR("fcntl F_SETFL failed");
		return false;
	}

	// // set up events
	fd[0].fd = *sock;
	fd[0].events = POLLIN;

	// set up address
	sockaddr->sin_family = NET_AF_INET;
	sockaddr->sin_port = net_htons(port);

	int rv = zsock_inet_pton(NET_AF_INET, addr, &sockaddr->sin_addr);

	if (rv != 1) {
		LOG_ERR("inet_pton failed");
		return false;
	}

	return true;
}

int gz_link_write(const GazeboMessage *msg)
{
	if (!connected) {
		LOG_WRN("Not connected, discarding message");
		return -1;
	}
	if (0 != k_msgq_put(&gz_msg_queue, msg, K_NO_WAIT)) {
		LOG_ERR("Failed to write message to queue");
		return -1;
	}
	int rc = zvfs_eventfd_write(ctrl_fd, 1);
	if (rc < 0) {
		return -1;
	}
	return 0;
}

bool gz_link_start(gz_link_msg_cb cb)
{
	s_addr.sin_family = NET_AF_INET;
	s_addr.sin_port = net_htons(SERVER_PORT);
	int ret = zsock_inet_pton(NET_AF_INET, ADDR, &s_addr.sin_addr);

	if (ret != 1) {
		LOG_ERR("server inet_pton failed");
		return false;
	}

	ret = setup_sock_tcp_v4(ADDR, CLIENT_PORT, &sock, &c_addr);
	if (!ret) {
		LOG_ERR("Failed to prepare client socket!");
		return false;
	}

	/* Create eventfds. */
	ctrl_fd = zvfs_eventfd(0, 0);
	fd[1].fd = ctrl_fd;
	fd[1].events = POLLIN;

	msg_cb = cb;

	(void)k_thread_create(&gz_thread_data, gz_thread_stack,
			      K_THREAD_STACK_SIZEOF(gz_thread_stack), gz_link_thread, NULL, NULL,
			      NULL, 60, 0, K_NO_WAIT);

	return true;
}

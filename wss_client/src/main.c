#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(wss_client, LOG_LEVEL_INF);

#include <zephyr/net/socket.h>
#include <zephyr/net/websocket.h>
#include <zephyr/net/tls_credentials.h>
#include <modem/nrf_modem_lib.h>
#include <modem/lte_lc.h>

#define SERVER_HOST "echo.websocket.events"
#define SERVER_PORT 443
#define SERVER_ADDR "35.71.131.46"  // IPv4
#define MSG_BUF_SIZE 1024

static uint8_t recv_buf[MSG_BUF_SIZE];

static int connect_tls_socket(const char *addr_str, int port)
{
	struct sockaddr_in addr = {
		.sin_family = AF_INET,
		.sin_port = htons(port),
	};
	inet_pton(AF_INET, addr_str, &addr.sin_addr);

	int sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TLS_1_2);
	if (sock < 0) {
		LOG_ERR("Failed to create TLS socket: %d", errno);
		return -1;
	}

	int verify = TLS_PEER_VERIFY_NONE;
	setsockopt(sock, SOL_TLS, TLS_PEER_VERIFY, &verify, sizeof(verify));
	setsockopt(sock, SOL_TLS, TLS_HOSTNAME, SERVER_HOST, strlen(SERVER_HOST));

	if (connect(sock, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
		LOG_ERR("TLS connect failed: %d", errno);
		close(sock);
		return -1;
	}

	return sock;
}

int main(void)
{
	nrf_modem_lib_init();

	if (lte_lc_connect() != 0) {
		LOG_ERR("LTE connection failed");
		return 1;
	}

	LOG_INF("Connecting...");

	int sock = connect_tls_socket(SERVER_ADDR, SERVER_PORT);
	if (sock < 0) {
		LOG_ERR("Failed to establish TLS socket.");
		return 1;
	}

	struct websocket_request req = {
		.host = SERVER_HOST,
		.url = "/",
		.tmp_buf = recv_buf,
		.tmp_buf_len = sizeof(recv_buf),
	};

	int ws_sock = websocket_connect(sock, &req, 3000, "IPv4");
	if (ws_sock < 0) {
		LOG_ERR("WebSocket connect failed: %d", ws_sock);
		close(sock);
		return 1;
	}

	LOG_INF("WebSocket connected.");

	char msg[32];
	uint64_t remaining;
	uint32_t type;
	int count = 1;
	int ret;

	// Receive and discard the initial unsolicited message
	ret = websocket_recv_msg(ws_sock, recv_buf, sizeof(recv_buf),
							&type, &remaining, 1000);

	if (ret > 0) {
		if (ret >= sizeof(recv_buf)) ret = sizeof(recv_buf) - 1;
		recv_buf[ret] = '\0';
		LOG_INF("Initial message: %s", recv_buf);
	} else if (ret == -EAGAIN) {
		LOG_WRN("No initial message received yet");
	} else {
		LOG_ERR("Error receiving initial message: %d", ret);
	}

	while (1) {

		snprintk(msg, sizeof(msg), "test%d", count++);
		websocket_send_msg(ws_sock, msg, strlen(msg),
		                   WEBSOCKET_OPCODE_DATA_TEXT, true, true, SYS_FOREVER_MS);
		LOG_INF("Sent: %s", msg);

		while (true) {
			ret = websocket_recv_msg(ws_sock, recv_buf, sizeof(recv_buf),
			                         &type, &remaining, SYS_FOREVER_MS);

			if (ret == -EAGAIN) {
				k_sleep(K_MSEC(100));
				continue;
			} else if (ret > 0) {
				recv_buf[ret] = '\0';
				LOG_INF("Received: %s", recv_buf);
				break;
			} else {
				LOG_ERR("Receive error: %d", ret);
				goto out;
			}
		}

		k_sleep(K_SECONDS(2));
	}

	out:

	close(ws_sock);
	close(sock);
	return 0;
}

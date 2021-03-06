#include "Wio3GConfig.h"
#include "Wio3GClient.h"

#define RECEIVE_MAX_LENGTH	(1500)

#define CONNECT_SUCCESS				(1)
#define CONNECT_TIMED_OUT			(-1)
#define CONNECT_INVALID_SERVER		(-2)
#define CONNECT_TRUNCATED			(-3)
#define CONNECT_INVALID_RESPONSE	(-4)

Wio3GClient::Wio3GClient(Wio3G* wio)
{
	_Wio = wio;
	_ConnectId = -1;
	_ReceiveBuffer = new byte[RECEIVE_MAX_LENGTH];
}

Wio3GClient::~Wio3GClient()
{
	delete [] _ReceiveBuffer;
}

int Wio3GClient::connect(IPAddress ip, uint16_t port)
{
	if (connected()) return CONNECT_INVALID_RESPONSE;	// Already connected.

	String ipStr = String(ip[0]);
	ipStr += ".";
	ipStr += String(ip[1]);
	ipStr += ".";
	ipStr += String(ip[2]);
	ipStr += ".";
	ipStr += String(ip[3]);
	int connectId = _Wio->SocketOpen(ipStr.c_str(), port, Wio3G::SOCKET_TCP);
	if (connectId < 0) return CONNECT_INVALID_SERVER;
	_ConnectId = connectId;

	return CONNECT_SUCCESS;
}

int Wio3GClient::connect(const char* host, uint16_t port)
{
	if (connected()) return CONNECT_INVALID_RESPONSE;	// Already connected.

	int connectId = _Wio->SocketOpen(host, port, Wio3G::SOCKET_TCP);
	if (connectId < 0) return CONNECT_INVALID_SERVER;
	_ConnectId = connectId;

	return CONNECT_SUCCESS;
}

size_t Wio3GClient::write(uint8_t data)
{
	if (!connected()) return 0;

	if (!_Wio->SocketSend(_ConnectId, &data, 1)) return 0;

	return 1;
}

size_t Wio3GClient::write(const uint8_t* buf, size_t size)
{
	if (!connected()) return 0;

	if (!_Wio->SocketSend(_ConnectId, buf, size)) return 0;

	return size;
}

int Wio3GClient::available()
{
	if (!connected()) return 0;

	int receiveSize = _Wio->SocketReceive(_ConnectId, _ReceiveBuffer, RECEIVE_MAX_LENGTH);
	for (int i = 0; i < receiveSize; i++) _ReceiveQueue.push(_ReceiveBuffer[i]);

	return _ReceiveQueue.size();
}

int Wio3GClient::read()
{
	if (!connected()) return -1;

	int actualSize = available();
	if (actualSize <= 0) return -1;	// None is available.

	byte data = _ReceiveQueue.front();
	_ReceiveQueue.pop();

	return data;
}

int Wio3GClient::read(uint8_t* buf, size_t size)
{
	if (!connected()) return 0;

	int actualSize = available();
	if (actualSize <= 0) return 0;	// None is available.

	int popSize = actualSize <= (int)size ? actualSize : size;
	for (int i = 0; i < popSize; i++) {
		buf[i] = _ReceiveQueue.front();
		_ReceiveQueue.pop();
	}

	return popSize;
}

int Wio3GClient::peek()
{
	if (!connected()) return 0;

	int actualSize = available();
	if (actualSize <= 0) return -1;	// None is available.

	return _ReceiveQueue.front();
}

void Wio3GClient::flush()
{
	// Nothing to do.
}

void Wio3GClient::stop()
{
	if (!connected()) return;

	_Wio->SocketClose(_ConnectId);
	_ConnectId = -1;
	while (!_ReceiveQueue.empty()) _ReceiveQueue.pop();
}

uint8_t Wio3GClient::connected()
{
	return _ConnectId >= 0 ? true : false;
}

Wio3GClient::operator bool()
{
	return _ConnectId >= 0 ? true : false;
}

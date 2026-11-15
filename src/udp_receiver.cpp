#include "udp_receiver.h"

#if defined(_WIN32)
#define WIN32_LEAN_AND_MEAN
#include <winsock2.h>
#include <ws2tcpip.h>
#endif

#include <cstdio>

#if defined(_WIN32)
static const SOCKET kInvalid = INVALID_SOCKET;

UdpReceiver::~UdpReceiver()
{
    Stop();
}

bool UdpReceiver::Start(unsigned short port)
{
    if (IsOpen()) return true;

    WSADATA wsa;
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) return false;
    wsaStarted = true;

    SOCKET sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (sock == INVALID_SOCKET)
    {
        Stop();
        return false;
    }

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);

    if (bind(sock, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) == SOCKET_ERROR)
    {
        closesocket(sock);
        Stop();
        return false;
    }

    u_long nonBlocking = 1;
    ioctlsocket(sock, FIONBIO, &nonBlocking);

    handle = static_cast<std::uintptr_t>(sock);
    return true;
}

bool UdpReceiver::IsOpen() const
{
    return handle != static_cast<std::uintptr_t>(kInvalid);
}

bool UdpReceiver::Poll(SensorSample& out)
{
    if (!IsOpen()) return false;

    bool got = false;
    char buffer[128];
    SOCKET sock = static_cast<SOCKET>(handle);

    for (;;)
    {
        sockaddr_in from{};
        int fromLength = sizeof(from);
        int received = recvfrom(sock, buffer, sizeof(buffer) - 1, 0,
                                reinterpret_cast<sockaddr*>(&from), &fromLength);
        if (received <= 0) break;

        buffer[received] = '\0';

        float x, y, z;
        if (std::sscanf(buffer, "%f,%f,%f", &x, &y, &z) == 3)
        {
            out.ax = x;
            out.ay = y;
            out.az = z;
            got = true;
        }
    }
    return got;
}

void UdpReceiver::Stop()
{
    if (handle != static_cast<std::uintptr_t>(kInvalid))
    {
        closesocket(static_cast<SOCKET>(handle));
        handle = static_cast<std::uintptr_t>(kInvalid);
    }
    if (wsaStarted)
    {
        WSACleanup();
        wsaStarted = false;
    }
}

#else
//Fallback for non-Windows builds of this demo.
UdpReceiver::~UdpReceiver() {}
bool UdpReceiver::Start(unsigned short) { return false; }
bool UdpReceiver::IsOpen() const { return false; }
bool UdpReceiver::Poll(SensorSample&) { return false; }
void UdpReceiver::Stop() {}
#endif
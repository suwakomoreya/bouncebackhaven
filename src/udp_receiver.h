#pragma once

#include <cstdint>

// A single acceleration sample in m/s^2 (phone axes).
struct SensorSample
{
    float ax = 0.0f;
    float ay = 0.0f;
    float az = 0.0f;
};

// Non-blocking UDP receiver for "ax,ay,az" ASCII datagrams.
//
// Kept in its own translation unit so that <winsock2.h> / <windows.h> never
// meet raylib.h, whose Rectangle / DrawText / CloseWindow names collide with
// the Win32 API.
class UdpReceiver
{
public:
    UdpReceiver() = default;
    ~UdpReceiver();

    UdpReceiver(const UdpReceiver&) = delete;
    UdpReceiver& operator=(const UdpReceiver&) = delete;

    bool Start(unsigned short port);
    bool IsOpen() const;
    void Stop();

    // Drains all queued datagrams. Returns true if at least one valid sample
    // was parsed; `out` then holds the most recent one.
    bool Poll(SensorSample& out);

private:
    std::uintptr_t handle = static_cast<std::uintptr_t>(~static_cast<std::uintptr_t>(0));
    bool wsaStarted = false;
};
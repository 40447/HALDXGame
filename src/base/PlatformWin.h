#pragma once

// 避免 windows.h 带来不少历史包袱（包含 winsock.h、min/max 宏等）
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif

// 顺序非常重要：先 winsock2 / ws2tcpip，最后再 windows.h
#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>

#pragma once

#ifndef WINMAIN_H
#define WINMAIN_H

// 定义下面这些宏以去掉 Windows 中那些不需要的组件
#ifndef FULL_WINTARD
  #ifndef WIN32_LEAN_AND_MEAN
  #define WIN32_LEAN_AND_MEAN
  #endif
  #define NOGDICAPMASKS
  #define NOSYSMETRICS
  #define NOMENUS
  #define NOICONS
  #define NOSYSCOMMANDS
  #define NORASTEROPS
  #define OEMRESOURCE
  #define NOATOM
  #define NOCLIPBOARD
  #define NOCOLOR
  #define NOCTLMGR
  #define NODRAWTEXT
  #define NOKERNEL
  // #define NONLS
  #define NOMEMMGR
  #define NOMETAFILE
  #define NOOPENFILE
  #define NOSCROLL
  #define NOSERVICE
  #define NOSOUND
  #define NOTEXTMETRIC
  #define NOWH
  #define NOCOMM
  #define NOKANJI
  #define NOHELP
  #define NOPROFILER
  #define NODEFERWINDOWPOS
  #define NOMCX
  #define NORPC
  #define NOPROXYSTUB
  #define NOIMAGE
  #define NOTAPE
#endif

#ifndef NOMINMAX
#define NOMINMAX
#endif

// 关键顺序：先 WinSock2 / WS2tcpip，再 Windows.h
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <Windows.h>

#endif // WINMAIN_H

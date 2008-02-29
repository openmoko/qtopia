// ### revise should use qglobal.h
#ifndef RSYNC_H
#define RSYNC_H
#ifndef _MSC_VER
#include "config_linux.h"
#else
#define _OS_WIN32_
#define Q_OS_WIN32
#define DO_RS_TRACE
#include "config_win32.h"
#endif

#endif

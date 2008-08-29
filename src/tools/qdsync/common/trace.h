/****************************************************************************
**
** Copyright (C) 2000-2008 TROLLTECH ASA. All rights reserved.
**
** This file is part of the Opensource Edition of the Qtopia Toolkit.
**
** This software is licensed under the terms of the GNU General Public
** License (GPL) version 2.
**
** See http://www.trolltech.com/gpl/ for GPL licensing information.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/
#ifndef TRACE_H
#define TRACE_H

#include <qdglobal.h>
#include <qdebug.h>

class QD_EXPORT _TraceDebugMethodPrefix : public QDebug
{
public:
    _TraceDebugMethodPrefix( const QString &indentspaces );
    _TraceDebugMethodPrefix( const _TraceDebugMethodPrefix &other );
    ~_TraceDebugMethodPrefix();

#define TRACEDEBUG_BODY\
    {\
        if ( _tstate == 0 ) {\
            _tstate++;\
            nospace();\
            QDebug::operator<<(t);\
            QDebug::operator<<("()");\
            space();\
        } else {\
            QDebug::operator<<(t);\
        }\
        return (*this);\
    }

    inline QDebug &operator<<(QChar t) TRACEDEBUG_BODY
    inline QDebug &operator<<(bool t) TRACEDEBUG_BODY
    inline QDebug &operator<<(char t) TRACEDEBUG_BODY
    inline QDebug &operator<<(signed short t) TRACEDEBUG_BODY
    inline QDebug &operator<<(unsigned short t) TRACEDEBUG_BODY
    inline QDebug &operator<<(signed int t) TRACEDEBUG_BODY
    inline QDebug &operator<<(unsigned int t) TRACEDEBUG_BODY
    inline QDebug &operator<<(signed long t) TRACEDEBUG_BODY
    inline QDebug &operator<<(unsigned long t) TRACEDEBUG_BODY
    inline QDebug &operator<<(qint64 t)TRACEDEBUG_BODY
    inline QDebug &operator<<(quint64 t)TRACEDEBUG_BODY
    inline QDebug &operator<<(float t) TRACEDEBUG_BODY
    inline QDebug &operator<<(double t) TRACEDEBUG_BODY
    inline QDebug &operator<<(const char* t) TRACEDEBUG_BODY
    inline QDebug &operator<<(const QString & t) TRACEDEBUG_BODY
    inline QDebug &operator<<(const QLatin1String &t) TRACEDEBUG_BODY
    inline QDebug &operator<<(const QByteArray & t) TRACEDEBUG_BODY
    inline QDebug &operator<<(const void * t) TRACEDEBUG_BODY
#undef TRACEDEBUG_BODY

private:
    int _tstate;
    QString indentspaces;
};

class QD_EXPORT _Trace
{
public:
    _Trace( const char *category, bool (*enabledHook) () );
    ~_Trace();
    bool enabled();
    _TraceDebugMethodPrefix methodLog();
    QDebug log();
private:
    void init();

    const char *category;
    int indent;
    int threadNumber;
    bool mEnabled;
};

template<class T> inline int _trace_opt(const T &) { return 1; }
/* Catch compile time enums (at most one symbol) */
template<> inline int _trace_opt<int>(const int & v) { return v; }

#define TRACE_OPTION(dbgcat,expr) \
    class dbgcat##_TraceLog { \
    public: \
	static inline bool enabled() { return expr; }\
	static inline _Trace trace() { return _Trace(#dbgcat, dbgcat##_TraceLog::enabled); }\
    };

// So that TRACE() can work.
class _TraceLog {
public: \
    static inline bool enabled() { return true; }
    static inline _Trace trace() { return _Trace("", _TraceLog::enabled); }
};

#ifdef QTOPIA_DESKTOP
// Circular dependency :(
#include <qtopiadesktoplog.h>

#define TRACE(dbgcat)\
    /*qLog(TRACE) << "TRACE(" << #dbgcat << ") called in file" << __FILE__ << "line" << __LINE__;*/\
    _Trace _trace_object = dbgcat##_TraceLog::trace();\
    if (!_trace_object.enabled()); else _trace_object.methodLog()

#else
#include <qtopialog.h>

#define QD_LOG_OPTION(x)\
    QLOG_OPTION_VOLATILE(QDSync_##x,qtopiaLogRequested("QDSync_" #x))\
    TRACE_OPTION(QDSync_##x,qtopiaLogRequested("QDSync_" #x))

// So that TRACE() can work.
QLOG_OPTION_VOLATILE(QDSync_,qtopiaLogRequested("QDSync_"));
TRACE_OPTION(QDSync_,qtopiaLogRequested("QDSync_"));

#define TRACE(dbgcat)\
    /*qLog(TRACE) << "TRACE(" << #dbgcat << ") called in file" << __FILE__ << "line" << __LINE__;*/\
    _Trace _trace_object = QDSync_##dbgcat##_TraceLog::trace();\
    if (!_trace_object.enabled()); else _trace_object.methodLog()

#endif

#define LOG()\
    /*qLog(TRACE) << "LOG() called in file" << __FILE__ << "line" << __LINE__;*/\
    if (!_trace_object.enabled()); else _trace_object.log()

#define WARNING()\
    /*qLog(TRACE) << "WARNING() called in file" << __FILE__ << "line" << __LINE__;*/\
    _trace_object.log()

// Redefine Q_ASSERT to something USEFUL!
#ifndef Q_OS_UNIX
#undef Q_ASSERT
#define Q_ASSERT(x) if ( x ); else qFatal(QString("Q_ASSERT failed! %3 at %1 line %2").arg(__FILE__).arg(__LINE__).arg(#x).toLocal8Bit().constData())
#endif

#endif // TRACE_H


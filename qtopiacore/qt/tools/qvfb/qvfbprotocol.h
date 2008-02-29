/****************************************************************************
**
** Copyright (C) 1992-2006 TROLLTECH ASA. All rights reserved.
**
** This file is part of the Phone Edition of the Qt Toolkit.
**
** $TROLLTECH_DUAL_LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/
#ifndef QVFBPROTOCOL_H
#define QVFBPROTOCOL_H

#include <QImage>
#include <QVector>
#include <QColor>
class QVFbKeyProtocol;
class QVFbMouseProtocol;
class QVFbViewProtocol : public QObject
{
    Q_OBJECT
public:
    QVFbViewProtocol(int display_id, QObject *parent = 0);

    virtual ~QVFbViewProtocol();

    int id() const { return mDisplayId; }

    void sendKeyboardData(QString unicode, int keycode,
            int modifiers, bool press, bool repeat);
    void sendMouseData(const QPoint &pos, int buttons, int wheel);

    virtual int width() const = 0;
    virtual int height() const = 0;
    virtual int depth() const = 0;
    virtual int linestep() const = 0;
    virtual int  numcols() const = 0;
    virtual QVector<QRgb> clut() const = 0;
    virtual unsigned char *data() const = 0;

    virtual void setRate(int) {}
public slots:
    virtual void flushChanges();

signals:
    void displayDataChanged(const QRect &);

protected:
    virtual QVFbKeyProtocol *keyHandler() const = 0;
    virtual QVFbMouseProtocol *mouseHandler() const = 0;

private:
    int mDisplayId;
};

class QVFbKeyProtocol
{
public:
    QVFbKeyProtocol(int display_id) : mDisplayId(display_id) {}
    virtual ~QVFbKeyProtocol() {}

    int id() const { return mDisplayId; }

    virtual void sendKeyboardData(QString unicode, int keycode,
            int modifiers, bool press, bool repeat) = 0;

private:
    int mDisplayId;
};

class QVFbMouseProtocol
{
public:
    QVFbMouseProtocol(int display_id) : mDisplayId(display_id) {}
    virtual ~QVFbMouseProtocol() {}

    int id() const { return mDisplayId; }

    virtual void sendMouseData(const QPoint &pos, int buttons, int wheel) = 0;

private:
    int mDisplayId;
};

/* since there is very little variation in input protocols defaults are
   provided */

class QVFbKeyPipeProtocol : public QVFbKeyProtocol
{
public:
    QVFbKeyPipeProtocol(int display_id);
    ~QVFbKeyPipeProtocol();

    void sendKeyboardData(QString unicode, int keycode,
            int modifiers, bool press, bool repeat);

    QString pipeName() const { return fileName; }
private:
    int fd;
    QString fileName;
};

class QVFbMousePipe: public QVFbMouseProtocol
{
public:
    QVFbMousePipe(int display_id);
    ~QVFbMousePipe();

    void sendMouseData(const QPoint &pos, int buttons, int wheel);

    QString pipeName() const { return fileName; }
protected:
    int fd;
    QString fileName;
};

class QTimer;
class QVFbMouseLinuxTP : public QObject, public QVFbMousePipe
{
    Q_OBJECT
public:
    QVFbMouseLinuxTP(int display_id);
    ~QVFbMouseLinuxTP();

    void sendMouseData(const QPoint &pos, int buttons, int wheel);

protected slots:
    void repeatLastPress();

protected:
    void writeToPipe(const QPoint &pos, ushort pressure);
    QPoint lastPos;
    QTimer *repeater;
};
#endif

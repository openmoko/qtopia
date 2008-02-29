/****************************************************************************
**
** Copyright (C) 2000-2007 TROLLTECH ASA. All rights reserved.
**
** This file is part of the Phone Edition of the Qtopia Toolkit.
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

#include <QObject>

class QIODevice;
class SmilTransferServerPrivate;
class SmilElement;

class SmilDataSource
{
public:
    explicit SmilDataSource(const QString &t=QString())
        : type(t), dev(0) {}
    virtual ~SmilDataSource() {}

    virtual void setDevice(QIODevice *d) { dev = d; }
    QIODevice *device() const { return dev; }

    void setMimeType(const QString &t) { type = t; }
    const QString &mimeType() const { return type; }

private:
    QString type;
    QIODevice *dev;
};

class SmilTransferServer : public QObject
{
    Q_OBJECT
public:
    explicit SmilTransferServer(QObject *parent);
    ~SmilTransferServer();

    void requestData(SmilElement *e, const QString &src);
    void endData(SmilElement *e, const QString &src);

signals:
    void transferRequested(SmilDataSource *s, const QString &src);
    void transferCancelled(SmilDataSource *s, const QString &src);

private:
    void requestTransfer(SmilDataSource *e, const QString &src);
    void endTransfer(SmilDataSource *e, const QString &src);

private:
    QString base;
    SmilTransferServerPrivate *d;
    friend class SmilDataStore;
};


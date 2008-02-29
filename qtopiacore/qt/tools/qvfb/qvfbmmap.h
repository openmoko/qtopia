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
#ifndef QVFB_MMAP_PROTOCOL_H
#define QVFB_MMAP_PROTOCOL_H

#include "qvfbprotocol.h"

class QVFbHeader;
class QTimer;
class QMMapViewProtocol : public QVFbViewProtocol
{
public:
    QMMapViewProtocol(int display_id, const QSize &size, int depth, QObject *parent = 0);
    ~QMMapViewProtocol();

    int width() const;
    int height() const;
    int depth() const;
    int linestep() const;
    int  numcols() const;
    QVector<QRgb> clut() const;
    unsigned char *data() const;

    void setRate(int);
    int rate() const;

protected slots:
    void flushChanges();

protected:
    QVFbKeyProtocol *keyHandler() const { return kh; }
    QVFbMouseProtocol *mouseHandler() const { return mh; }

private:
    QVFbKeyPipeProtocol *kh;
    QVFbMouseLinuxTP *mh;
    QVFbHeader *hdr;
    QString fileName;
    int fd;
    size_t dataSize;
    size_t displaySize;
    unsigned char *dataCache;
    QTimer *mRefreshTimer;
};

#endif

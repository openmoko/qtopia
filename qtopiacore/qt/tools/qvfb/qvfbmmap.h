/****************************************************************************
**
** Copyright (C) 1992-2007 Trolltech ASA. All rights reserved.
**
** This file is part of the tools applications of the Qt Toolkit.
**
** This file may be used under the terms of the GNU General Public
** License version 2.0 as published by the Free Software Foundation
** and appearing in the file LICENSE.GPL included in the packaging of
** this file.  Please review the following information to ensure GNU
** General Public Licensing requirements will be met:
** http://www.trolltech.com/products/qt/opensource.html
**
** If you are unsure which license is appropriate for your use, please
** review the following information:
** http://www.trolltech.com/products/qt/licensing.html or contact the
** sales department at sales@trolltech.com.
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

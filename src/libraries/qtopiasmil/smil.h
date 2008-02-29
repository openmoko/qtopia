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

#ifndef SMIL_H
#define SMIL_H

#include <qwidget.h>
#include <qtopiaglobal.h>

class QTextStream;
class SmilDataSource;
class SmilViewPrivate;
class SmilElement;

class QTOPIASMIL_EXPORT SmilView : public QWidget
{
    Q_OBJECT
public:
    explicit SmilView(QWidget *parent=0, Qt::WFlags f=0);
    ~SmilView();

    bool setSource(const QString &str);
    SmilElement *rootElement() const;

public slots:
    void play();
    void reset();

signals:
    void finished();
    void transferRequested(SmilDataSource *, const QString &src);
    void transferCancelled(SmilDataSource *, const QString &src);

protected:
    void paintEvent(QPaintEvent *p);

private:
    void clear();

private:
    SmilViewPrivate *d;
};

#endif

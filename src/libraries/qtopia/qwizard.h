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

#ifndef QWIZARD_H
#define QWIZARD_H

#include <QWidget>
#include <QMap>

#include <qtopiaglobal.h>

class QAction;
class QLabel;
class QWidgetStack;
class QPushButton;
class QPushButton;
class QVBoxLayout;
class QHBoxLayout;
class QFrame;
class QSignalMapper;
class QWizardPrivate;

class QTOPIA_EXPORT QWizard : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(QString prevText READ prevText WRITE setPrevText )
    Q_PROPERTY(QString nextText READ nextText WRITE setNextText )

public:
    explicit QWizard( QWidget *parent = 0, Qt::WFlags f = 0 );
    virtual ~QWizard();
    void loadPages( QObject *p = 0 );
    void addPage( QWidget * );
    /*! \reimp */
    void addPage( QWidget *, const QString & );
    QWidget *page ( int ) const;
    int indexOf( QWidget *page ) const;
    QWidget *currentPage() const;
    QString prevText() const;
    QString nextText() const;
    void setPrevText( QString );
    void setNextText( QString );
    QSize sizeHint() const;
    void connectPageEnabler( QObject *, const char *, QWidget * );

signals:
    void accept();
    void reject();
    void aboutToShowPage( int );

public slots:
    virtual void showPage( int );
    virtual void setAppropriate ( QWidget *, bool );
    void setNextEnabled( QWidget *, bool );
    void setPrevEnabled( QWidget *, bool );
    void setFinishEnabled( QWidget *, bool );

private:
    friend class QWizardPrivate;
    QWizardPrivate *d;
};

#endif

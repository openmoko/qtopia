/**********************************************************************
** Copyright (C) 2000-2005 Trolltech AS.  All rights reserved.
**
** This file is part of the Qtopia Environment.
** 
** This program is free software; you can redistribute it and/or modify it
** under the terms of the GNU General Public License as published by the
** Free Software Foundation; either version 2 of the License, or (at your
** option) any later version.
** 
** A copy of the GNU GPL license version 2 is included in this package as 
** LICENSE.GPL.
**
** This program is distributed in the hope that it will be useful, but
** WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. 
** See the GNU General Public License for more details.
**
** In addition, as a special exception Trolltech gives permission to link
** the code of this program with Qtopia applications copyrighted, developed
** and distributed by Trolltech under the terms of the Qtopia Personal Use
** License Agreement. You must comply with the GNU General Public License
** in all respects for all of the code used other than the applications
** licensed under the Qtopia Personal Use License Agreement. If you modify
** this file, you may extend this exception to your version of the file,
** but you are not obligated to do so. If you do not wish to do so, delete
** this exception statement from your version.
** 
** See http://www.trolltech.com/gpl/ for GPL licensing information.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/

#ifndef QWIZARDDIALOG_H
#define QWIZARDDIALOG_H

// This class is a copy of QWizard from Qt.
//
// It is copied here to maintain binary compatibility.


#ifndef QT_H
#include <qdialog.h>
#endif // QT_H
#include <qtopia/qpeglobal.h>

class QWizardPrivate;
class QWizardPagePrivate;
class QWizard;
class QHBoxLayout;

class QTOPIA_EXPORT QWizard : public QDialog
{
    Q_OBJECT
public:
    QWizard( QWidget *parent=0, const char *name=0, bool modal=FALSE,
	     WFlags f=0 );
    ~QWizard();

    void show();

    void setFont( const QFont & font );

    virtual void addPage( QWidget *, const QString & );
    virtual void removePage( QWidget * );

    QString title( QWidget * ) const;
    void setTitle( QWidget *, const QString & );
    
    virtual void showPage( QWidget * );

    QWidget * currentPage() const;

    QWidget* page( int pos ) const;
    int pageCount() const;

    virtual bool appropriate( QWidget * ) const;
    virtual void setAppropriate( QWidget *, bool );

    QPushButton * backButton() const;
    QPushButton * nextButton() const;
    QPushButton * finishButton() const;
    QPushButton * cancelButton() const;
    QPushButton * helpButton() const;

    bool eventFilter( QObject *, QEvent * );

public slots:
    virtual void setBackEnabled( QWidget *, bool );
    virtual void setNextEnabled( QWidget *, bool );
    virtual void setFinishEnabled( QWidget *, bool );

    virtual void setHelpEnabled( QWidget *, bool );

    virtual void setFinish(  QWidget *, bool );

protected slots:
    virtual void back();
    virtual void next();
    virtual void help();

signals:
    void helpClicked();
    void selected( const QString& );

protected:
    virtual void layOutButtonRow( QHBoxLayout * );
    virtual void layOutTitleRow( QHBoxLayout *, const QString & );

private:
    void setBackEnabled( bool );
    void setNextEnabled( bool );

    void setHelpEnabled( bool );

    void setNextPage( QWidget * );

    void updateButtons();

    void layOut();
    QWizardPrivate *d;

private:	// Disabled copy constructor and operator=
#if defined(Q_DISABLE_COPY)
    QWizard( const QWizard & );
    QWizard& operator=( const QWizard & );
#endif
};

#endif // QWIZARD_H

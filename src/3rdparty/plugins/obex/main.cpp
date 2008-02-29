/**********************************************************************
** Copyright (C) 2000-2002 Trolltech AS.  All rights reserved.
**
** This file is part of the Qtopia Environment.
**
** Licensees holding valid Qtopia Developer license may use this
** file in accordance with the Qtopia Developer License Agreement
** provided with the Software.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING
** THE WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR
** PURPOSE.
**
** email sales@trolltech.com for information about Qtopia License
** Agreements.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/
#include <qpe/qpeapplication.h>
#include "qirserver.h"


#include <qmessagebox.h>
#include <qpushbutton.h>
#include <qcheckbox.h>
#include <qtimer.h>

#include <qvbox.h>
#include <qlineedit.h>
#include <qlabel.h>


class Beamer : public QVBox
{
    Q_OBJECT;
public:
    Beamer( QWidget *parent = 0, const char * name = 0 );

private slots:
    void incoming( int, const QString&, const QString& )  ;
    void beam();
private:
    QIrServer *ir;
    QLineEdit *filename;
};

Beamer::Beamer( QWidget *parent, const char * name )
    :QVBox( parent, name )
{
    ir  = new QIrServer( this );

    connect( ir, SIGNAL(receiving( int, const QString&, const QString& )),
	     this, SLOT( incoming( int, const QString&, const QString& )) );
    //connect( ir, SIGNAL(progress(int)),
	     //this, SLOT( incomingProgress(int)) );


    QCheckBox *rcv = new QCheckBox( tr("Receive"), this );
    connect( rcv, SIGNAL(toggled(bool)),
	     ir, SLOT(setReceivingEnabled(bool)) );


    QPushButton *beam = new QPushButton( tr("Beam file:"), this );
    filename = new QLineEdit( this );
    connect( beam, SIGNAL(clicked()), this, SLOT(beam()) );

    QPushButton *cancel = new QPushButton( tr("Cancel"), this );
    connect( cancel, SIGNAL(clicked()),
	     ir, SLOT(cancel()) );


    QPushButton *quit = new QPushButton( tr("Quit"), this );
    connect( quit, SIGNAL(clicked()), qApp, SLOT(quit()) );
}

void Beamer::beam()
{
    ir->beam( filename->text() );
}

void Beamer::incoming( int reclen, const QString& filename,
		       const QString& mimetype )
{
    QString msg = tr("Do you want to accept\n %1\n type %2\n "
		     "size %3?").arg(filename).arg(mimetype).arg(reclen);
    if ( QMessageBox::warning( (QWidget*)0, tr( "Receive IR" ),
			       msg, QMessageBox::Yes,
			       QMessageBox::No|QMessageBox::Default|
			       QMessageBox::Escape, 0 ) 
	 != QMessageBox::Yes ) {
	ir->cancel();
    }
}

#include "main.moc"

int main( int argc, char ** argv )
{
    QPEApplication a(argc,argv);

    Beamer mw;

    mw.show();
    a.setMainWidget( &mw );

    return a.exec();

}

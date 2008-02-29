/****************************************************************************
**
** Copyright (C) 2000-2007 TROLLTECH ASA. All rights reserved.
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
#include <qpeapplication.h>
#include <qmultilineedit.h>
#include <qvbox.h>

#include <popupim.h>

/*
   A Stub class to allow testing of a Popup Input Method in isolation
   of plugin code and the Qtopia server
*/
class TestMultiLineEdit : public QMultiLineEdit
{
    Q_OBJECT
public:
    TestMultiLineEdit(QWidget *parent, const char *name = 0)
	: QMultiLineEdit(parent, name) {}

public slots:
    void interpretKeyPress( ushort unicode, ushort keycode,
	    ushort modifiers, bool press, bool repeat )
    {
	QKeyEvent ke(press ? QEvent::KeyPress : QEvent::KeyRelease,
		keycode, 0, modifiers, QChar(unicode), repeat);

	if (press)
	    keyPressEvent(&ke);
	else
	    keyReleaseEvent(&ke);
    }
};

int main(int argc, char **argv) 
{
    QPEApplication a(argc, argv);

    QVBox *vb = new QVBox();
    TestMultiLineEdit *mle = new TestMultiLineEdit(vb);
    PopupIM *pi = new PopupIM(vb);
    QObject::connect(pi, SIGNAL(keyPress(ushort,ushort,ushort,bool,bool)),
	    mle, SLOT(interpretKeyPress(ushort,ushort,ushort,bool,bool)));

    a.showMainWidget(vb);
    return a.exec();
}

#include "main.moc"

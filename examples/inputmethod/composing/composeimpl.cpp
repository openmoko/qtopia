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

#include <QStringList>
#include <QDebug>
#include "composeimpl.h"
#include "composeim.h"

/*
   Constructs the ComposeImpl
*/
ComposeImpl::ComposeImpl(QObject *parent)
    : QtopiaInputMethod(parent), input(0), ref(0)
{
    /* XPM */
    static const char * pix_xpm[] = {
	"16 13 3 1",
	" 	c #FFFFFFFFFFFF",
	"#	c #000000000000",
	".	c #FFFFFFFFFFFF",
	"                ",
	"      ####      ",
	"    ##....##    ",
	"   #........#   ",
	"   #........#   ",
	"  #...####...#  ",
	"  #..#.......#  ",
	"  #..#.......#  ",
	"  #..#.......#  ",
	"   #..####..#   ",
	"   #........#   ",
	"    ##....##    ",
	"      ####      "};
    icn = QIcon(QPixmap((const char **)pix_xpm));
}

/*
   Destroys the ComposeImpl
*/
ComposeImpl::~ComposeImpl()
{
    if (input)
	delete input;
}

/*
   Returns the QWSInputMethod provided by the plugin.
*/
QWSInputMethod *ComposeImpl::inputModifier( )
{
    if ( !input )
	input = new ComposeIM( );
    return input;
}

void ComposeImpl::setHint(const QString &hint, bool)
{
    inputModifier();
    if (hint.isEmpty() || hint == "numbers" || hint == "phone") {
        if (input->active()) {
            input->setActive(false);
            emit stateChanged(Sleeping);
        }
    } else if (!input->active()) {
        input->setActive(true);
        emit stateChanged(Ready);
    }
}

/*
   Resets the state of the input method.
*/
void ComposeImpl::reset()
{
    if ( input )
	input->reset();
}

/*
   Returns the state of the input method. this allows
   the server to hide the input method icon if no
   input methods are active.
*/
QtopiaInputMethod::State ComposeImpl::state() const
{
    return (input && input->active()) ? Ready : Sleeping;
}

/*
   Returns the icon for the input method plugin
*/
QIcon ComposeImpl::icon() const
{
    return icn;
}


/*
   Returns the name of the input method plugin suitable
   for displaying to the user.
*/
QString ComposeImpl::name() const
{
    return qApp->translate( "InputMethods", "Compose" );
}

/*
   Returns the name of the input method plugin suitable
   for using to identify it in code
*/
QString ComposeImpl::identifier() const
{
    return "Compose";
}

/*
  Returns the version string for the input method.
*/
QString ComposeImpl::version() const
{
    return "4.0.0";
}

/*
  Returns the properties describing the capability and
  requirements of the input method.

  In this case indicates that the input method should
  only be loaded if there is a keypad present and
  that the input method will modify key or mouse input.
*/
int ComposeImpl::properties() const
{
    return RequireKeypad | InputModifier;
}

/* Required to make the plugin loadable.  not also the 
   ref member and initializing ref to 0 in the constructor
*/
QTOPIA_EXPORT_PLUGIN(ComposeImpl)

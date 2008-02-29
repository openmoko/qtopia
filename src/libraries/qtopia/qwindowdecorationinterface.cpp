/****************************************************************************
**
** Copyright (C) 2000-2006 TROLLTECH ASA. All rights reserved.
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

#include "qwindowdecorationinterface.h"

/*! \class QWindowDecorationInterface
  \brief The QWindowDecorationInterface class provides an interface for Qtopia
  window decoration styles.

  Window decoration styles may be added to Qtopia via plug-ins. In order to
  write a style plug-in you must create an interface to your QStyle derived
  class by deriving from the QWindowDecorationInterface class and implementing
  the pure virtual functions.

  The window being decorated is defined by the
  QWindowDecorationPlugin::WindowData struct:
\code
struct WindowData {
    const QWidget *window;
    QRect rect;
    QPalette palette;
    QString caption;
    enum Flags { Maximized=0x01, Dialog=0x02, Active=0x04 };
    Q_UINT32 flags;
    Q_UINT32 reserved;
};
\endcode
*/

/*! \internal
    \fn QWindowDecorationInterface::~QWindowDecorationInterface()
*/

/*! \enum QWindowDecorationInterface::WindowData::Flags

    \value Maximized The window is maximized.
    \value Dialog The window is a dialog.
    \value Active The window has keyboard focus.

*/

/*! \enum QWindowDecorationInterface::Metric

  \value TitleHeight the height of the title.
  \value LeftBorder the width of the border on the left of the window.
  \value RightBorder the width of the border on the right of the window.
  \value TopBorder the width of the border on the top of the window, above
    the title bar.
  \value BottomBorder the width of the border on the bottom of the window.
  \value OKWidth the width of the OK button.
  \value CloseWidth the width of the Close (X) button.
  \value HelpWidth the width of the Help (?) button.
  \value MaximizeWidth the width of the maximize/restore button.
  \value CornerGrabSize the size of the area allowing diagonal resize at
    each corner of the window.
*/

/*! \enum QWindowDecorationInterface::Button

  \value OK the OK button.
  \value Close the close button.
  \value Help the help button.
  \value Maximize the maximize/restore button.
*/

/*! \enum QWindowDecorationInterface::Area

  \value Border defines the entire decoration area, excluding the title bar.
  \value Title defines the area at the top of the window that contains the
    buttons and captions.  It must extend the full width of the window.
  \value TitleText defines the window caption.
*/

/*! \fn int QWindowDecorationInterface::metric( Metric m, const WindowData *wd ) const

  returns the metric \a m for the window \a wd.
*/

/*! \fn void QWindowDecorationInterface::drawArea( Area area, QPainter *p, const WindowData *wd ) const

  Draw the specified \a area using QPainter \a p for window \a wd.
*/

/*! \fn void QWindowDecorationInterface::drawButton( Button b, QPainter *p, const WindowData *wd, int x, int y, int w, int h, QDecoration::DecorationState state ) const

  Draw button \a b with QPainter \a p for window \a wd within the bounds
  supplied by \a x, \a y, \a w, \a h in the state specified by \a state.
*/

/*! \fn QRegion QWindowDecorationInterface::mask( const WindowData *wd ) const

  Returns the mask of the decoration including all borders and the title
  for window \a wd as a QRegion.  The window decorations do not necessarily
  need to be rectangular, however the title bar  must be rectangular and
  must be the width of the window.  This ensures the title is drawn correctly
  for maximized windows.
*/

/*! \fn QString QWindowDecorationInterface::name() const

  The name() function returns the name of the decoration. This will
  be displayed in the appearance settings dialog.
*/

/*! \fn QPixmap QWindowDecorationInterface::icon() const

  The icon() function returns the icon of the decoration. This may
  be displayed in the appearance settings dialog.
*/


QWindowDecorationPlugin::QWindowDecorationPlugin(QObject*)
{
}


/*! \fn QWindowDecorationPlugin::~QWindowDecorationPlugin()

  Deconstructor.
*/
QWindowDecorationPlugin::~QWindowDecorationPlugin()
{
}

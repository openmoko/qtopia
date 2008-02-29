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

#include <qtopiafeatures.h>
#include "screenclick.h"
#include <qwindowsystem_qws.h>
#include <QSettings>

/*!
  \class ScreenClick
  \ingroup QtopiaServer::Task
  \brief The ScreenClick class enables an audible clicking sound
         whenever the stylus is used.

  The ScreenClick class is not a true task.  Instead, a real task should derive
  from the ScreenClick baseclass and implement the ScreenClick::screenClick()
  virtual method.

  For example, an implementation may look like this:

  \code
  class MyScreenClick : public ScreenClick
  {
  protected:
      virtual void screenClick(bool pressed)
      {
          // Make click noise
      }
  };
  QTOPIA_TASK(MyScreenClick, MyScreenClick);
  \endcode

  Screen clicking will only occur when the \c {Trolltech/Sound/System/Touch}
  configuration variable is true, otherwise ScreenClick::screenClick() will not
  be called.  Creating this class automatically enables the "AudibleScreenClick"
  QtopiaFeature.

  \sa KeyClick
 */

/*!
  Construct a new ScreenClick instance.
 */
ScreenClick::ScreenClick()
: m_clickenabled(false), m_up(true)
{
    QtopiaServerApplication::instance()->installQWSEventFilter(this);
    connect(qApp, SIGNAL(volumeChanged(bool)), this, SLOT(rereadVolume()));
    rereadVolume();
    QtopiaFeatures::setFeature("AudibleScreenClick");
}

/*!
  Destroys the ScreenClick instance.
 */
ScreenClick::~ScreenClick()
{
    QtopiaServerApplication::instance()->removeQWSEventFilter(this);
}

/*!
  \fn void ScreenClick::screenClick(bool pressed)

  Called whenever the user taps or releases the screen.  \a pressed will be
  true when the user taps the screen, and false when they release.
 */

/*! \internal */
bool ScreenClick::qwsEventFilter(QWSEvent *e)
{
    if ( e->type == QWSEvent::Mouse ) {
        QWSMouseEvent *me = (QWSMouseEvent *)e;
        if ( me->simpleData.state&Qt::LeftButton ) {
            if ( m_up ) {
                m_up = false;
                if(m_clickenabled)
                    screenClick(true);
            }
        } else if ( !m_up ) {
            m_up = true;
            if(m_clickenabled)
                screenClick(false);
        }
    }
    return false;
}

void ScreenClick::rereadVolume()
{
    QSettings cfg("Trolltech","Sound");
    cfg.beginGroup("System");
    m_clickenabled = cfg.value("Touch").toBool();
}


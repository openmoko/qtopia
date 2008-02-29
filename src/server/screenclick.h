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

#ifndef _SCREENCLICK_H_
#define _SCREENCLICK_H_

#include "qtopiaserverapplication.h"

class ScreenClick : public QObject,
                    public QtopiaServerApplication::QWSEventFilter
{
Q_OBJECT
public:
    ScreenClick();
    virtual ~ScreenClick();

protected:
    virtual bool qwsEventFilter( QWSEvent * );
    virtual void screenClick(bool) = 0;

private slots:
    void rereadVolume();

private:
    bool m_clickenabled;
    bool m_up;
};

#endif // _SCREENCLICK_H_

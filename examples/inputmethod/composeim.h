/****************************************************************************
** $Id: .emacs,v 1.3 1998/02/20 15:06:53 agulbra Exp $
**
** Definition of something or other
**
** Created : 979899
**
** Copyright (C) 1997 by Troll Tech AS.  All rights reserved.
**
****************************************************************************/

#ifndef COMPOSEIM_H
#define COMPOSEIM_H
#include <qwindowsystem_qws.h>


class ComposeIM : public QWSInputMethod
{
public:
    ComposeIM();

    void reset();
    //void setMicroFocus( int x, int y );
    //void mouseHandler( int index, int state );
    bool filter(int unicode, int keycode, int modifiers, 
			    bool isPress, bool autoRepeat);
    
    enum State { Off, On };

private:
    State state;
    QString composed;
};



#endif

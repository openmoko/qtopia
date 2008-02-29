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

#include <qtopiaglobal.h>
#include <themedview.h>
#include <qwindowsystem_qws.h>
#include "phonethemeview.h"
#include "qsoftmenubarprovider.h"

class QTimer;
class QSettings;

class ContextLabel : public PhoneThemedView, public QWSServer::KeyboardFilter
{
    Q_OBJECT
public:
    ContextLabel( QWidget *parent=0, Qt::WFlags f=0 );
    ~ContextLabel();


    QSize reservedSize() const;

protected:
    void updateLabels();
    virtual bool filter(int unicode, int keycode, int modifiers, bool press,
                        bool autoRepeat);

protected slots:
    void itemPressed(ThemeItem *item);
    void itemReleased(ThemeItem *item);
    void keyChanged(const QSoftMenuBarProvider::MenuButton &);

protected:
    virtual void themeLoaded(const QString &);

private:
    struct Button {
        int key;
        ThemeImageItem *imgItem;
        ThemeTextItem *txtItem;
        bool changed;
    };

    int buttonForItem(ThemeItem *item) const;

    Button *buttons;
    int buttonCount;
    bool blockUpdates;
    int pressedBtn;
    QSoftMenuBarProvider *menuProvider;
};


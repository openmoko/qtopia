/****************************************************************************
**
** Copyright (C) 1992-2007 Trolltech ASA. All rights reserved.
**
** This file is part of the QtGui module of the Qt Toolkit.
**
** This file may be used under the terms of the GNU General Public
** License version 2.0 as published by the Free Software Foundation
** and appearing in the file LICENSE.GPL included in the packaging of
** this file.  Please review the following information to ensure GNU
** General Public Licensing requirements will be met:
** http://www.trolltech.com/products/qt/opensource.html
**
** If you are unsure which license is appropriate for your use, please
** review the following information:
** http://www.trolltech.com/products/qt/licensing.html or contact the
** sales department at sales@trolltech.com.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef QPUSHBUTTON_H
#define QPUSHBUTTON_H

#include <QtGui/qabstractbutton.h>

QT_BEGIN_HEADER

QT_MODULE(Gui)

class QPushButtonPrivate;
class QMenu;

class Q_GUI_EXPORT QPushButton : public QAbstractButton
{
    Q_OBJECT

    Q_PROPERTY(bool autoDefault READ autoDefault WRITE setAutoDefault)
    Q_PROPERTY(bool default READ isDefault WRITE setDefault)
    Q_PROPERTY(bool flat READ isFlat WRITE setFlat)

public:
    explicit QPushButton(QWidget *parent=0);
    explicit QPushButton(const QString &text, QWidget *parent=0);
    QPushButton(const QIcon& icon, const QString &text, QWidget *parent=0);
    ~QPushButton();

    QSize sizeHint() const;

    bool autoDefault() const;
    void setAutoDefault(bool);
    bool isDefault() const;
    void setDefault(bool);

#ifndef QT_NO_MENU
    void setMenu(QMenu* menu);
    QMenu* menu() const;
#endif

    void setFlat(bool);
    bool isFlat() const;

public Q_SLOTS:
#ifndef QT_NO_MENU
    void showMenu();
#endif

protected:
    bool event(QEvent *e);
    void paintEvent(QPaintEvent *);
    void keyPressEvent(QKeyEvent *);
    void focusInEvent(QFocusEvent *);
    void focusOutEvent(QFocusEvent *);

public:
#ifdef QT3_SUPPORT
    QT3_SUPPORT_CONSTRUCTOR QPushButton(QWidget *parent, const char* name);
    QT3_SUPPORT_CONSTRUCTOR QPushButton(const QString &text, QWidget *parent, const char* name);
    QT3_SUPPORT_CONSTRUCTOR QPushButton(const QIcon& icon, const QString &text, QWidget *parent, const char* name);
    inline QT3_SUPPORT void openPopup()  { showMenu(); }
    inline QT3_SUPPORT bool isMenuButton() const { return menu() !=  0; }
    inline QT3_SUPPORT void setPopup(QMenu* popup) {setMenu(popup); }
    inline QT3_SUPPORT QMenu* popup() const { return menu(); }
#endif

private:
    Q_DISABLE_COPY(QPushButton)
    Q_DECLARE_PRIVATE(QPushButton)
#ifndef QT_NO_MENU        
    Q_PRIVATE_SLOT(d_func(), void _q_popupPressed())
#endif
};

QT_END_HEADER

#endif // QPUSHBUTTON_H

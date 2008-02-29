/****************************************************************************
**
** Copyright (C) 1992-2006 TROLLTECH ASA. All rights reserved.
**
** This file is part of the Phone Edition of the Qt Toolkit.
**
** $TROLLTECH_DUAL_LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef QSTATUSBAR_H
#define QSTATUSBAR_H

#include <QtGui/qwidget.h>

QT_BEGIN_HEADER

QT_MODULE(Gui)

#ifndef QT_NO_STATUSBAR

class QStatusBarPrivate;

class Q_GUI_EXPORT QStatusBar: public QWidget
{
    Q_OBJECT

    Q_PROPERTY(bool sizeGripEnabled READ isSizeGripEnabled WRITE setSizeGripEnabled)

public:
    explicit QStatusBar(QWidget* parent=0);
    virtual ~QStatusBar();

    void addWidget(QWidget *widget, int stretch = 0);
    int insertWidget(int index, QWidget *widget, int stretch = 0);
    void addPermanentWidget(QWidget *widget, int stretch = 0);
    int insertPermanentWidget(int index, QWidget *widget, int stretch = 0);
    void removeWidget(QWidget *widget);

    void setSizeGripEnabled(bool);
    bool isSizeGripEnabled() const;

    QString currentMessage() const;

public Q_SLOTS:
    void showMessage(const QString &text, int timeout = 0);
    void clearMessage();

#ifdef QT3_SUPPORT
public:
    QT3_SUPPORT_CONSTRUCTOR QStatusBar(QWidget* parent, const char* name);
    QT3_SUPPORT void addWidget(QWidget *w, int stretch, bool permanent)
        { if (permanent) addPermanentWidget(w, stretch); else addWidget(w, stretch); }
public Q_SLOTS:
    inline QT_MOC_COMPAT void message(const QString &text, int timeout = 0) { showMessage(text, timeout); }
    inline QT_MOC_COMPAT void clear() { clearMessage(); }
#endif

Q_SIGNALS:
    void messageChanged(const QString &text);

protected:
    void paintEvent(QPaintEvent *);
    void resizeEvent(QResizeEvent *);

    // ### Qt 5: consider making reformat() and hideOrShow() private
    void reformat();
    void hideOrShow();
    bool event(QEvent *);

private:
    Q_DISABLE_COPY(QStatusBar)
    Q_DECLARE_PRIVATE(QStatusBar)
};

#endif // QT_NO_STATUSBAR

QT_END_HEADER

#endif // QSTATUSBAR_H

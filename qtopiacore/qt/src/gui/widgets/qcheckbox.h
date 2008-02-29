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

#ifndef QCHECKBOX_H
#define QCHECKBOX_H

#include <QtGui/qabstractbutton.h>

QT_BEGIN_HEADER

QT_MODULE(Gui)

class QCheckBoxPrivate;

class Q_GUI_EXPORT QCheckBox : public QAbstractButton
{
    Q_OBJECT

    Q_PROPERTY(bool tristate READ isTristate WRITE setTristate)

public:
    explicit QCheckBox(QWidget *parent=0);
    explicit QCheckBox(const QString &text, QWidget *parent=0);


    QSize sizeHint() const;

    void setTristate(bool y = true);
    bool isTristate() const;

    Qt::CheckState checkState() const;
    void setCheckState(Qt::CheckState state);

Q_SIGNALS:
    void stateChanged(int);

protected:
    bool event(QEvent *e);
    bool hitButton(const QPoint &pos) const;
    void checkStateSet();
    void nextCheckState();
    void paintEvent(QPaintEvent *);
    void mouseMoveEvent(QMouseEvent *);

#ifdef QT3_SUPPORT
public:
    enum ToggleState {
        Off =      Qt::Unchecked,
        NoChange = Qt::PartiallyChecked,
        On =       Qt::Checked
    };
    inline QT3_SUPPORT ToggleState state() const
        { return static_cast<QCheckBox::ToggleState>(static_cast<int>(checkState())); }
    inline QT3_SUPPORT void setState(ToggleState state)
        { setCheckState(static_cast<Qt::CheckState>(static_cast<int>(state))); }
    inline QT3_SUPPORT void setNoChange()
        { setCheckState(Qt::PartiallyChecked); }
    QT3_SUPPORT_CONSTRUCTOR QCheckBox(QWidget *parent, const char* name);
    QT3_SUPPORT_CONSTRUCTOR QCheckBox(const QString &text, QWidget *parent, const char* name);
#endif

private:
    Q_DECLARE_PRIVATE(QCheckBox)
    Q_DISABLE_COPY(QCheckBox)
};

QT_END_HEADER

#endif // QCHECKBOX_H

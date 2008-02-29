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

#ifndef Q3BUTTONGROUP_H
#define Q3BUTTONGROUP_H

#include <QtGui/qbuttongroup.h>
#include <Qt3Support/q3groupbox.h>
#include <QtCore/qmap.h>

QT_BEGIN_HEADER

QT_MODULE(Qt3SupportLight)

class QAbstractButton;

class Q_COMPAT_EXPORT Q3ButtonGroup : public Q3GroupBox
{
    Q_OBJECT
    Q_PROPERTY(bool exclusive READ isExclusive WRITE setExclusive)
    Q_PROPERTY(bool radioButtonExclusive READ isRadioButtonExclusive WRITE setRadioButtonExclusive)
    Q_PROPERTY(int selectedId READ selectedId WRITE setButton)

public:
    Q3ButtonGroup(QWidget* parent=0, const char* name=0);
    Q3ButtonGroup(const QString &title,
                  QWidget* parent=0, const char* name=0);
    Q3ButtonGroup(int columns, Qt::Orientation o,
                  QWidget* parent=0, const char* name=0);
    Q3ButtonGroup(int columns, Qt::Orientation o, const QString &title,
                  QWidget* parent=0, const char* name=0);
    ~Q3ButtonGroup();

    bool isExclusive() const;
    bool isRadioButtonExclusive() const { return radio_excl; }
    void setExclusive(bool);
    void setRadioButtonExclusive(bool);

public:
    int insert(QAbstractButton *, int id=-1);
    void remove(QAbstractButton *);
    QAbstractButton    *find(int id) const;
    int id(QAbstractButton *) const;
    int count() const;

    void setButton(int id);

    QAbstractButton *selected() const;
    int selectedId() const;

Q_SIGNALS:
    void pressed(int id);
    void released(int id);
    void clicked(int id);

protected Q_SLOTS:
    void buttonPressed();
    void buttonReleased();
    void buttonClicked();

protected:
    bool event(QEvent * e);

private:
    Q_DISABLE_COPY(Q3ButtonGroup)

    void init();
    void fixChildren() const;
    int insert_helper(QAbstractButton* , int id=-1);

    bool excl_grp; // Not used.
    bool radio_excl;
    QMap<int, QAbstractButton*> buttonIds;
    QButtonGroup group;
};

class Q_COMPAT_EXPORT Q3VButtonGroup : public Q3ButtonGroup
{
    Q_OBJECT
public:
    inline Q3VButtonGroup(QWidget* parent=0, const char* name=0)
        : Q3ButtonGroup(1, Qt::Horizontal /* sic! */, parent, name) {}
    inline Q3VButtonGroup(const QString &title, QWidget* parent=0, const char* name=0)
        : Q3ButtonGroup(1, Qt::Horizontal /* sic! */, title, parent, name) {}

private:
    Q_DISABLE_COPY(Q3VButtonGroup)
};


class Q_COMPAT_EXPORT Q3HButtonGroup : public Q3ButtonGroup
{
    Q_OBJECT
public:
    inline Q3HButtonGroup(QWidget* parent=0, const char* name=0)
        : Q3ButtonGroup(1, Qt::Vertical /* sic! */, parent, name) {}
    inline Q3HButtonGroup(const QString &title, QWidget* parent=0, const char* name=0)
        : Q3ButtonGroup(1, Qt::Vertical /* sic! */, title, parent, name) {}

private:
    Q_DISABLE_COPY(Q3HButtonGroup)
};

QT_END_HEADER

#endif // Q3BUTTONGROUP_H

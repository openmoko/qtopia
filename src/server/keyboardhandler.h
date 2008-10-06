/***************************************************************************
**
** Copyright (C) 2008 Openmoko Inc.
**
** This software is licensed under the terms of the GNU General Public
** License (GPL) version 2 or at your option any later version.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
***************************************************************************/

#ifndef KeyboardHandler_h
#define KeyboardHandler_h

#ifdef QT_ILLUME_LAUNCHER

#include <QDBusConnection>
#include <QObject>

class QWidget;

struct KeyboardData {
    KeyboardData(const QString& _name = QString(), int _id = -1)
        : name(_name)
        , id(_id)
    {}

    QString name;
    int id;
};

/**
 * Handle keyboard switching
 */
class KeyboardHandler : public QObject {
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.openmoko.qtopia.Keyboards")

public:
    KeyboardHandler(QObject*);
    ~KeyboardHandler();

public Q_SLOTS:
    Q_SCRIPTABLE QList<KeyboardData> keyboards() const;
    Q_SCRIPTABLE void setKeyboard(int);

private:
    QMap<int, QWidget*> m_widgets;
    QWidget* m_currentWidget;
};

#endif

#endif

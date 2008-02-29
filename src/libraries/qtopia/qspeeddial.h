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

#if defined(QTOPIA_PHONE) && !defined(SPEEDDIAL_H)
#define SPEEDDIAL_H

#include <qtopiaserviceselector.h>
#include <QListView>

class QSpeedDial;
class QSpeedDialListPrivate;
class QSpeedDialList;


class QTOPIA_EXPORT QSpeedDialList : public QListView
{
    Q_OBJECT
    friend class QSpeedDialDialog;

public:
    explicit QSpeedDialList(QWidget* parent=0);
    ~QSpeedDialList();

    QString currentInput() const;
    void setCurrentInput(const QString&);

    QString rowInput(int row) const;

    void setCurrentRow(int row);
    int currentRow() const;

    int count() const;

public slots:
    void reload(const QString& sd);
    void editItem(int row);
    void editItem();
    void clearItem(int row);
    void clearItem();

signals:
    void currentRowChanged(int row);
    void rowClicked(int row);

protected:
    void keyPressEvent(QKeyEvent*);
    void timerEvent(QTimerEvent*);
    void scrollContentsBy(int dx, int dy);

private slots:
    void select(const QModelIndex& index);
    void click(const QModelIndex& index);
    void selectionChanged();

private:
    void init(const QString&);
    QSpeedDialList(const QString& label, const QString& icon, QWidget* parent);

    QSpeedDialListPrivate* d;
};



class QTOPIA_EXPORT QSpeedDial
{
public:
    // QSpeedDial is very similar to QDeviceButtonManager

    static QString addWithDialog(const QString& label, const QString& icon,
        const QtopiaServiceRequest& action, QWidget* parent);

    static QtopiaServiceDescription* find(const QString& input);
    static void remove(const QString& input);
    static void set(const QString& input, const QtopiaServiceDescription&);
};

#endif

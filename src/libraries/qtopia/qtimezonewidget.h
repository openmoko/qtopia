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
#ifndef QTIMEZONEWIDGET_H
#define QTIMEZONEWIDGET_H

#include <qtopiaglobal.h>
#include <QComboBox>
#include <QStringList>

class QToolButton;
class QTimeZoneWidget;
class QTimeZoneWidgetPrivate;

class QTOPIA_EXPORT QTimeZoneComboBox : public QComboBox
{
    Q_OBJECT
public:
    explicit QTimeZoneComboBox( QWidget* parent=0 );
    ~QTimeZoneComboBox();

    QString currZone() const;
    QString prevZone() const;
    void setCurrZone( const QString& id );

public slots:
    void setToPreviousIndex();

protected:
    friend class QTimeZoneWidget;
    void keyPressEvent( QKeyEvent *e );
    void mousePressEvent(QMouseEvent*e);
    void updateZones();

private slots:
    void handleSystemChannel(const QString&, const QByteArray&);
    void indexChange( const int index );

private:
    QStringList identifiers;
    QStringList extras;
    int         prevIndex1;
    int         prevIndex2;
};

class QTOPIA_EXPORT QTimeZoneWidget : public QWidget
{
    Q_OBJECT
public:
    explicit QTimeZoneWidget( QWidget* parent = 0 );
    ~QTimeZoneWidget();

    // allows the "No Timezone" or "None" option.
    void setLocalIncluded(bool);
    bool localIncluded() const;

    QString currentZone() const;
    void setCurrentZone( const QString& id );

signals:
    void signalNewTz( const QString& id );
private slots:
    void slotTzActive( int index );
    void slotExecute( void );

private:
    QToolButton *cmdTz;
    QTimeZoneComboBox *cmbTz;
    QTimeZoneWidgetPrivate *d;
};

#endif

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

#ifndef QMEDIATOOLS_H
#define QMEDIATOOLS_H

#include <QtCore>

#include <qtopiamedia/qmediacontent.h>

class QMediaContentContextPrivate;

class QTOPIAMEDIA_EXPORT QMediaContentContext : public QObject
{
    Q_OBJECT
public:
    explicit QMediaContentContext( QObject* parent = 0 );
    ~QMediaContentContext();

    void addObject( QObject* object );
    void removeObject( QObject* object );

    QMediaContent* content() const;

signals:
    void contentChanged( QMediaContent* content );

public slots:
    void setMediaContent( QMediaContent* content );

private:
    QMediaContent *m_content;
    QMediaContentContextPrivate *m_d;
};

class QMediaControlNotifierPrivate;

class QTOPIAMEDIA_EXPORT QMediaControlNotifier : public QObject
{
    Q_OBJECT
public:
    explicit QMediaControlNotifier( const QString& control, QObject* parent = 0 );
    ~QMediaControlNotifier();

    QMediaContent* content() const;

signals:
    void valid();
    void invalid();

public slots:
    void setMediaContent( QMediaContent* content );

private slots:
    void evaluate();

private:
    QString m_control;
    QMediaContent *m_content;
    QMediaControlNotifierPrivate *m_d;
};

class QTOPIAMEDIA_EXPORT KeyFilter : public QObject
{
public:
    KeyFilter( QObject* subject, QObject* target, QObject* parent = 0 );

    void addKey( int key );

    bool eventFilter( QObject* o, QEvent* e );

private:
    QObject *m_target;
    QSet<int> m_keys;
};

#endif // QMEDIATOOLS_H

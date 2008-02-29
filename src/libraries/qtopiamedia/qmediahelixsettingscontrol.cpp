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

#include <qabstractipcinterface.h>

#include "qmediahelixsettingscontrol.h"

// {{{ QMediaHelixSettingsControlPrivate
class QMediaHelixSettingsControlPrivate : public QAbstractIpcInterface
{
    Q_OBJECT

public:
    QMediaHelixSettingsControlPrivate(QString const& id):
        QAbstractIpcInterface("/MediaServer", QMediaHelixSettingsControl::name(), id)
    {
    }

    QStringList availableOptions()
    {
        return value("AvailableOptions").toStringList();
    }

    QVariant optionValue(QString const& name)
    {
        return value(name);
    }

public slots:
    void setOption(QString const& name, QVariant const& value)
    {
        invoke(SLOT(setOption(QString, QVariant)), name, value);
    }

signals:
    void optionChanged(QString name, QVariant value);
};
// }}}


/*!
    \class QMediaHelixSettingsControl
    \brief The QMediaHelixSettingsControl class is used to alter settings of the helix engine.

    This class can be used to query and alter Helix DNA Client related options.

    It is only available when the Helix Client is being used as the media
    provider in Qtopia's media system.

    \ingroup multimedia
*/

/*!
    Create a QMediaHelixSettingsControl

    The \a parent is the Parent QObject.
*/

QMediaHelixSettingsControl::QMediaHelixSettingsControl
(
 QObject*               parent
):
    QObject(parent)
{
    d = new QMediaHelixSettingsControlPrivate("HelixGlobalSettings");

    connect(d, SIGNAL(optionChanged(QString, QVariant)),
            this, SIGNAL(optionChanged(QString, QVariant)));
}

/*!
    Destroy a QMediaHelixSettingsControl
*/

QMediaHelixSettingsControl::~QMediaHelixSettingsControl()
{
    delete d;
}

/*!
    Return a list of options supported by the Qtopia Helix integration.
*/

QStringList QMediaHelixSettingsControl::availableOptions()
{
    return d->availableOptions();
}

/*!
    Set a Helix option by name into the Helix Client Engine.

    The \a name parameter is the name of the helix option to set.
    The \a value parameter is the value of the helix option.
*/

void QMediaHelixSettingsControl::setOption(QString const& name, QVariant const& value)
{
    d->setOption(name, value);
}

/*!
    Retrieve a value by name from the Helix Client Engine.

    The \a name parameter is the name of the helix option to fetch.
*/

QVariant QMediaHelixSettingsControl::optionValue(QString const& name)
{
    return d->optionValue(name);
}

/*!
    Return the name of the control.
*/

QString QMediaHelixSettingsControl::name()
{
    return QLatin1String("MediaHelixSettingsControl");
}

/*!
    \fn QMediaHelixSettingsControl::optionChanged(QString name, QVariant value);

    This Signal is emitted when the system has processed the option changed
    request initiated with setOption. The \a name and \a value paramters
    match the values specified with the setOption call.
*/

#include "qmediahelixsettingscontrol.moc"


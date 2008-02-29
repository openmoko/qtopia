/****************************************************************************
**
** Copyright (C) 2000-2007 TROLLTECH ASA. All rights reserved.
**
** This file is part of the Opensource Edition of the Qtopia Toolkit.
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

#ifndef RESOURCESOURCEBROWSER_H
#define RESOURCESOURCEBROWSER_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qtopia API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#include <QList>
#include <QMap>
#include <QWidget>

class QAbstractButton;
class QDSAction;
class QLabel;
class QPushButton;

class ResourceSourceSelectorPrivate;

// A base class for widgets implementing resource selection
class ResourceSourceSelector : public QWidget
{
    Q_OBJECT

public:
    enum ServiceType {
        ContentRequired,
        NoContentRequired,
    };

    enum ArrangementType {
        HorizontalArrangement,
        VerticalArrangement,
    };

    // Describe a selection of services to operate on our selected resource
    struct ServicesDescriptor
    {
        ServicesDescriptor( const QStringList& attr, const QString& req, const QString& resp, ServiceType content )
            : attributes( attr ),
              requestType( req ),
              responseType ( resp ),
              requiresContent ( content == ContentRequired ) {}

        QStringList attributes;
        QString requestType;
        QString responseType;
        bool requiresContent;
    };

    typedef QList<ServicesDescriptor> ServicesList;
    typedef QMap<QString, QString> Dictionary;

    ResourceSourceSelector( QWidget *parent );
    ~ResourceSourceSelector();

    QLabel* label();
    QPushButton* changeButton();
    QPushButton* removeButton();

    void haveResource( bool f );

    void init( ArrangementType arrangement, ServicesList* services = 0 );

    template<typename HandlerType>
    void connectSignals( HandlerType* handler );

signals:
    void serviceRequest( const QString& type, QDSAction& action );

private slots:
    void serviceRequest( QAbstractButton* );

private:
    void connectSignals( QObject* handler );

    ResourceSourceSelectorPrivate *d;
};

template<typename HandlerType>
void ResourceSourceSelector::connectSignals( HandlerType* handler )
{
    // The object handling our signals must supply the following slots:
    void (HandlerType::*changeSlotTest)() = &HandlerType::change;
    Q_UNUSED(changeSlotTest)

    void (HandlerType::*removeSlotTest)() = &HandlerType::remove;
    Q_UNUSED(removeSlotTest)

    void (HandlerType::*serviceRequestSlotTest)(const QString&, QDSAction&) = &HandlerType::serviceRequest;
    Q_UNUSED(serviceRequestSlotTest)

    connectSignals( static_cast<QObject*>( handler ) );
}

#endif // RESOURCESOURCEBROWSER_H

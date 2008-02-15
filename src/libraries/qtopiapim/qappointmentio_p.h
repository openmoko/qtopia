/****************************************************************************
**
** Copyright (C) 2000-2008 TROLLTECH ASA. All rights reserved.
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

#ifndef APPOINTMENTIO_PRIVATE_H
#define APPOINTMENTIO_PRIVATE_H

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

#include <QVector>
#include <QList>
#include <QDateTime>
#include <qtopia/pim/qappointment.h>
#include <qtopia/pim/qappointmentmodel.h>
#include <qcategorymanager.h>

#include "qrecordio_p.h"

class QAppointmentIO : public QRecordIO {

Q_OBJECT

public:
    explicit QAppointmentIO(QObject *parent);
    virtual ~QAppointmentIO();

    virtual void setRangeFilter(const QDateTime &earliest, const QDateTime &latest) = 0;

    virtual void setDurationType(QAppointmentModel::DurationType) = 0;

    virtual QAppointment appointment(const QUniqueId &) const = 0;
    virtual QAppointment appointment(int row) const = 0;

    virtual QAppointment appointment(const QUniqueId &id, bool) const 
    { return appointment(id); }
    virtual QAppointment appointment(int row, bool) const
    { return appointment(row); }

    virtual QVariant appointmentField(int row, QAppointmentModel::Field k) const;
    virtual bool setAppointmentField(int row, QAppointmentModel::Field k,  const QVariant &);

    virtual bool removeAppointment(int row) = 0;
    virtual bool removeAppointment(const QUniqueId & id) = 0;
    virtual bool updateAppointment(const QAppointment &) = 0; // not done by row
    virtual QUniqueId addAppointment(const QAppointment &, const QPimSource &) = 0; // not done by row

    /* event relationships, otherwise known as detatched events and exceptions

    this handles the edit this, edit remaining problem quite robustly.

    Syncing on the other hand, our appointements don't have exceptions, other
db systems do and refer to just one id for a series of split events.



     */

    /* modify only this */
    virtual bool removeOccurrence(const QUniqueId &original,
            const QDate &) = 0;
    virtual QUniqueId replaceOccurrence(const QUniqueId &original,
            const QOccurrence &, const QDate&) = 0;
    /* modify remaining */
    virtual QUniqueId replaceRemaining(const QUniqueId &original,
            const QAppointment &, const QDate&) = 0;
   
    
    virtual bool restoreOccurrence(const QUniqueId &original,
            const QDate &) = 0;

signals:
    void alarmTriggered(const QDateTime &when, const QUniqueId &);

public slots:
    virtual void checkAdded(const QUniqueId &);
    virtual void checkRemoved(const QUniqueId &);
    virtual void checkRemoved(const QList<QUniqueId> &);
    virtual void checkUpdated(const QUniqueId &);

protected:
    void notifyAdded(const QUniqueId &);
    void notifyUpdated(const QUniqueId &);
    void notifyRemoved(const QUniqueId &);
    void notifyRemoved(const QList<QUniqueId> &);

    virtual void ensureDataCurrent(bool force = false);

    // alarm functions
    static QDateTime nextAlarm(const QAppointment &appointment);
    static void removeAlarm(const QAppointment &appointment);
    static void addAlarm(const QAppointment &appointment);

private slots:
    void pimMessage(const QString &message, const QByteArray &data);

private:
    static QList<QAppointmentIO *> activeAppointments;
};

#endif

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

#ifndef MODEMNETWORKREGISTER_H
#define MODEMNETWORKREGISTER_H

#include <qnetworkregistration.h>
#include <qpreferrednetworkoperators.h>
#include <qbandselection.h>

#include <QDialog>
#include <QListWidget>

class QModemNetworkRegistration;
class QWaitWidget;

class ModemNetworkRegister : public QListWidget
{
    Q_OBJECT
public:
    ModemNetworkRegister( QWidget *parent = 0 );
    ~ModemNetworkRegister();

private slots:
    void operationSelected( QListWidgetItem * );
    void selectOperator( const QList<QNetworkRegistration::AvailableOperator> & );
    void band( QBandSelection::BandMode, const QString & );
    void selectBand( const QStringList & );
    void setBandResult( QTelephony::Result );
    void acceptOpDlg();
    void setCurrentOperatorResult( QTelephony::Result result );

private:
    QNetworkRegistration *m_client;
    QBandSelection *m_bandSel;
    QString m_curBand;
    QBandSelection::BandMode m_curBandMode;
    QWaitWidget *m_waitWidget;
    QDialog *m_opDlg;
    QListWidget *m_opList;
    QListWidgetItem *m_originalOp;
    QList<QNetworkRegistration::AvailableOperator> m_result;

    void init();
    void selectSearchMode();
    void preferredOperators();
    void showCurrentOperator();
};

class PreferredOperatorsDialog : public QDialog
{
    Q_OBJECT
public:
    PreferredOperatorsDialog( QWidget *parent = 0, Qt::WFlags fl = 0 );
    ~PreferredOperatorsDialog();

protected:
    void accept();
    void showEvent( QShowEvent * );

private slots:
    void operatorNames( const QList<QPreferredNetworkOperators::NameInfo> & );
    void preferredOperators( QPreferredNetworkOperators::List,
            const QList<QPreferredNetworkOperators::Info> & );
    void addNetwork();
    void networkSelected( QListWidgetItem * );
    void removeNetwork();
    void moveUp();
    void moveDown();
    void rowChanged( int );
    void requestOperatorInfo();

private:
    void init();
    void populateList();
    void swap( int , int );
    void updateIndex( int, bool );
    void checkIndex();
    bool isPreferred( unsigned int );

    QListWidget *m_list;
    QPreferredNetworkOperators *m_PNOClient;
    QList<QPreferredNetworkOperators::NameInfo> m_operatorNames;
    QList<QPreferredNetworkOperators::Info> m_currentOpers;
    QList<QPreferredNetworkOperators::Info> m_originalOpers;
    QWaitWidget *m_waitWidget;
    QAction *m_add, *m_remove, *m_up, *m_down;
};

#endif /* MODEMNETWORKREGISTER_H */

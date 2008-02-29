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

#include "modemnetwork.h"

#include <qtopiaapplication.h>
#include <qwaitwidget.h>
#include <qsoftmenubar.h>
#include <QVBoxLayout>
#include <QRadioButton>
#include <QButtonGroup>
#include <QMenu>
#include <QLabel>
#include <QSpinBox>
#include <QTimer>
#include <QTranslatableSettings>
#include <QtopiaItemDelegate>
#include <QScrollArea>


class PhoneNetworksListWidget : public QListWidget
{
    Q_OBJECT
public:
    PhoneNetworksListWidget(QWidget *parent = 0);
    QSize minimumSizeHint() const;
};

PhoneNetworksListWidget::PhoneNetworksListWidget( QWidget *parent )
    : QListWidget( parent )
{
}

QSize PhoneNetworksListWidget::minimumSizeHint() const
{
    QSize sz = QListWidget::minimumSizeHint();
    if ( count() > 0 ) {
        int h = sizeHintForRow( 0 );
        return QSize( sz.width(), h * 5 );
    }
    return sz;
}



ModemNetworkRegister::ModemNetworkRegister( QWidget *parent )
    : QListWidget( parent )
{
    init();

    connect( this, SIGNAL(itemActivated(QListWidgetItem*)),
            this, SLOT(operationSelected(QListWidgetItem*)) );
    if ( m_client ) {
        connect( m_client, SIGNAL(availableOperators(QList<QNetworkRegistration::AvailableOperator>)),
                this, SLOT(selectOperator(QList<QNetworkRegistration::AvailableOperator>)) );
        connect( m_client, SIGNAL(setCurrentOperatorResult(QTelephony::Result)),
                 this, SLOT(setCurrentOperatorResult(QTelephony::Result)) );
    }
    if ( m_bandSel ) {
        connect( m_bandSel, SIGNAL(band(QBandSelection::BandMode,QString)),
                this, SLOT(band(QBandSelection::BandMode,QString)) );
        connect( m_bandSel, SIGNAL(bands(QStringList)),
                this, SLOT(selectBand(QStringList)) );
        connect( m_bandSel, SIGNAL(setBandResult(QTelephony::Result)),
                this, SLOT(setBandResult(QTelephony::Result)) );
        m_bandSel->requestBand();
    }
}

ModemNetworkRegister::~ModemNetworkRegister()
{
}

void ModemNetworkRegister::init()
{
    setItemDelegate( new QtopiaItemDelegate );
    setFrameStyle( QFrame::NoFrame );

    m_client = new QNetworkRegistration( "modem", this );
    m_bandSel = new QBandSelection( "modem", this );

    setWindowTitle( tr( "Call Networks" ) );
    setObjectName( "modem" );

    (void) new QListWidgetItem( tr( "Current status" ), this );
    (void) new QListWidgetItem( tr( "Search mode" ), this );
    (void) new QListWidgetItem( tr( "Select operator" ), this );
    (void) new QListWidgetItem( tr( "Preferred networks" ), this );
    if ( m_bandSel->available() )
        (void) new QListWidgetItem( tr( "Select band" ), this );

    setCurrentRow( 0 );

    m_waitWidget = new QWaitWidget( this );
    m_waitWidget->setCancelEnabled( true );
}

void ModemNetworkRegister::operationSelected( QListWidgetItem * )
{
    switch ( currentRow() ) {
    case 0:
        showCurrentOperator();
        break;
    case 1:
        selectSearchMode();
        break;
    case 2:
        if ( !m_waitWidget->isVisible() ) {
            m_client->requestAvailableOperators();
            m_waitWidget->show();
        }
        break;
    case 3:
        preferredOperators();
        break;
    case 4:
        if ( !m_waitWidget->isVisible() ) {
            m_bandSel->requestBands();
            m_waitWidget->show();
        }
        break;
    default:
        break;
    }
}

static QString countryForOperatorId( const QString& id )
{
    if ( !id.startsWith( QChar('2') ) )
        return QString();
    QTranslatableSettings settings( "Trolltech", "GsmOperatorCountry" );
    settings.beginGroup( "Countries" );
    return settings.value( id.mid(1,3) ).toString();
}

void ModemNetworkRegister::showCurrentOperator()
{
    QDialog dlg( this );
    dlg.setWindowTitle( tr( "Current Network" ) );
    QVBoxLayout layout( &dlg );

    // operator
    QLabel lbl( "<b>" + tr( "Operator:" ) + "</b>", &dlg );
    QLabel name( m_client->currentOperatorName(), &dlg );
    // country
    QLabel lblc( "<b>" + tr( "Country:" ) + "</b>", &dlg );
    QLabel country
        ( countryForOperatorId( m_client->currentOperatorId() ), &dlg );
    // technology
    QLabel lbl1( "<b>" + tr( "Technology:" ) + "</b>", &dlg );
    QString state = m_client->currentOperatorTechnology().isEmpty() ?
        tr( "Unknown" ) : m_client->currentOperatorTechnology();
    QLabel technology( state, &dlg );
    // registration
    QLabel lbl2( "<b>" + tr( "Registration state:" ) + "</b>", &dlg );
    switch ( m_client->registrationState() ) {
    case QTelephony::RegistrationNone:
        state = tr( "Not registered" );
        break;
    case QTelephony::RegistrationHome:
        state = tr( "Registered to home network" );
        break;
    case QTelephony::RegistrationSearching:
        state = tr( "Searching" );
        break;
    case QTelephony::RegistrationDenied:
        state = tr( "Registration denied" );
        break;
    case QTelephony::RegistrationUnknown:
        state = tr( "Registered but unknown state" );
        break;
    case QTelephony::RegistrationRoaming:
        state = tr( "Roaming" );
        break;
    default:
        state = tr( "Unknown" );
        break;
    }
    QLabel regState( state, &dlg );
    regState.setWordWrap( true );
    // mode
    QLabel lbl3( "<b>" + tr( "Mode:" ) + "</b>", &dlg );
    switch( m_client->currentOperatorMode() ) {
    case QTelephony::OperatorModeAutomatic:
        state = tr( "Automatic" );
        break;
    case QTelephony::OperatorModeManual:
        state = tr( "Manual" );
        break;
    case QTelephony::OperatorModeManualAutomatic:
        state = tr( "Manual/Automatic");
        break;
    default:
        state = tr( "Other state" );
        break;
    }
    QLabel mode( state, &dlg );
    layout.addWidget( &lbl );
    layout.addWidget( &name );
    layout.addWidget( &lblc );
    layout.addWidget( &country );
    layout.addWidget( &lbl1 );
    layout.addWidget( &technology );
    layout.addWidget( &lbl2 );
    layout.addWidget( &regState );
    layout.addWidget( &lbl3 );
    layout.addWidget( &mode );
    // band
    if ( m_bandSel->available() ) {
        QLabel lbl4( "<b>" + tr( "Band:" ) + "</b>", &dlg );
        state = m_curBandMode == QBandSelection::Automatic ? tr( "Automatic" ) : tr( "Manual/" ) + m_curBand;
        QLabel band( state, &dlg );
        layout.addWidget( &lbl4 );
        layout.addWidget( &band );
    }
    layout.addStretch();

    QtopiaApplication::execDialog( &dlg );
}

void ModemNetworkRegister::band( QBandSelection::BandMode mode, const QString &value )
{
    m_curBandMode = mode;
    m_curBand = value;
}

void ModemNetworkRegister::selectBand( const QStringList& bandList )
{
    if ( m_waitWidget->isVisible() )
        m_waitWidget->hide();
    else // assume operation is cancelled by the user by closing wait widget.
        return;

    QDialog *dlg = new QDialog( this );
    dlg->setWindowTitle( tr( "Select band" ) );
    QVBoxLayout *layout = new QVBoxLayout( dlg );
    QButtonGroup *group = new QButtonGroup( dlg );
    int id = 0;

    // automatic
    QRadioButton *autoBand = new QRadioButton( tr( "Automatic" ), dlg );
    if ( m_curBandMode == QBandSelection::Automatic )
        autoBand->setChecked( true );
    group->addButton( autoBand, id++ );
    layout->addWidget( autoBand );

    // available bands
    QRadioButton *btn;
    foreach ( QString band, bandList ) {
        btn = new QRadioButton( band, dlg );
        if ( m_curBandMode == QBandSelection::Manual && band == m_curBand )
            btn->setChecked( true );
        group->addButton( btn, id++ );
        layout->addWidget( btn );
    }
    layout->addStretch();

    id = group->checkedId();
    if ( id < 0 ) { // if nothing is checked, check and activate Auto option
        id = group->id( autoBand );
        autoBand->setChecked( true );
        m_bandSel->setBand( QBandSelection::Automatic, QString() );
    }

    if ( QtopiaApplication::execDialog( dlg ) ) {
        if ( id == group->checkedId() ) // no change
            return;

        if ( autoBand->isChecked() )
            m_bandSel->setBand( QBandSelection::Automatic, QString() );
        else
            m_bandSel->setBand( QBandSelection::Manual, group->checkedButton()->text() );
    }
}

void ModemNetworkRegister::setBandResult( QTelephony::Result result )
{
    if ( result == QTelephony::OK )
        m_bandSel->requestBand();
}

void ModemNetworkRegister::selectSearchMode()
{
    QDialog *dlg = new QDialog( this );
    dlg->setWindowTitle( tr( "Search Mode" ) );
    QVBoxLayout *layout = new QVBoxLayout( dlg );
    QButtonGroup *group = new QButtonGroup( dlg );
    QRadioButton *automatic = new QRadioButton( tr( "Automatic" ), dlg );
    group->addButton( automatic, (int)QTelephony::OperatorModeAutomatic );
    layout->addWidget( automatic );
    QRadioButton *manual = new QRadioButton( tr( "Manual" ), dlg );
    group->addButton( manual, (int)QTelephony::OperatorModeManual );
    layout->addWidget( manual );

    QTelephony::OperatorMode originalMode = m_client->currentOperatorMode();
    if ( originalMode == QTelephony::OperatorModeAutomatic )
        automatic->setChecked( true );
    else
        manual->setChecked( true );

    if ( QtopiaApplication::execDialog( dlg ) ) {
        if ( (int)originalMode == group->checkedId() )
            return;
        if ( automatic->isChecked() )
            m_client->setCurrentOperator( QTelephony::OperatorModeAutomatic );
        else
            m_client->setCurrentOperator( QTelephony::OperatorModeManual,
                    m_client->currentOperatorId(), m_client->currentOperatorTechnology() );
    }
}

void ModemNetworkRegister::selectOperator( const QList<QNetworkRegistration::AvailableOperator> &result )
{
    if ( m_waitWidget->isVisible() )
        m_waitWidget->hide();
    else // assume operation is cancelled by the user by closing wait widget.
        return;

    if ( result.count() == 0 ) {
        if ( QMessageBox::question( this, tr( "No operator found" ),
                tr( "<qt>Would you like to search again?</qt>" ),
                QMessageBox::Yes, QMessageBox::No ) == QMessageBox::Yes ) {
                m_waitWidget->show();
                m_client->requestAvailableOperators();
        }
        return;
    }

    m_result = result;

    m_opDlg = new QDialog( this );
    m_opDlg->setWindowTitle( tr( "Available operators" ) );
    QVBoxLayout *layout = new QVBoxLayout( m_opDlg );
    m_opList = new PhoneNetworksListWidget( m_opDlg );
    m_opList->setItemDelegate( new QtopiaItemDelegate );
    m_opList->setFrameStyle( QFrame::NoFrame );
    int iconMetric = QApplication::style()->pixelMetric( QStyle::PM_SmallIconSize );
    m_opList->setIconSize( QSize( iconMetric, iconMetric ) );
    m_opList->setUniformItemSizes( true );
    m_opList->setAlternatingRowColors( true );
    layout->addWidget( m_opList );
    QtopiaApplication::setMenuLike( m_opDlg, true );

    QListWidgetItem *item = 0;
    QString utran = tr("3G", "3g/umts/utran network");
    QString gsm = tr("GSM", "GSM network");
    foreach( QNetworkRegistration::AvailableOperator op, result ) {
        QString name = op.name;
        if ( op.technology == "GSM" ) //no tr
            name = name + " (" + gsm  +")" ;
        else if ( op.technology == "UTRAN" )
            name = name + " (" + utran  +")" ;
        QString country = countryForOperatorId( op.id );
        if ( !country.isEmpty() )
            name = name + "\n  " + country;
        item = new QListWidgetItem( name, m_opList );
        switch ( op.availability ) {
        case QTelephony::OperatorUnavailable:
            item->setIcon( QIcon( ":icon/close" ) );
            break;
        case QTelephony::OperatorAvailable:
            item->setIcon( QIcon( ":icon/globe" ) );
            break;
        case QTelephony::OperatorCurrent:
            item->setSelected( true );
            item->setIcon( QIcon( ":icon/done" ) );
            m_originalOp = item;
            break;
        case QTelephony::OperatorForbidden:
            item->setIcon( QIcon( ":icon/uninstall" ) );
            break;
        default:
            break;
        }
    }

    connect( m_opList, SIGNAL(itemActivated(QListWidgetItem*)), this, SLOT(acceptOpDlg()) );

    QtopiaApplication::execDialog( m_opDlg );
}

void ModemNetworkRegister::acceptOpDlg()
{
    if ( m_originalOp == m_opList->currentItem() ) {
        m_opDlg->accept();
        return;
    }

    int i = m_opList->currentRow();

    if ( m_result.at( i ).availability == QTelephony::OperatorForbidden ) {
        QMessageBox::warning( this, tr( "Forbidden" ),
                "<qt>" + tr( "You cannot use a forbidden operator. "
                    "Please select an available operator." ) + "</qt>" );
        return;
    }

    // cannot do manual registraion in automatic mode
    // ask if the user wants the mode to be changed.
    if ( m_client->currentOperatorMode() == QTelephony::OperatorModeAutomatic ) {
        if ( QMessageBox::question( this, tr( "Automatic Mode" ),
                tr( "<qt>Would you wish to change search mode to Manual"
                    " and register to the selected operator?</qt>" ),
                QMessageBox::Yes, QMessageBox::No ) == QMessageBox::Yes ) {
            m_client->setCurrentOperator( QTelephony::OperatorModeManual,
                m_result.at( i ).id,
                m_result.at( i ).technology );
        }
    } else { // manual mode
        m_client->setCurrentOperator( m_client->currentOperatorMode(),
                m_result.at( i ).id,
                m_result.at( i ).technology );
    }

    m_opDlg->accept();
}

void ModemNetworkRegister::setCurrentOperatorResult( QTelephony::Result result )
{
    if ( result != QTelephony::OK ) {
        QMessageBox::warning( this, tr( "Select operator" ),
                "<qt>" + tr( "Failed to register to the selected network operator." ) +
                "</qt>" );
    }
}

void ModemNetworkRegister::preferredOperators()
{
    PreferredOperatorsDialog dlg( this );
    QtopiaApplication::execDialog( &dlg );
}

//-----------------------------------------------------
PreferredOperatorsDialog::PreferredOperatorsDialog( QWidget *parent, Qt::WFlags fl )
    : QDialog( parent, fl )
{
    init();

    if ( m_PNOClient ) {
        connect( m_PNOClient, SIGNAL(operatorNames(QList<QPreferredNetworkOperators::NameInfo>)),
                this, SLOT(operatorNames(QList<QPreferredNetworkOperators::NameInfo>)) );
        connect( m_PNOClient, SIGNAL(preferredOperators(QPreferredNetworkOperators::List,
                        const QList<QPreferredNetworkOperators::Info>&)),
                this, SLOT(preferredOperators(QPreferredNetworkOperators::List,
                        const QList<QPreferredNetworkOperators::Info>&)) );
    }
    connect( m_list, SIGNAL(currentRowChanged(int)),
            this, SLOT(rowChanged(int)) );
}

PreferredOperatorsDialog::~PreferredOperatorsDialog()
{
}

void PreferredOperatorsDialog::showEvent( QShowEvent *e )
{
    QTimer::singleShot( 0, this, SLOT(requestOperatorInfo()) );
    QDialog::showEvent( e );
}

void PreferredOperatorsDialog::requestOperatorInfo()
{
    m_PNOClient->requestOperatorNames();
    m_PNOClient->requestPreferredOperators( QPreferredNetworkOperators::Current );
    m_waitWidget->show();
}

void PreferredOperatorsDialog::init()
{
    setWindowTitle( tr( "Preferred Networks" ) );
    QVBoxLayout *layout = new QVBoxLayout( this );
    m_list = new QListWidget( this );
    layout->addWidget( m_list );

    QMenu *contextMenu = QSoftMenuBar::menuFor( this );
    m_add = contextMenu->addAction( QIcon( ":icon/new" ), tr( "Add..." ), this, SLOT(addNetwork()) );
    m_remove = contextMenu->addAction( QIcon( ":icon/trash" ), tr( "Remove" ), this, SLOT(removeNetwork()) );
    m_up = contextMenu->addAction( QIcon( ":icon/up" ), tr( "Move up" ), this, SLOT(moveUp()) );
    m_down = contextMenu->addAction( QIcon( ":icon/down" ), tr( "Move down" ), this, SLOT(moveDown()) );
    QSoftMenuBar::setLabel( this, Qt::Key_Select, QSoftMenuBar::NoLabel );

    m_waitWidget = new QWaitWidget( this );
    m_waitWidget->setCancelEnabled( true );
    connect( m_waitWidget, SIGNAL(cancelled()), this, SLOT(reject()) );
    m_PNOClient = new QPreferredNetworkOperators( "modem", this );
}

void PreferredOperatorsDialog::operatorNames( const QList<QPreferredNetworkOperators::NameInfo> &names )
{
    m_operatorNames = names;
    if ( m_currentOpers.count() > 0 ) {
        m_waitWidget->hide();
        populateList();
    }
}

void PreferredOperatorsDialog::preferredOperators( QPreferredNetworkOperators::List,
        const QList<QPreferredNetworkOperators::Info> &names )
{
    m_currentOpers = names; // will contain latest user selection
    m_originalOpers = names; // will be used to compare changes when dialog accepted
    if ( m_operatorNames.count() > 0 ) {
        m_waitWidget->hide();
        populateList();
    }
}

void PreferredOperatorsDialog::populateList()
{
    QList<QPreferredNetworkOperators::Info> resolved =
        m_PNOClient->resolveNames( m_currentOpers, m_operatorNames );
    for ( int i = 0; i < resolved.count(); i++ ) {
        QString name = resolved.at( i ).name;
        QString country = countryForOperatorId
            ( "2" + QString::number( resolved.at( i ).id ) );
        if ( !country.isEmpty() )
            name = name + "\n  " + country;
        (void) new QListWidgetItem( name, m_list );
    }
    if ( resolved.count() > 0 )
        m_list->setCurrentRow( 0 );
}

void PreferredOperatorsDialog::addNetwork()
{
    QDialog dlg( this );
    dlg.setModal( true );
    dlg.setWindowTitle( tr( "Select Network" ) );
    QVBoxLayout layout( &dlg );
    QListWidget list( &dlg );
    layout.addWidget( &list );

    for ( int i = 0; i < m_operatorNames.count(); i++ ) {
        // filter out network that are alrealy in the preferred list
        if ( isPreferred( m_operatorNames.at( i ).id ) )
            continue;

        // display id number of the networks with the same name
        QString name = m_operatorNames.at( i ).name;
        if ( ( i > 0 && m_operatorNames.at( i - 1 ).name == name )
          || i != m_operatorNames.count() - 1 && m_operatorNames.at( i + 1 ).name == name )
            name = name + "(" + QString::number( m_operatorNames.at( i ).id ) + ")";
        QString country = countryForOperatorId
            ( "2" + QString::number( m_operatorNames.at( i ).id ) );
        if ( !country.isEmpty() )
            name = name + "\n  " + country;
        (void) new QListWidgetItem( name, &list, m_operatorNames.at( i ).id );
    }

    connect( &list, SIGNAL(itemActivated(QListWidgetItem*)),
            this, SLOT(networkSelected(QListWidgetItem*)) );

    connect( &list, SIGNAL(itemActivated(QListWidgetItem*)),
            &dlg, SLOT(accept()) );

    if ( m_operatorNames.count() > 0 )
        list.setCurrentRow( 0 );

    QtopiaApplication::setMenuLike( &dlg, true );

    QtopiaApplication::execDialog( &dlg );
}

bool PreferredOperatorsDialog::isPreferred( unsigned int id )
{
    for ( int i = 0; i < m_currentOpers.count(); i++ ) {
        if ( m_currentOpers.at( i ).id == id )
            return true;
    }
    return false;
}

void PreferredOperatorsDialog::networkSelected( QListWidgetItem *item )
{
    // get preferred position
    QDialog dlg( this );
    dlg.setWindowTitle( tr( "Select position" ) );
    QVBoxLayout layout( &dlg );
    layout.setSpacing( 2 );
    layout.setMargin( 2 );
    QString networkName = item->text();
    networkName.replace("\n", "");
    QLabel label( &dlg );
    label.setTextFormat(Qt::RichText);
    label.setText( tr( "Add %1 to preferred network list.<br><br>Please select a position between 1 and %2.",
                "%1 = name of network, %2 = number of networks" )
            .arg( networkName ).arg( m_currentOpers.count() + 1 ) );
    label.setWordWrap( true );
    QSpinBox spinBox( &dlg );
    spinBox.setMinimum( 1 );
    spinBox.setMaximum( m_currentOpers.count() + 1 );
    spinBox.setValue( m_currentOpers.count() + 1 );
    layout.addWidget( &label );
    layout.addWidget( &spinBox );

    QPreferredNetworkOperators::Info oper;

    if ( m_currentOpers.count() == 0 ) {
        oper.index = 1;
    } else {
        if ( QtopiaApplication::execDialog( &dlg ) )
            oper.index = spinBox.value();
        else
            return;
    }

    oper.format = 2;
    oper.id = item->type();
    oper.name = item->text();

    // add new item to current list
    m_currentOpers.insert( oper.index - 1, oper );

    // add 1 to index of items below this new one
    for ( int i = oper.index; i < m_currentOpers.count(); i++ )
        updateIndex( i, true );

    QListWidgetItem *newItem = new QListWidgetItem( oper.name );
    m_list->insertItem( oper.index - 1, newItem );
    m_list->setCurrentRow( oper.index - 1 );
}

void PreferredOperatorsDialog::removeNetwork()
{
    int index = m_list->currentRow();

    // update network to delete selected operator
    QPreferredNetworkOperators::Info delItem;
    delItem.index = m_currentOpers.at( index ).index;
    delItem.id = 0;
    delItem.name = QString();
    delItem.format = 0;

    m_currentOpers.removeAt( index );

    // substract 1 from index of items below this new one
    for ( int i = index; i < m_currentOpers.count(); i++ )
        updateIndex( i, false );

    delete m_list->takeItem( m_list->currentRow() );
    // ensure 'Remove' menu to disappear when item count is down to one.
    rowChanged( m_list->currentRow() );
}

void PreferredOperatorsDialog::updateIndex( int pos, bool increase )
{
    int i = increase ? 1 : -1;
    QPreferredNetworkOperators::Info newItem;
    newItem.index = m_currentOpers.at( pos ).index + i;
    newItem.format = m_currentOpers.at( pos ).format;
    newItem.id = m_currentOpers.at( pos ).id;
    newItem.name = m_currentOpers.at( pos ).name;
    newItem.technologies = m_currentOpers.at( pos ).technologies;

    m_currentOpers.removeAt( pos );
    m_currentOpers.insert( pos, newItem );
}

void PreferredOperatorsDialog::moveUp()
{
    int i = m_list->currentRow();
    if ( i == 0 )
        return;

    swap( i - 1, i );

    QListWidgetItem *item = m_list->takeItem( i );
    m_list->insertItem( i - 1, item );
    m_list->setCurrentRow( i - 1 );
}

void PreferredOperatorsDialog::moveDown()
{
    int i = m_list->currentRow();
    if ( i == m_list->count() - 1 )
        return;

    swap( i, i + 1 );

    QListWidgetItem *item = m_list->takeItem( i );
    m_list->insertItem( i + 1, item );
    m_list->setCurrentRow( i + 1 );
}

void PreferredOperatorsDialog::swap( int first, int second )
{
    // swap information except index
    QPreferredNetworkOperators::Info oper1 = m_currentOpers.takeAt( first );
    QPreferredNetworkOperators::Info oper2 = m_currentOpers.takeAt( first );
    QPreferredNetworkOperators::Info operNew1;
    QPreferredNetworkOperators::Info operNew2;

    operNew1.index = oper2.index;
    operNew1.format = oper1.format;
    operNew1.id = oper1.id;
    operNew1.name = oper1.name;
    operNew1.technologies = oper1.technologies;

    operNew2.index = oper1.index;
    operNew2.format = oper2.format;
    operNew2.id = oper2.id;
    operNew2.name = oper2.name;
    operNew2.technologies = oper2.technologies;

    m_currentOpers.insert( first, operNew2 );
    m_currentOpers.insert( second, operNew1 );
}

void PreferredOperatorsDialog::rowChanged( int i )
{
    m_remove->setVisible( m_list->count() > 1 );
    m_remove->setEnabled( m_list->count() > 1 );
    m_up->setVisible( i != 0 );
    m_up->setEnabled( i != 0 );
    m_down->setVisible( i != m_list->count() - 1 );
    m_down->setEnabled( i != m_list->count() - 1 );
}

void PreferredOperatorsDialog::checkIndex()
{
    // index should be in order
    for ( int i = 1; i < m_currentOpers.count(); i++ ) {
        if ( m_currentOpers.at( i ).index != m_currentOpers.at( i - 1 ).index + 1 ) {
            QPreferredNetworkOperators::Info item;
            item.index = m_currentOpers.at( i - 1 ).index + 1;
            item.format = m_currentOpers.at( i ).format;
            item.id = m_currentOpers.at( i ).id;
            item.name = m_currentOpers.at( i ).name;
            item.technologies = m_currentOpers.at( i ).technologies;

            m_currentOpers.removeAt( i );
            m_currentOpers.insert( i , item );
        }
    }
}

void PreferredOperatorsDialog::accept()
{
    QList<QPreferredNetworkOperators::Info> list;
    list = m_PNOClient->resolveNames( m_currentOpers, m_operatorNames );

    // ensure the index is in order
    checkIndex();

    // update networks changed its position.
    for ( int i = 0; i < m_currentOpers.count(); i++ ) {
        if ( i >= m_originalOpers.count() ||
                m_originalOpers.at( i ).id != m_currentOpers.at( i ).id
                || ( m_originalOpers.at( i ).id == m_currentOpers.at( i ).id
                    && m_originalOpers.at( i ).index != m_currentOpers.at( i ).index ) ) {
            m_PNOClient->writePreferredOperator(
                    QPreferredNetworkOperators::Current,
                    m_currentOpers.at( i ) );
        }
    }
    for ( int j = m_currentOpers.count(); j < m_originalOpers.count(); j++ ) {
        // remove the remaining items
        QPreferredNetworkOperators::Info delItem;
        delItem.index = m_originalOpers.at( j ).index;
        delItem.id = 0;
        delItem.name = QString();
        delItem.format = 0;

        m_PNOClient->writePreferredOperator(
                QPreferredNetworkOperators::Current,
                delItem );
    }

    QDialog::accept();
}

#include "modemnetwork.moc"

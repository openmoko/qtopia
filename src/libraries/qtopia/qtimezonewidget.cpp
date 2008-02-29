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

#include <QToolButton>
#include <QFile>
#include <QRegExp>
#include <QLayout>
#include <QSettings>
#include <QDesktopWidget>

#include <stdlib.h>

#include "qtimezone.h"
#include "qworldmap.h"
#include "qtimezonewidget.h"
#include "qtopiaapplication.h"
#include <qsoftmenubar.h>

#include <qtopiachannel.h>

// ============================================================================
//
// QTimeZoneWidgetPrivate
//
// ============================================================================

/*!
  \class QTimeZoneWidget
  \mainclass

  \brief The QTimeZoneWidget widget allows configuration of time zone information.

  \ingroup time
*/

class QTimeZoneWidgetPrivate : public QObject
{
    Q_OBJECT
public:
    QTimeZoneWidgetPrivate() : QObject(0), includeLocal(false) {}
    bool includeLocal;
};


// ============================================================================
//
// QTimeZoneComboBox
//
// ============================================================================

QTimeZoneComboBox::QTimeZoneComboBox( QWidget *p )
:   QComboBox( p ),
    prevIndex1( 0 ),
    prevIndex2( 0 )
{
    updateZones();
    // check to see if TZ is set, if it is set the current item to that
    QString tz = getenv("TZ");
    if (parent()->inherits("QTimeZoneWidget")) {
        if ( ((QTimeZoneWidget *)parent())->localIncluded() ) {
            // overide to the 'local' type.
            tz = "None"; // No tr
        }
    }
    if ( !tz.isNull() ) {
        int n = 0,
            index = 0;
        for ( QStringList::Iterator it=identifiers.begin();
              it!=identifiers.end(); ++it) {
            if ( *it == tz )
                index = n;
            n++;
        }
        prevIndex1 = prevIndex2 = index;
        setCurrentIndex(index);
    } else {
        setCurrentIndex(0);
    }

    connect( qobject_cast<QComboBox*>( this ),
             SIGNAL( currentIndexChanged( int ) ),
             this,
             SLOT( indexChange( int ) ) );

    // listen on QPE/System
    QtopiaChannel *channel = new QtopiaChannel( "QPE/System", this );
    connect( channel, SIGNAL(received(const QString&,const QByteArray&)),
        this, SLOT(handleSystemChannel(const QString&,const QByteArray&)) );
}

QTimeZoneComboBox::~QTimeZoneComboBox()
{
}

void QTimeZoneComboBox::updateZones()
{
    QString cur = currentText();
    clear();
    identifiers.clear();
    int curix=0;
    QString tz = getenv("TZ");
    bool tzFound = false; // found the current timezone.
    QSettings cfg("Trolltech","WorldTime");
    cfg.beginGroup("TimeZones");
    int listIndex = 0;
    if (parent()->inherits("QTimeZoneWidget")) {
        if ( ((QTimeZoneWidget *)parent())->localIncluded() ) {
            // overide to the 'local' type.
            identifiers.append( "None" ); // No tr
            addItem( tr("None") );
            if ( cur == tr("None"))
                curix = 0;
            listIndex++;
        }
    }
    int cfgIndex = 0;
    while (1) {
        QString zn = cfg.value("Zone"+QString::number(cfgIndex), QString()).toString();
        if ( zn.isNull() )
            break;
        if ( zn == tz )
            tzFound = true;

        QString nm = QTimeZone( zn.toLatin1() ).city();
        identifiers.append(zn);
        addItem(nm);
        if ( nm == cur )
            curix = listIndex;
        ++cfgIndex;
        ++listIndex;
    }
    for (QStringList::Iterator it=extras.begin(); it!=extras.end(); ++it) {
        QTimeZone z( (*it).toLatin1() );
        addItem(z.city());
        identifiers.append(*it);
        if ( *it == cur )
            curix = listIndex;
        ++listIndex;
    }
    if ( !tzFound && !tz.isEmpty()) {
        int i =  tz.indexOf( '/' );
        QString nm = tz.mid( i+1 ).replace(QRegExp("_"), " ");
        identifiers.append(tz);
#ifndef QT_NO_TRANSLATION
        if (qApp)
            nm = qApp->translate("QTimeZone", nm.toAscii().constData());
#endif
        addItem(nm);
        if ( nm == cur )
            curix = listIndex;
        ++listIndex;
    }
    addItem(tr("More..."));

    setCurrentIndex(curix);
    emit activated(curix);
}


void QTimeZoneComboBox::keyPressEvent( QKeyEvent *e )
{
    // ### should popup() in Qt 3.0 (it's virtual there)
//    updateZones();
    QComboBox::keyPressEvent(e);
}

void QTimeZoneComboBox::mousePressEvent(QMouseEvent*e)
{
    // ### should popup() in Qt 3.0 (it's virtual there)
//    updateZones();
    QComboBox::mousePressEvent(e);
}

void QTimeZoneComboBox::indexChange( const int index )
{
    prevIndex2 = prevIndex1;
    if ( index < 0 )
        prevIndex1 = 0;
    else
        prevIndex1 = index;
}

void QTimeZoneComboBox::setToPreviousIndex()
{
    setCurrentIndex( prevIndex2 );
}

QString QTimeZoneComboBox::prevZone() const
{
    if (identifiers.count())
        return identifiers[prevIndex2];
    return QString();
}

QString QTimeZoneComboBox::currZone() const
{
    if (identifiers.count())
        return identifiers[currentIndex()];
    return QString();
}

void QTimeZoneComboBox::setCurrZone( const QString& id )
{
    for (int i=0; i<identifiers.count(); i++) {
        if ( identifiers[i] == id ) {
            setCurrentIndex(i);
            return;
        }
    }

    QString name = QTimeZone(id.toLatin1()).city();
    int index = count() - 1;
    if ( index > 0 ) {
        insertItem( index, name );
        setCurrentIndex( index );
        identifiers.append(id);
        extras.append(id);
        emit activated( index );
    }
}

void QTimeZoneComboBox::handleSystemChannel(const QString&msg, const QByteArray&)
{
    if ( msg == "timeZoneListChange()" ) {
        updateZones();
    }
}

// ============================================================================
//
// QWorldmapDialog
//
// ============================================================================

class QWorldmapDialog : public QDialog
{
    Q_OBJECT;

public:
    QWorldmapDialog( QWidget* parent = 0, Qt::WFlags f = 0 );

    void setZone( const QTimeZone& zone );
    QTimeZone selectedZone() const;

public slots:
    int exec();
    void selected( const QTimeZone& zone );
    void cancelled();

private:
    QWorldmap* mMap;
    QTimeZone mZone;
};

/*!
    \internal
*/
QWorldmapDialog::QWorldmapDialog( QWidget* parent, Qt::WFlags f )
:   QDialog( parent, f | Qt::FramelessWindowHint ),
    mMap( 0 ),
    mZone()
{
    setWindowTitle( tr( "Select Time Zone" ) );

    QVBoxLayout *bl = new QVBoxLayout(this);
    mMap = new QWorldmap(this);
    QSizePolicy sp = mMap->sizePolicy();
    sp.setHeightForWidth(true);
    mMap->setSizePolicy(sp);
    bl->addWidget( mMap );
    bl->setSpacing( 4 );
    bl->setMargin( 4 );

    setMinimumWidth( qApp->desktop()->width() );
    setMaximumHeight( qApp->desktop()->width() / 2 );

    QSoftMenuBar::setLabel( this, Qt::Key_Back, QSoftMenuBar::Cancel );

    connect( mMap,
             SIGNAL( newZone( const QTimeZone& ) ),
             this,
             SLOT( selected( const QTimeZone& ) ) );

    connect( mMap,
             SIGNAL( selectZoneCanceled() ),
             this,
             SLOT( cancelled() ) );

    mMap->setFocus();
}

/*!
    \internal
*/
int QWorldmapDialog::exec()
{
    if ( isHidden() )
        show();

    if ( mZone.isValid() )
        mMap->setZone( mZone );

    mMap->selectNewZone();
    return QDialog::exec();
}

/*!
    \internal
*/
void QWorldmapDialog::setZone( const QTimeZone& zone )
{
    mZone = zone;
}

/*!
    \internal
*/
void QWorldmapDialog::selected( const QTimeZone& zone )
{
    if ( zone.isValid() ) {
        mZone = zone;
        accept();
    } else {
        reject();
    }
}

/*!
    \internal
*/
void QWorldmapDialog::cancelled()
{
    reject();
}

/*!
    \internal
*/
QTimeZone QWorldmapDialog::selectedZone() const
{
    return mZone;
}

// ============================================================================
//
// QTimeZoneWidget
//
// ============================================================================

/*!
    Creates a new QTimeZoneWidget with parent \a p.  The combobox will be
    populated with the available timezones.
*/

QTimeZoneWidget::QTimeZoneWidget(QWidget* p) :
    QWidget(p)
{
    QHBoxLayout *hbl = new QHBoxLayout;
#ifndef QT_NO_TRANSLATION
    static int transLoaded = 0;
    if (!transLoaded) {
        QtopiaApplication::loadTranslations("timezone");
        transLoaded++;
    }
#endif
    d = new QTimeZoneWidgetPrivate();

    // build the combobox before we do any updates...
    cmbTz = new QTimeZoneComboBox( this );
    cmbTz->setObjectName( "timezone combo" );
    hbl->addWidget(cmbTz);

    // set up a connection to catch a newly selected item and throw our
    // signal
    QObject::connect( cmbTz, SIGNAL( activated(int) ),
                      this, SLOT( slotTzActive(int) ) );
    setLayout(hbl);
}

/*!
  Destroys a QTimeZoneWidget.
*/

QTimeZoneWidget::~QTimeZoneWidget()
{
    delete d;
}

/*!
  If \a b is true, allow no timezone as an option.  If \a b is false then
  a specific timezone must be selected.
*/
void QTimeZoneWidget::setLocalIncluded(bool b)
{
    d->includeLocal = b;
    cmbTz->updateZones();
}

/*!
  Returns true if no timezone is included as an option; otherwise false.
*/
bool QTimeZoneWidget::localIncluded() const
{
    return d->includeLocal;
}

/*!
  Returns the currently selected timezone as a string in location format, for example,
  \code Australia/Brisbane \endcode
*/

QString QTimeZoneWidget::currentZone() const
{
    return cmbTz->currZone();
}

/*!
  Sets the current timezone to \a id.
*/

void QTimeZoneWidget::setCurrentZone( const QString& id )
{
    cmbTz->setCurrZone( id );
}

/*! \fn void QTimeZoneWidget::signalNewTz( const QString& id )
  This signal is emitted when a qtimezone has been selected.
  The \a id
  is a \l QString in location format, for example, \code Australia/Brisbane \endcode
*/

void QTimeZoneWidget::slotTzActive( int idx )
{
    if (idx == cmbTz->count()-1) {
        slotExecute();
    } else {
        emit signalNewTz( cmbTz->currZone() );
    }
}

/*!
  \internal
*/

void QTimeZoneWidget::slotExecute( void )
{
    QWorldmapDialog* map = new QWorldmapDialog( this );

    if ( cmbTz->prevZone().isEmpty() || ( cmbTz->prevZone() == "None" ) )
        map->setZone( QTimeZone(getenv( "TZ" )) );
    else
        map->setZone( QTimeZone( cmbTz->prevZone().toLatin1() ) );

    map->setModal( true );
    if ( map->exec() == QDialog::Accepted && map->selectedZone().isValid() ) {
        cmbTz->setCurrZone( map->selectedZone().id() );
    } else {
        cmbTz->setToPreviousIndex();
    }
}

#include "qtimezonewidget.moc"

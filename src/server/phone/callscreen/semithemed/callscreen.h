/****************************************************************************
**
** Copyright (C) 2008-2008 TROLLTECH ASA. All rights reserved.
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

#ifndef CALLSCREEN_H
#define CALLSCREEN_H

#include <themedview.h>

#include <QListWidget>
#include "serverthemeview.h"
#include <QPhoneCall>
#include <QSimControlEvent>

class DialerControl;
class QMenu;
class CallItemEntry;
class QAction;
class QLineEdit;
class QVBoxLayout;
class CallItemModel;
class SecondaryCallScreen;
class ThemeListModel;
class CallAudioHandler;
class QAudioStateConfiguration;
class QPhoneCall;

class QSimToolkit;
class QAbstractMessageBox;
class MouseControlDialog;
class CallScreenVolumeView;

class CallScreenView : public PhoneThemedView {
    Q_OBJECT

public:
    CallScreenView(QWidget* parent);

Q_SIGNALS:
    void themeWasLoaded(const QString&);

protected:
    QWidget *newWidget(ThemeWidgetItem* input, const QString& name);
    void themeLoaded(const QString&);

};

class CallScreen : public QWidget
{
    friend class CallItemListView;
    friend class CallItemEntry;
    friend class CallScreenView;

    Q_OBJECT
public:
    CallScreen(DialerControl *ctrl, QWidget *parent, Qt::WFlags fl = 0);

    QString ringTone();
    int activeCallCount() const { return m_activeCount; }
    int heldCallCount() const { return m_holdCount; }
    bool incomingCall() const { return m_incoming; }
    bool inMultiCall() const { return m_activeCount > 1 || m_holdCount > 1; }
    void makeVisible();

    PhoneThemedView* view() const { return m_view; }

signals:
    void acceptIncoming();
    void muteRing();
    void listEmpty();
    void testKeys(const QString&, bool&);
    void filterKeys(const QString&, bool&);
    void filterSelect(const QString&, bool&);

public slots:
    void stateChanged();
    void requestFailed(const QPhoneCall &,QPhoneCall::Request);
    CallItemEntry *findCall(const QPhoneCall &call, CallItemModel *model);
    void themeLoaded( const QString &theme );

protected:
    void showEvent(QShowEvent *);
    void keyPressEvent(QKeyEvent *);
    void keyReleaseEvent(QKeyEvent *);
    void closeEvent(QCloseEvent *);
    bool eventFilter(QObject *, QEvent *);
    void hideEvent( QHideEvent * );

private slots:
    void updateAll();
    void splitCall();
    void callSelected(const QModelIndex&);
    void themeItemReleased(ThemeItem*);
    void actionGsmSelected();
    void updateLabels();
    void initializeAudioConf();
    void simControlEvent(const QSimControlEvent &);
    void muteRingSelected();
    void callConnected(const QPhoneCall &);
    void callDropped(const QPhoneCall &);
    void callIncoming(const QPhoneCall &);
    void callDialing(const QPhoneCall &);
    void rejectModalDialog();

private:
    void updateAll(bool forceRemoval);
    void clearDtmfDigits(bool clearOneChar = false);
    void appendDtmfDigits(const QString &);
    void setSelectMode(bool);
    bool dialNumbers(const QString & numbers);
    void setGsmMenuItem();
    void setItemActive(const QString &name, bool active);
    void setItemText(const QString& name, const QString& text);
    void updatePowerStatus();

private:
    // User interface
    CallScreenView *m_view;
    QListView *m_listView;
    ThemeListModel* m_model;

    // dtmf handling
    QLineEdit *m_digits;
    QString m_dtmfActiveCall;
    QString m_dtmfDigits;

    // Softmenu handling
    QMenu *m_contextMenu;
    QAction *m_actionAnswer;
    QAction *m_actionSendBusy;
    QAction *m_actionMute;
    QAction *m_actionHold;
    QAction *m_actionResume;
    QAction *m_actionEnd;
    QAction *m_actionEndAll;
    QAction *m_actionMerge;
    QAction *m_actionSplit;
    QAction *m_actionTransfer;
    QAction *m_actionGsm;
    CallScreenVolumeView *m_volumeView;

    // call handling
    DialerControl *m_control;
    int m_activeCount;
    int m_holdCount;
    bool m_incoming;
    QTimer* m_updateTimer;
    QTimer* m_gsmActionTimer;
    CallAudioHandler* m_callAudioHandler;
    QAudioStateConfiguration *m_audioConf;
    QSimToolkit *m_simToolkit;
    QAbstractMessageBox *m_simMsgBox;
    QTimer *m_symbolTimer;

    bool m_hadActiveCall;
};

#endif

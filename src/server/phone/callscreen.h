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

#ifndef CALLSCREEN_H
#define CALLSCREEN_H

#include <themedview.h>

#include <QListWidget>
#include "phonethemeview.h"
#include <QPhoneCall>
#ifdef QTOPIA_CELL
#include <QSimControlEvent>
#endif

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

class CallScreen : public PhoneThemedView
{
    friend class CallItemDelegate;
    friend class CallItemListView;
    friend class CallItemEntry;

    Q_OBJECT
public:
    CallScreen(DialerControl *ctrl, QWidget *parent, Qt::WFlags fl = 0);

    QString ringTone();
    int activeCallCount() const { return activeCount; }
    int heldCallCount() const { return holdCount; }
    bool incomingCall() const { return incoming; }
    bool inMultiCall() const { return activeCount > 1 || holdCount > 1; }

signals:
    void acceptIncoming();
    void increaseCallVolume();
    void decreaseCallVolume();
    void muteRing();
    void listEmpty();
    void testKeys(const QString&, bool&);
    void filterKeys(const QString&, bool&);
    void filterSelect(const QString&, bool&);

public slots:
    void stateChanged();
    void requestFailed(const QPhoneCall &,QPhoneCall::Request);
    CallItemEntry *findCall(const QPhoneCall &call, CallItemModel *model);

protected slots:
    void manualLayout();

protected:
    virtual void themeLoaded( const QString &theme );
    QWidget *newWidget(ThemeWidgetItem* input, const QString& name);
    void showEvent(QShowEvent *);
    void keyPressEvent(QKeyEvent *);
    void keyReleaseEvent(QKeyEvent *);
    void closeEvent(QCloseEvent *);
    bool eventFilter(QObject *, QEvent *);
    void hideEvent( QHideEvent * );
    void mouseReleaseEvent(QMouseEvent *);

private slots:
    void updateAll();
    void splitCall();
    void callSelected(const QModelIndex&);
    void callClicked(const QModelIndex&);
    void themeItemClicked(ThemeItem*);
    void actionGsmSelected();
    void updateLabels();
    void setVideoWidget();
    void deleteVideoWidget();
    void initializeAudioConf();
#ifdef QTOPIA_CELL
    void simControlEvent(const QSimControlEvent &);
#endif
    void grabMouse();
    void releaseMouse();
    void muteRingSelected();
    void callConnected(const QPhoneCall &);
    void callDropped(const QPhoneCall &);
    void callIncoming(const QPhoneCall &);
    void callDialing(const QPhoneCall &);
    void showProgressDlg();
    void hideProgressDlg();
    void interactionDelayTimeout();
    void rejectModalDialog();

private:
    void clearDtmfDigits(bool clearOneChar = false);
    void appendDtmfDigits(const QString &);
    void setSelectMode(bool);
    int callListHeight() const;
    bool dialNumbers(const QString & numbers);
    void setGsmMenuItem();
    void setItemActive(const QString &name, bool active);

private:
    QString dtmfActiveCall;
    QString dtmfDigits;
    DialerControl *control;
    QLineEdit *digits;
    QListView *listView;
    QMenu *contextMenu;
    QAction *actionAnswer;
    QAction *actionSendBusy;
    QAction *actionMute;
    QAction *actionHold;
    QAction *actionResume;
    QAction *actionEnd;
    QAction *actionEndAll;
    QAction *actionMerge;
    QAction *actionSplit;
    QAction *actionTransfer;
    QAction *actionGsm;
    int activeCount;
    int holdCount;
    bool incoming;
    bool keypadVisible;
    QVBoxLayout *mLayout;
    QTimer* updateTimer;
    QTimer* gsmActionTimer;
    SecondaryCallScreen *secondaryCallScreen;
    ThemeListModel* m_model;
    CallAudioHandler* m_callAudioHandler;
    QWidget* videoWidget;
    QAudioStateConfiguration *m_audioConf;
#ifdef QTOPIA_CELL
    QSimToolkit *simToolkit;
#endif
    QAbstractMessageBox *simMsgBox;
    bool showWaitDlg;
    QTimer *symbolTimer;
};

#endif

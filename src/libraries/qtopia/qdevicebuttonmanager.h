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
#ifndef DEVICE_BUTTON_MANAGER_H
#define DEVICE_BUTTON_MANAGER_H

#include <qobject.h>
#include <qstring.h>
#include <qdevicebutton.h>

class QtopiaServiceRequest;
class QTranslatableSettings;

class QTOPIA_EXPORT QDeviceButtonManager : public QObject {
  Q_OBJECT

 public:
  static QDeviceButtonManager& instance();

  const QList<QDeviceButton*>& buttons() const;

  const QDeviceButton* buttonForKeycode(int keyCode) const;
  const QDeviceButton* buttonForKeycode(int keyCode, const QString& context) const;

  void remapPressedAction(int button, const QtopiaServiceRequest& qcopMessage);
  void remapHeldAction(int button, const QtopiaServiceRequest& qcopMessage);
  void remapReleasedAction(int action, const QtopiaServiceRequest& qcopMessage);

  void factoryResetButtons();

 protected:
  QDeviceButtonManager();
  virtual ~QDeviceButtonManager();

  private slots:
    void received(const QString& message, const QByteArray& data);

 private:
  void loadButtons(bool factory=false);
  void loadButtonSettings(QTranslatableSettings&,bool local,bool factory);

  static QPointer<QDeviceButtonManager> m_Instance;
  QList<QDeviceButton*> m_Buttons;
};

#define QTOPIA_DEFINED_DEVICEBUTTONMANAGER

#endif

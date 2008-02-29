/****************************************************************************
**
** Copyright (C) 1992-2006 TROLLTECH ASA. All rights reserved.
**
** This file is part of the Phone Edition of the Qt Toolkit.
**
** $TROLLTECH_DUAL_LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef Q3PROGRESSDIALOG_H
#define Q3PROGRESSDIALOG_H

#include <QtGui/qdialog.h>

QT_BEGIN_HEADER

QT_MODULE(Qt3SupportLight)

#ifndef QT_NO_PROGRESSDIALOG

class Q3ProgressDialogData;
class QLabel;
class QPushButton;
class QTimer;
class Q3ProgressBar;

class Q_COMPAT_EXPORT Q3ProgressDialog : public QDialog
{
    Q_OBJECT
    Q_PROPERTY(bool wasCanceled READ wasCanceled)
    Q_PROPERTY(bool wasCancelled READ wasCancelled DESIGNABLE false STORED false)
    Q_PROPERTY(int totalSteps READ totalSteps WRITE setTotalSteps)
    Q_PROPERTY(int progress READ progress WRITE setProgress)
    Q_PROPERTY(bool autoReset READ autoReset WRITE setAutoReset)
    Q_PROPERTY(bool autoClose READ autoClose WRITE setAutoClose)
    Q_PROPERTY(int minimumDuration READ minimumDuration WRITE setMinimumDuration)
    Q_PROPERTY(QString labelText READ labelText WRITE setLabelText)

public:
    Q3ProgressDialog(QWidget* parent, const char* name, bool modal=false,
                                           Qt::WindowFlags f=0);
    Q3ProgressDialog(const QString& labelText,
                                           const QString &cancelButtonText, int totalSteps,
                                           QWidget* parent=0, const char* name=0,
                                           bool modal=false, Qt::WindowFlags f=0);
    Q3ProgressDialog(QWidget* parent = 0, Qt::WindowFlags f = 0);
    Q3ProgressDialog(const QString& labelText, const QString &cancelButtonText,
                     int totalSteps, QWidget* parent=0, Qt::WindowFlags f=0);
    ~Q3ProgressDialog();

    void setLabel(QLabel *);
    void setCancelButton(QPushButton *);
    void setBar(Q3ProgressBar *);

    bool wasCanceled() const;
    bool wasCancelled() const;

    int totalSteps() const;
    int progress()   const;

    QSize sizeHint() const;

    QString labelText() const;

    void setAutoReset(bool b);
    bool autoReset() const;
    void setAutoClose(bool b);
    bool autoClose() const;

public Q_SLOTS:
    void cancel();
    void reset();
    void setTotalSteps(int totalSteps);
    void setProgress(int progress);
    void setProgress(int progress, int totalSteps);
    void setLabelText(const QString &);
    void setCancelButtonText(const QString &);

    void setMinimumDuration(int ms);
public:
    int minimumDuration() const;

Q_SIGNALS:
    void canceled();
    void cancelled();

protected:
    void resizeEvent(QResizeEvent *);
    void closeEvent(QCloseEvent *);
    void changeEvent(QEvent *);
    void showEvent(QShowEvent *e);

protected Q_SLOTS:
    void forceShow();

private:
    void init(QWidget *creator, const QString& lbl, const QString &canc,
              int totstps);
    void layout();
    QLabel *label()  const;
    Q3ProgressBar *bar()    const;
    Q3ProgressDialogData *d;
    QTimer *forceTimer;

private:
    Q_DISABLE_COPY(Q3ProgressDialog)
};

#endif // QT_NO_PROGRESSDIALOG

QT_END_HEADER

#endif // Q3PROGRESSDIALOG_H

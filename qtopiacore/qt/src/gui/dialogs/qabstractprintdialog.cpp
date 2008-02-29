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

#include "qabstractprintdialog.h"
#include "qabstractprintdialog_p.h"
#include "qprinter.h"
#include "private/qprinter_p.h"

#ifndef QT_NO_PRINTDIALOG

/*!
    \class QAbstractPrintDialog
    \brief The QAbstractPrintDialog class provides a base implementation for
    print dialogs used to configure printers.
*/

/*!
    \enum QAbstractPrintDialog::PrintRange

    Used to specify the print range selection option.

    \value AllPages All pages should be printed.
    \value Selection Only the selection should be printed.
    \value PageRange The specified page range should be printed.

    \sa QPrinter::PrintRange
*/

/*!
    \enum QAbstractPrintDialog::PrintDialogOption

    Used to specify which parts of the print dialog should be enabled.

    \value None None of the options are enabled.
    \value PrintToFile The print to file option is enabled.
    \value PrintSelection The print selection option is enalbed.
    \value PrintPageRange The page range selection option is enabled.
    \value PrintCollateCopies
*/

/*!
    Constructs an abstract print dialog for \a printer with \a parent
    as parent widget.
*/
QAbstractPrintDialog::QAbstractPrintDialog(QPrinter *printer, QWidget *parent)
    : QDialog(*(new QAbstractPrintDialogPrivate), parent)
{
    Q_D(QAbstractPrintDialog);
    d->printer = printer;

    if (printer->d_func()->printDialog)
        d->init(printer->d_func()->printDialog->d_func());

    printer->d_func()->printDialog = this;
}

/*!
     \internal
*/
QAbstractPrintDialog::QAbstractPrintDialog(QAbstractPrintDialogPrivate &ptr,
                                           QPrinter *printer,
                                           QWidget *parent)
    : QDialog(ptr, parent)
{
    Q_D(QAbstractPrintDialog);
    d->printer = printer;
    if (printer->d_func()->printDialog)
        d->init(printer->d_func()->printDialog->d_func());
    printer->d_func()->printDialog = this;
}


/*!
    Sets the set of options that should be enabled in the print dialog
    to \a options.

    Note that this function has no effect on Mac OS X. See the QPrintDialog
    documentation for more information.
*/
void QAbstractPrintDialog::setEnabledOptions(PrintDialogOptions options)
{
    Q_D(QAbstractPrintDialog);
    d->options = options;
}

/*!
    Adds the option \a option to the set of enabled options in this dialog.

    Note that this function has no effect on Mac OS X. See the QPrintDialog
    documentation for more information.
*/
void QAbstractPrintDialog::addEnabledOption(PrintDialogOption option)
{
    Q_D(QAbstractPrintDialog);
    d->options |= option;
}

/*!
    Returns the set of enabled options in this dialog.
*/
QAbstractPrintDialog::PrintDialogOptions QAbstractPrintDialog::enabledOptions() const
{
    Q_D(const QAbstractPrintDialog);
    return d->options;
}

/*!
    Returns true if the specified \a option is enabled; otherwise returns false
*/
bool QAbstractPrintDialog::isOptionEnabled(PrintDialogOption option) const
{
    Q_D(const QAbstractPrintDialog);
    return d->options & option;
}

/*!
    Sets the print range option in to be \a range.
 */
void QAbstractPrintDialog::setPrintRange(PrintRange range)
{
    Q_D(QAbstractPrintDialog);
    d->printRange = range;
}

/*!
    Returns the print range.
*/
QAbstractPrintDialog::PrintRange QAbstractPrintDialog::printRange() const
{
    Q_D(const QAbstractPrintDialog);
    return d->printRange;
}

/*!
    Sets the page range in this dialog to be from \a min to \a max. This also
    enables the PrintPageRange option.
*/
void QAbstractPrintDialog::setMinMax(int min, int max)
{
    Q_D(QAbstractPrintDialog);
    Q_ASSERT_X(min <= max, "QAbstractPrintDialog::setMinMax",
               "'min' must be less than or equal to 'max'");
    d->minPage = min;
    d->maxPage = max;
    d->options |= PrintPageRange;
}

/*!
    Returns the minimum page in the page range.
*/
int QAbstractPrintDialog::minPage() const
{
    Q_D(const QAbstractPrintDialog);
    return d->minPage;
}

/*!
    Returns the maximum page in the page range.
*/
int QAbstractPrintDialog::maxPage() const
{
    Q_D(const QAbstractPrintDialog);
    return d->maxPage;
}

/*!
    Sets the range in the print dialog to be from \a from to \a to.
*/
void QAbstractPrintDialog::setFromTo(int from, int to)
{
    Q_D(QAbstractPrintDialog);
    Q_ASSERT_X(from <= to, "QAbstractPrintDialog::setFromTo",
               "'from' must be less than or equal to 'to'");
    d->fromPage = from;
    d->toPage = to;

    if (d->minPage == 0 && d->maxPage == 0)
        setMinMax(1, to);
}

/*!
    Returns the first page to be printed
*/
int QAbstractPrintDialog::fromPage() const
{
    Q_D(const QAbstractPrintDialog);
    return d->fromPage;
}

/*!
    Returns the last page to be printed.
*/
int QAbstractPrintDialog::toPage() const
{
    Q_D(const QAbstractPrintDialog);
    return d->toPage;
}

/*!
    Returns the printer that this printer dialog operates
    on.
*/
QPrinter *QAbstractPrintDialog::printer() const
{
    Q_D(const QAbstractPrintDialog);
    return d->printer;
}

/*!
    \fn int QAbstractPrintDialog::exec()

    This virtual function is called to pop up the dialog. It must be
    reimplemented in subclasses.
*/

/*!
    \class QPrintDialog

    \brief The QPrintDialog class provides a dialog for specifying
    the printer's configuration.

    \ingroup dialogs

    The dialog allows users to change document-related settings, such
    as the paper size and orientation, type of print (color or
    grayscale), range of pages, and number of copies to print.

    Controls are also provided to enable users to choose from the
    printers available, including any configured network printers.

    Typically, QPrintDialog objects are constructed with a QPrinter
    object, and executed using the exec() function.

    \code
        QPrintDialog printDialog(printer, parent);
        if (printDialog.exec() == QDialog::Accepted) {
            // print ...
        }
    \endcode

    If the dialog is accepted by the user, the QPrinter object is
    correctly configured for printing.

    \raw HTML
    <table align="center">
    <tr><td>
    \endraw
    \inlineimage plastique-printdialog.png
    \raw HTML
    </td><td>
    \endraw
    \inlineimage plastique-printdialog-properties.png
    \raw HTML
    </td></tr>
    </table>
    \endraw

    The printer dialog (shown above in Plastique style) enables access to common
    printing properties. On X11 platforms that use the CUPS printing system, the
    settings for each available printer can be modified via the dialog's
    \gui{Properties} push button.

    On Windows and Mac OS X, the native print dialog is used, which
    means that some QWidget and QDialog properties set on the dialog
    won't be respected. In addition, the native print dialog on Mac OS X does
    not support setting printer options, i.e. QAbstractPrintDialog::setEnabledOptions()
    and QAbstractPrintDialog::addEnabledOption() have no effect.

    \sa QPageSetupDialog, QPrinter, {Pixelator Example}, {Order Form Example},
        {Image Viewer Example}, {Scribble Example}
*/

/*!
    \fn QPrintDialog::QPrintDialog(QPrinter *printer, QWidget *parent)

    Constructs a new modal printer dialog for the given \a printer
    with the given \a parent.
*/

/*!
    \fn QPrintDialog::~QPrintDialog()

    Destroys the print dialog.
*/

/*!
    \fn int QPrintDialog::exec()
    \reimp
*/

#endif // QT_NO_PRINTDIALOG

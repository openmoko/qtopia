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

#include "qvalidator.h"
#ifndef QT_NO_VALIDATOR

#include <limits.h>
#include <math.h>

/*!
    \class QValidator
    \brief The QValidator class provides validation of input text.

    \ingroup misc
    \mainclass

    The class itself is abstract. Two subclasses, \l QIntValidator and
    \l QDoubleValidator, provide basic numeric-range checking, and \l
    QRegExpValidator provides general checking using a custom regular
    expression.

    If the built-in validators aren't sufficient, you can subclass
    QValidator. The class has two virtual functions: validate() and
    fixup().

    \l validate() must be implemented by every subclass. It returns
    \l Invalid, \l Intermediate or \l Acceptable depending on whether
    its argument is valid (for the subclass's definition of valid).

    These three states require some explanation. An \l Invalid string
    is \e clearly invalid. \l Intermediate is less obvious: the
    concept of validity is difficult to apply when the string is
    incomplete (still being edited). QValidator defines \l Intermediate
    as the property of a string that is neither clearly invalid nor
    acceptable as a final result. \l Acceptable means that the string
    is acceptable as a final result. One might say that any string
    that is a plausible intermediate state during entry of an \l
    Acceptable string is \l Intermediate.

    Here are some examples:

    \list

    \i For a line edit that accepts integers from 10 to 999 inclusive,
    42 and 123 are \l Acceptable, the empty string and 5 are \l
    Intermediate, and "asdf" and 1114 is \l Invalid.

    \i For an editable combobox that accepts URLs, any well-formed URL
    is \l Acceptable, "http://www.trolltech.com/," is \l Intermediate
    (it might be a cut and paste action that accidentally took in a
    comma at the end), the empty string is \l Intermediate (the user
    might select and delete all of the text in preparation for entering
    a new URL) and "http:///./" is \l Invalid.

    \i For a spin box that accepts lengths, "11cm" and "1in" are \l
    Acceptable, "11" and the empty string are \l Intermediate, and
    "http://www.trolltech.com" and "hour" are \l Invalid.

    \endlist

    \l fixup() is provided for validators that can repair some user
    errors. The default implementation does nothing. QLineEdit, for
    example, will call fixup() if the user presses Enter (or Return)
    and the content is not currently valid. This allows the fixup()
    function the opportunity of performing some magic to make an \l
    Invalid string \l Acceptable.

    QValidator is typically used with QLineEdit, QSpinBox and
    QComboBox.

    \sa QIntValidator, QDoubleValidator, QRegExpValidator, {Line Edits Example}
*/


/*!
    \enum QValidator::State

    This enum type defines the states in which a validated string can
    exist.

    \value Invalid       The string is \e clearly invalid.
    \value Intermediate  The string is a plausible intermediate value
                         during editing.
    \value Acceptable    The string is acceptable as a final result;
                         i.e. it is valid.

    \omitvalue Valid
*/


/*!
    Sets up the validator. The \a parent parameter is
    passed on to the QObject constructor.
*/

QValidator::QValidator(QObject * parent)
    : QObject(parent)
{
}

#ifdef QT3_SUPPORT
/*!
  \obsolete
    Sets up the validator. The \a parent and \a name parameters are
    passed on to the QObject constructor.
*/

QValidator::QValidator(QObject * parent, const char *name)
    : QObject(parent)
{
    setObjectName(QString::fromAscii(name));
}
#endif

/*!
    Destroys the validator, freeing any storage and other resources
    used.
*/

QValidator::~QValidator()
{
}


/*!
    \fn QValidator::State QValidator::validate(QString &input, int &pos) const

    This virtual function returns \l Invalid if \a input is invalid
    according to this validator's rules, \l Intermediate if it
    is likely that a little more editing will make the input
    acceptable (e.g. the user types "4" into a widget which accepts
    integers between 10 and 99), and \l Acceptable if the input is
    valid.

    The function can change both \a input and \a pos (the cursor position)
    if required.
*/


/*!
    \fn void QValidator::fixup(QString & input) const

    This function attempts to change \a input to be valid according to
    this validator's rules. It need not result in a valid string:
    callers of this function must re-test afterwards; the default does
    nothing.

    Reimplementations of this function can change \a input even if
    they do not produce a valid string. For example, an ISBN validator
    might want to delete every character except digits and "-", even
    if the result is still not a valid ISBN; a surname validator might
    want to remove whitespace from the start and end of the string,
    even if the resulting string is not in the list of accepted
    surnames.
*/

void QValidator::fixup(QString &) const
{
}


/*!
    \class QIntValidator
    \brief The QIntValidator class provides a validator that ensures
    a string contains a valid integer within a specified range.

    \ingroup misc

    Example of use:

    \code
    QValidator *validator = new QIntValidator(100, 999, this);
    QLineEdit *edit = new QLineEdit(this);

    // the edit lineedit will only accept integers between 100 and 999
    edit->setValidator(validator);
    \endcode

    Below we present some examples of validators. In practice they would
    normally be associated with a widget as in the example above.

    \code
    QString str;
    int pos = 0;
    QIntValidator v(100, 999, this);

    str = "1";
    v.validate(str, pos);     // returns Intermediate
    str = "12";
    v.validate(str, pos);     // returns Intermediate

    str = "123";
    v.validate(str, pos);     // returns Acceptable
    str = "678";
    v.validate(str, pos);     // returns Acceptable

    str = "1234";
    v.validate(str, pos);     // returns Invalid
    str = "-123";
    v.validate(str, pos);     // returns Invalid
    str = "abc";
    v.validate(str, pos);     // returns Invalid
    str = "12cm";
    v.validate(str, pos);     // returns Invalid
    \endcode

    The minimum and maximum values are set in one call with setRange(),
    or individually with setBottom() and setTop().

    \sa QDoubleValidator, QRegExpValidator, {Line Edits Example}
*/


/*!
    Constructs a validator with a \a parent object that
    accepts all integers.
*/

QIntValidator::QIntValidator(QObject * parent)
    : QValidator(parent)
{
    b = INT_MIN;
    t = INT_MAX;
}


/*!
    Constructs a validator with a \a parent, that accepts integers
    from \a minimum to \a maximum inclusive.
*/

QIntValidator::QIntValidator(int minimum, int maximum,
                              QObject * parent)
    : QValidator(parent)
{
    b = minimum;
    t = maximum;
}


#ifdef QT3_SUPPORT
/*!
  \obsolete

    Constructs a validator with a \a parent object and a \a name that
    accepts all integers.
*/

QIntValidator::QIntValidator(QObject * parent, const char *name)
    : QValidator(parent)
{
    setObjectName(QString::fromAscii(name));
    b = INT_MIN;
    t = INT_MAX;
}


/*!
  \obsolete

    Constructs a validator called \a name with a \a parent, that
    accepts integers from \a minimum to \a maximum inclusive.
*/

QIntValidator::QIntValidator(int minimum, int maximum,
                              QObject * parent, const char* name)
    : QValidator(parent)
{
    setObjectName(QString::fromAscii(name));
    b = minimum;
    t = maximum;
}
#endif

/*!
    Destroys the validator.
*/

QIntValidator::~QIntValidator()
{
    // nothing
}


/*!
    \fn QValidator::State QIntValidator::validate(QString &input, int &pos) const

    Returns \l Acceptable if the \a input is an integer within the
    valid range, \l Intermediate if the \a input is an integer outside
    the valid range and \l Invalid if the \a input is not an integer.

    Note: If the valid range consists of just positive integers (e.g. 32 to 100)
    and \a input is a negative integer then Invalid is returned.

    \code
    int pos = 0;

    s = "abc";
    v.validate(s, pos);    // returns Invalid

    s = "5";
    v.validate(s, pos);    // returns Intermediate

    s = "50";
    v.validate(s, pos);    // returns Acceptable
    \endcode

    By default, the \a pos parameter is not used by this validator.
*/

QValidator::State QIntValidator::validate(QString & input, int &) const
{
    if (input.contains(QLatin1Char(' ')))
        return Invalid;
    if (input.isEmpty() || (b < 0 && input == QLatin1String("-")))
        return Intermediate;
    bool ok;
    int entered = input.toInt(&ok);
    if (!ok || (entered < 0 && b >= 0)) {
        return Invalid;
    } else if (entered >= b && entered <= t) {
        return Acceptable;
    } else {
        if (entered >= 0)
            return (entered > t) ? Invalid : Intermediate;
        else
            return (entered < b) ? Invalid : Intermediate;
    }
}


/*!
    Sets the range of the validator to only accept integers between \a
    bottom and \a top inclusive.
*/

void QIntValidator::setRange(int bottom, int top)
{
    b = bottom;
    t = top;
}


/*!
    \property QIntValidator::bottom
    \brief the validator's lowest acceptable value

    \sa setRange()
*/
void QIntValidator::setBottom(int bottom)
{
    setRange(bottom, top());
}

/*!
    \property QIntValidator::top
    \brief the validator's highest acceptable value

    \sa setRange()
*/
void QIntValidator::setTop(int top)
{
    setRange(bottom(), top);
}


#ifndef QT_NO_REGEXP

/*!
    \class QDoubleValidator

    \brief The QDoubleValidator class provides range checking of
    floating-point numbers.

    \ingroup misc

    QDoubleValidator provides an upper bound, a lower bound, and a
    limit on the number of digits after the decimal point. It does not
    provide a fixup() function.

    You can set the acceptable range in one call with setRange(), or
    with setBottom() and setTop(). Set the number of decimal places
    with setDecimals(). The validate() function returns the validation
    state.

    \sa QIntValidator, QRegExpValidator, {Line Edits Example}
*/

/*!
    Constructs a validator object with a \a parent object
    that accepts any double.
*/

QDoubleValidator::QDoubleValidator(QObject * parent)
    : QValidator(parent)
{
    b = -HUGE_VAL;
    t = HUGE_VAL;
    d = 1000;
}


/*!
    Constructs a validator object with a \a parent object. This
    validator will accept doubles from \a bottom to \a top inclusive,
    with up to \a decimals digits after the decimal point.
*/

QDoubleValidator::QDoubleValidator(double bottom, double top, int decimals,
                                    QObject * parent)
    : QValidator(parent)
{
    b = bottom;
    t = top;
    d = decimals;
}

#ifdef QT3_SUPPORT
/*!
  \obsolete

    Constructs a validator object with a \a parent object and a \a name
    that accepts any double.
*/

QDoubleValidator::QDoubleValidator(QObject * parent, const char *name)
    : QValidator(parent)
{
    setObjectName(QString::fromAscii(name));
    b = -HUGE_VAL;
    t = HUGE_VAL;
    d = 1000;
}


/*!
  \obsolete

    Constructs a validator object with a \a parent object, called \a
    name. This validator will accept doubles from \a bottom to \a top
    inclusive, with up to \a decimals digits after the decimal point.
*/

QDoubleValidator::QDoubleValidator(double bottom, double top, int decimals,
                                    QObject * parent, const char* name)
    : QValidator(parent)
{
    setObjectName(QString::fromAscii(name));
    b = bottom;
    t = top;
    d = decimals;
}
#endif

/*!
    Destroys the validator.
*/

QDoubleValidator::~QDoubleValidator()
{
}


/*!
    \fn QValidator::State QDoubleValidator::validate(QString &input, int &pos) const

    Returns \l Acceptable if the string \a input contains a double
    that is within the valid range and is in the correct format.

    Returns \l Intermediate if \a input contains a double that is
    outside the range or is in the wrong format; e.g. with too many
    digits after the decimal point or is empty.

    Returns \l Invalid if the \a input is not a double.

    Note: If the valid range consists of just positive doubles (e.g. 0.0 to 100.0)
    and \a input is a negative double then \l Invalid is returned.

    By default, the \a pos parameter is not used by this validator.
*/

QValidator::State QDoubleValidator::validate(QString & input, int &) const
{
    QRegExp empty(QString::fromLatin1("-?\\.?"));
    if (input.contains(QLatin1Char(' ')))
        return Invalid;
    if (b >= 0 && input.startsWith(QLatin1Char('-')))
        return Invalid;
    if (empty.exactMatch(input))
        return Intermediate;
    bool ok = true;
    double entered = input.toDouble(&ok);
    int nume = input.count(QLatin1Char('e'), Qt::CaseInsensitive);
    if (!ok) {
        // explicit exponent regexp
        QRegExp expexpexp(QString::fromLatin1("[Ee][+-]?(\\d*)$"));
        int eePos = expexpexp.indexIn(input);
        if (eePos > 0 && nume == 1) {
            QString mantissa = input.left(eePos);
            entered = mantissa.toDouble(&ok);
            if (!ok)
                return Invalid;
            if (expexpexp.cap(1).isEmpty())
                return Intermediate;
        } else if (eePos == 0) {
            return Intermediate;
        } else {
            return Invalid;
        }
    }

    int i = input.indexOf(QLatin1Char('.'));
    if (i >= 0 && nume == 0) {
        // has decimal point (but no E), now count digits after that
        i++;
        int j = i;
        while(input[j].isDigit())
            j++;
        if (j - i > d)
            return Intermediate;
    }

    if (entered < b || entered > t)
        return Intermediate;
    return Acceptable;
}


/*!
    Sets the validator to accept doubles from \a minimum to \a maximum
    inclusive, with at most \a decimals digits after the decimal
    point.
*/

void QDoubleValidator::setRange(double minimum, double maximum, int decimals)
{
    b = minimum;
    t = maximum;
    d = decimals;
}

/*!
    \property QDoubleValidator::bottom
    \brief the validator's minimum acceptable value

    \sa setRange()
*/

void QDoubleValidator::setBottom(double bottom)
{
    setRange(bottom, top(), decimals());
}


/*!
    \property QDoubleValidator::top
    \brief the validator's maximum acceptable value

    \sa setRange()
*/

void QDoubleValidator::setTop(double top)
{
    setRange(bottom(), top, decimals());
}

/*!
    \property QDoubleValidator::decimals
    \brief the validator's maximum number of digits after the decimal point

    \sa setRange()
*/

void QDoubleValidator::setDecimals(int decimals)
{
    setRange(bottom(), top(), decimals);
}


/*!
    \class QRegExpValidator
    \brief The QRegExpValidator class is used to check a string
    against a regular expression.

    \ingroup misc

    QRegExpValidator uses a regular expression (regexp) to
    determine whether an input string is \l Acceptable, \l
    Intermediate, or \l Invalid. The regexp can either be supplied
    when the QRegExpValidator is constructed, or at a later time.

    The regexp is treated as if it begins with the start of string
    assertion, \bold{^}, and ends with the end of string assertion
    \bold{$} so the match is against the entire input string, or from
    the given position if a start position greater than zero is given.

    For a brief introduction to Qt's regexp engine see \l QRegExp.

    Example of use:
    \code
    // regexp: optional '-' followed by between 1 and 3 digits
    QRegExp rx("-?\\d{1,3}");
    QValidator *validator = new QRegExpValidator(rx, this);

    QLineEdit *edit = new QLineEdit(this);
    edit->setValidator(validator);
    \endcode

    Below we present some examples of validators. In practice they would
    normally be associated with a widget as in the example above.

    \code
    // integers 1 to 9999
    QRegExp rx("[1-9]\\d{0,3}");
    // the validator treats the regexp as "^[1-9]\\d{0,3}$"
    QRegExpValidator v(rx, 0);
    QString s;
    int pos = 0;

    s = "0";     v.validate(s, pos);    // returns Invalid
    s = "12345"; v.validate(s, pos);    // returns Invalid
    s = "1";     v.validate(s, pos);    // returns Acceptable

    rx.setPattern("\\S+");            // one or more non-whitespace characters
    v.setRegExp(rx);
    s = "myfile.txt";  v.validate(s, pos); // Returns Acceptable
    s = "my file.txt"; v.validate(s, pos); // Returns Invalid

    // A, B or C followed by exactly five digits followed by W, X, Y or Z
    rx.setPattern("[A-C]\\d{5}[W-Z]");
    v.setRegExp(rx);
    s = "a12345Z"; v.validate(s, pos);        // Returns Invalid
    s = "A12345Z"; v.validate(s, pos);        // Returns Acceptable
    s = "B12";     v.validate(s, pos);        // Returns Intermediate

    // match most 'readme' files
    rx.setPattern("read\\S?me(\.(txt|asc|1st))?");
    rx.setCaseSensitive(false);
    v.setRegExp(rx);
    s = "readme";      v.validate(s, pos); // Returns Acceptable
    s = "README.1ST";  v.validate(s, pos); // Returns Acceptable
    s = "read me.txt"; v.validate(s, pos); // Returns Invalid
    s = "readm";       v.validate(s, pos); // Returns Intermediate
    \endcode

    \sa QRegExp, QIntValidator, QDoubleValidator, {Settings Editor Example}
*/

/*!
    Constructs a validator with a \a parent object that accepts
    any string (including an empty one) as valid.
*/

QRegExpValidator::QRegExpValidator(QObject *parent)
    : QValidator(parent), r(QString::fromLatin1(".*"))
{
}

/*!
    Constructs a validator with a \a parent object that
    accepts all strings that match the regular expression \a rx.

    The match is made against the entire string; e.g. if the regexp is
    \bold{[A-Fa-f0-9]+} it will be treated as \bold{^[A-Fa-f0-9]+$}.
*/

QRegExpValidator::QRegExpValidator(const QRegExp& rx, QObject *parent)
    : QValidator(parent), r(rx)
{
}

#ifdef QT3_SUPPORT
/*!
  \obsolete

    Constructs a validator with a \a parent object and \a name that accepts
    any string (including an empty one) as valid.
*/

QRegExpValidator::QRegExpValidator(QObject *parent, const char *name)
    : QValidator(parent), r(QString::fromLatin1(".*"))
{
        setObjectName(QString::fromAscii(name));
}

/*!
  \obsolete

    Constructs a validator with a \a parent object and a \a name that
    accepts all strings that match the regular expression \a rx.

    The match is made against the entire string; e.g. if the regexp is
    \bold{[A-Fa-f0-9]+} it will be treated as \bold{^[A-Fa-f0-9]+$}.
*/

QRegExpValidator::QRegExpValidator(const QRegExp& rx, QObject *parent,
                                    const char *name)
    : QValidator(parent), r(rx)
{
        setObjectName(QString::fromAscii(name));
}
#endif

/*!
    Destroys the validator.
*/

QRegExpValidator::~QRegExpValidator()
{
}

/*!
    Returns \l Acceptable if \a input is matched by the regular
    expression for this validator, \l Intermediate if it has matched
    partially (i.e. could be a valid match if additional valid
    characters are added), and \l Invalid if \a input is not matched.

    The \a pos parameter is set to the length of the \a input parameter.

    For example, if the regular expression is \bold{\\w\\d\\d}
    (word-character, digit, digit) then "A57" is \l Acceptable,
    "E5" is \l Intermediate, and "+9" is \l Invalid.

    \sa QRegExp::exactMatch()
*/

QValidator::State QRegExpValidator::validate(QString &input, int& pos) const
{
    if (r.exactMatch(input)) {
        return Acceptable;
    } else {
        if (const_cast<QRegExp &>(r).matchedLength() == input.size()) {
            return Intermediate;
        } else {
            pos = input.size();
            return Invalid;
        }
    }
}

/*!
    \property QRegExpValidator::regExp
    \brief the regular expression used for validation
*/

void QRegExpValidator::setRegExp(const QRegExp& rx)
{
    r = rx;
}

#endif

#endif

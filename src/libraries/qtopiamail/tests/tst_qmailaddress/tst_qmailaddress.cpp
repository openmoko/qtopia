/****************************************************************************
**
** This file is part of the Qt Extended Opensource Package.
**
** Copyright (C) 2008 Trolltech ASA.
**
** Contact: Qt Extended Information (info@qtextended.org)
**
** This file may be used under the terms of the GNU General Public License
** version 2.0 as published by the Free Software Foundation and appearing
** in the file LICENSE.GPL included in the packaging of this file.
**
** Please review the following information to ensure GNU General Public
** Licensing requirements will be met:
**     http://www.fsf.org/licensing/licenses/info/GPLv2.html.
**
**
****************************************************************************/

// To detail string differences on comparison failure:
#include "shared/string_difference.h"

#include <QtopiaApplication>
#include <QObject>
#include <QTest>
#include <shared/qtopiaunittest.h>
#include <QMailAddress>


//TESTED_CLASS=QMailAddress
//TESTED_FILES=src/libraries/qtopiamail/qmailmessage.cpp

/*
    This class primarily tests that QMailAddress correctly handles e-mail addresses.
*/
class tst_QMailAddress : public QObject
{
    Q_OBJECT

public:
    tst_QMailAddress();
    virtual ~tst_QMailAddress();

private slots:
    void constructor1_data();
    void constructor1();
    void constructor2_data();
    void constructor2();
    void name();
    void address();
    void isGroup();
    void groupMembers_data();
    void groupMembers();
    void notGroup();
    void toString();
    void isPhoneNumber_data();
    void isPhoneNumber();
    void isEmailAddress_data();
    void isEmailAddress();
    void toStringList_data();
    void toStringList();
    void fromStringList1_data();
    void fromStringList1();
    void fromStringList2();
    void removeComments_data();
    void removeComments();
    void removeWhitespace_data();
    void removeWhitespace();
};

QTEST_APP_MAIN( tst_QMailAddress, QtopiaApplication )
#include "tst_qmailaddress.moc"


tst_QMailAddress::tst_QMailAddress()
{
}

tst_QMailAddress::~tst_QMailAddress()
{
}

//    QMailAddress::QMailAddress(const QString &s);
void tst_QMailAddress::constructor1_data()
{
    QTest::addColumn<bool>("valid_email_address");
    QTest::addColumn<QString>("from");
    QTest::addColumn<QString>("name");
    QTest::addColumn<QString>("address");
    QTest::addColumn<QString>("to_string");

    QTest::newRow("No angle brackets") 
        << true
        << "wizard@oz.com" 
        << "wizard@oz.com" 
        << "wizard@oz.com" 
        << "wizard@oz.com";

    QTest::newRow("preceding comment") 
        << true
        << "(Wizard of Oz) wizard@oz.com" 
        << "(Wizard of Oz) wizard@oz.com" 
        << "(Wizard of Oz) wizard@oz.com" 
        << "(Wizard of Oz) wizard@oz.com";

    QTest::newRow("trailing comment") 
        << true
        << "wizard@oz.com (Wizard of Oz)"
        << "wizard@oz.com (Wizard of Oz)"
        << "wizard@oz.com (Wizard of Oz)"
        << "wizard@oz.com (Wizard of Oz)";

    QTest::newRow("intervening comment") 
        << true
        << "wizard(Wizard of Oz)@oz.com"
        << "wizard(Wizard of Oz)@oz.com"
        << "wizard(Wizard of Oz)@oz.com"
        << "wizard(Wizard of Oz)@oz.com";

    QTest::newRow("No address part") 
        << false
        << "Wizard Of Oz" 
        << "Wizard Of Oz" 
        << "Wizard Of Oz" 
        << "Wizard Of Oz";

    QTest::newRow("Left angle bracket") 
        << true
        << "<wizard@oz.com" 
        << "wizard@oz.com" 
        << "wizard@oz.com" 
        << "wizard@oz.com";

    QTest::newRow("Right angle bracket") 
        << true
        << "wizard@oz.com>" 
        << "wizard@oz.com" 
        << "wizard@oz.com" 
        << "wizard@oz.com";

    QTest::newRow("Two angle brackets") 
        << true
        << "Wizard Of Oz <wizard@oz.com>" 
        << "Wizard Of Oz"
        << "wizard@oz.com" 
        << "Wizard Of Oz <wizard@oz.com>";

    QTest::newRow("Upper-case address characters") 
        << true
        << "Wizard Of Oz <WiZarD@Oz.coM>" 
        << "Wizard Of Oz"
        << "WiZarD@Oz.coM" 
        << "Wizard Of Oz <WiZarD@Oz.coM>";

    QTest::newRow("Quoted name") 
        << true
        << "\"Wizard Of Oz\" <wizard@oz.com>" 
        << "Wizard Of Oz"
        << "wizard@oz.com" 
        << "\"Wizard Of Oz\" <wizard@oz.com>";

    QTest::newRow("Trailing garbage") 
        << true
        << "Wizard Of Oz <wizard@oz.com> crap" 
        << "Wizard Of Oz"
        << "wizard@oz.com" 
        << "Wizard Of Oz <wizard@oz.com>";

    QTest::newRow("Trailing type specifier") 
        << true
        << "Wizard Of Oz <wizard@oz.com>/TYPE=unknown" 
        << "Wizard Of Oz"
        << "wizard@oz.com" 
        << "Wizard Of Oz <wizard@oz.com> /TYPE=unknown";

    QTest::newRow("With whitespace") 
        << true
        << "  \t \"Wizard Of Oz\"\t\t\n \r <wizard@oz.com>  \r\r \t"
        << "Wizard Of Oz"
        << "wizard@oz.com" 
        << "\"Wizard Of Oz\" <wizard@oz.com>";

    QTest::newRow("'(' needs quoting") 
        << true
        << "\"Wizard (Of Oz\" <wizard@oz.com>"
        << "Wizard (Of Oz"
        << "wizard@oz.com"
        << "\"Wizard (Of Oz\" <wizard@oz.com>";

    QTest::newRow("')' needs quoting") 
        << true
        << "\"Wizard) Of Oz\" <wizard@oz.com>"
        << "Wizard) Of Oz"
        << "wizard@oz.com"
        << "\"Wizard) Of Oz\" <wizard@oz.com>";

    QTest::newRow("Comments don't need quoting") 
        << true
        << "Wizard (Of Oz) <wizard@oz.com>"
        << "Wizard (Of Oz)"
        << "wizard@oz.com"
        << "Wizard (Of Oz) <wizard@oz.com>";

    QTest::newRow("Nested Comments don't need quoting") 
        << true
        << "Wizard ((Of) Oz) <wizard@oz.com>"
        << "Wizard ((Of) Oz)"
        << "wizard@oz.com"
        << "Wizard ((Of) Oz) <wizard@oz.com>";

    QTest::newRow("Mismatched Comments need quoting") 
        << true
        << "\"Wizard ((Of Oz)\" <wizard@oz.com>"
        << "Wizard ((Of Oz)"
        << "wizard@oz.com"
        << "\"Wizard ((Of Oz)\" <wizard@oz.com>";

    QTest::newRow("Non-Comments need quoting") 
        << true
        << "\"Wizard )Of Oz(\" <wizard@oz.com>"
        << "Wizard )Of Oz("
        << "wizard@oz.com"
        << "\"Wizard )Of Oz(\" <wizard@oz.com>";

    QTest::newRow("'<' and '>' need quoting") 
        << true
        << "\"Wizard <Of Oz>\" <wizard@oz.com>"
        << "Wizard <Of Oz>"
        << "wizard@oz.com"
        << "\"Wizard <Of Oz>\" <wizard@oz.com>";

    QTest::newRow("'[' and ']' need quoting") 
        << true
        << "\"Wizard [Of Oz]\" <wizard@oz.com>"
        << "Wizard [Of Oz]"
        << "wizard@oz.com"
        << "\"Wizard [Of Oz]\" <wizard@oz.com>";

    QTest::newRow("'@' needs quoting") 
        << true
        << "\"Wizard at SETI@Home\" <wizard@seti.com>"
        << "Wizard at SETI@Home"
        << "wizard@seti.com"
        << "\"Wizard at SETI@Home\" <wizard@seti.com>";

    QTest::newRow("';' and ':' need quoting") 
        << true
        << "\"Wizard;Wizard of Oz:\" <wizard@oz.com>"
        << "Wizard;Wizard of Oz:"
        << "wizard@oz.com"
        << "\"Wizard;Wizard of Oz:\" <wizard@oz.com>";

    QTest::newRow("',' needs quoting") 
        << true
        << "\"Wizard, Of Oz\" <wizard@oz.com>"
        << "Wizard, Of Oz"
        << "wizard@oz.com"
        << "\"Wizard, Of Oz\" <wizard@oz.com>";

    QTest::newRow("'.' needs quoting") 
        << true
        << "\"O. Wizard\" <wizard@oz.com>"
        << "O. Wizard"
        << "wizard@oz.com"
        << "\"O. Wizard\" <wizard@oz.com>";

    /* Honestly, I don't know what to do about this...
    QTest::newRow("'\\' needs quoting") 
        << true
        << "\"Wizard\\Oz\" <wizard@oz.com>"
        << "Wizard\\Oz"
        << "wizard@oz.com"
        << "\"Wizard\\Oz\" <wizard@oz.com>";
    */
}

void tst_QMailAddress::constructor1()
{
    QFETCH( QString, from ); 

    QMailAddress addr(from);
    QTEST( addr.isEmailAddress(), "valid_email_address" );
    QTEST( addr.name(), "name" );
    QTEST( addr.address(), "address" );
    QTEST( addr.toString(), "to_string" );
}

//    QMailAddress::QMailAddress(const QString &name, const QString &addr);
void tst_QMailAddress::constructor2_data()
{
    QTest::addColumn<QString>("arg1");
    QTest::addColumn<QString>("arg2");
    QTest::addColumn<QString>("name");
    QTest::addColumn<QString>("address");
    QTest::addColumn<QString>("to_string");

    QTest::newRow("No name") 
        << QString()
        << "wizard@oz.com"
        << QString()
        << "wizard@oz.com"
        << "wizard@oz.com";

    QTest::newRow("Empty name") 
        << ""
        << "wizard@oz.com"
        << QString()
        << "wizard@oz.com"
        << "wizard@oz.com";

    QTest::newRow("No address") 
        << "Wizard Of Oz"
        << QString()
        << "Wizard Of Oz"
        << QString()
        << "Wizard Of Oz";

    QTest::newRow("Empty address") 
        << "Wizard Of Oz"
        << ""
        << "Wizard Of Oz"
        << QString()
        << "Wizard Of Oz";

    QTest::newRow("Name and address") 
        << "Wizard Of Oz" 
        << "wizard@oz.com"
        << "Wizard Of Oz" 
        << "wizard@oz.com"
        << "Wizard Of Oz <wizard@oz.com>";

    QTest::newRow("Upper-case address characters")
        << "Wizard Of Oz" 
        << "WIzARd@oZ.Com"
        << "Wizard Of Oz" 
        << "WIzARd@oZ.Com"
        << "Wizard Of Oz <WIzARd@oZ.Com>";

    QTest::newRow("Quoted name") 
        << "\"Wizard Of Oz\"" 
        << "wizard@oz.com"
        << "Wizard Of Oz" 
        << "wizard@oz.com"
        << "\"Wizard Of Oz\" <wizard@oz.com>";

    QTest::newRow("With trailing type specifier") 
        << "Wizard Of Oz" 
        << "wizard@oz.com /TYPE=email"
        << "Wizard Of Oz" 
        << "wizard@oz.com"
        << "Wizard Of Oz <wizard@oz.com> /TYPE=email";

    QTest::newRow("'(' needs quoting") 
        << "Wizard (Of Oz"
        << "<wizard@oz.com>"
        << "Wizard (Of Oz"
        << "wizard@oz.com"
        << "\"Wizard (Of Oz\" <wizard@oz.com>";

    QTest::newRow("')' needs quoting") 
        << "Wizard) Of Oz"
        << "<wizard@oz.com>"
        << "Wizard) Of Oz"
        << "wizard@oz.com"
        << "\"Wizard) Of Oz\" <wizard@oz.com>";

    QTest::newRow("Comments don't need quoting") 
        << "Wizard (Of Oz)"
        << "wizard@oz.com"
        << "Wizard (Of Oz)"
        << "wizard@oz.com"
        << "Wizard (Of Oz) <wizard@oz.com>";

    QTest::newRow("Nested Comments don't need quoting") 
        << "Wizard ((Of) Oz)"
        << "wizard@oz.com"
        << "Wizard ((Of) Oz)"
        << "wizard@oz.com"
        << "Wizard ((Of) Oz) <wizard@oz.com>";

    QTest::newRow("Mismatched Comments need quoting") 
        << "Wizard ((Of Oz)"
        << "wizard@oz.com"
        << "Wizard ((Of Oz)"
        << "wizard@oz.com"
        << "\"Wizard ((Of Oz)\" <wizard@oz.com>";

    QTest::newRow("Non-Comments need quoting") 
        << "Wizard )Of Oz("
        << "wizard@oz.com"
        << "Wizard )Of Oz("
        << "wizard@oz.com"
        << "\"Wizard )Of Oz(\" <wizard@oz.com>";

    QTest::newRow("'<' and '>' need quoting") 
        << "Wizard <Of Oz>"
        << "wizard@oz.com"
        << "Wizard <Of Oz>"
        << "wizard@oz.com"
        << "\"Wizard <Of Oz>\" <wizard@oz.com>";

    QTest::newRow("'[' and ']' need quoting") 
        << "Wizard [Of Oz]"
        << "wizard@oz.com"
        << "Wizard [Of Oz]"
        << "wizard@oz.com"
        << "\"Wizard [Of Oz]\" <wizard@oz.com>";

    QTest::newRow("'@' needs quoting") 
        << "Wizard at SETI@Home"
        << "wizard@seti.com"
        << "Wizard at SETI@Home"
        << "wizard@seti.com"
        << "\"Wizard at SETI@Home\" <wizard@seti.com>";

    QTest::newRow("':' and ';' need quoting") 
        << "Wizard:Wizard of Oz;"
        << "wizard@oz.com"
        << "Wizard:Wizard of Oz;"
        << "wizard@oz.com"
        << "\"Wizard:Wizard of Oz;\" <wizard@oz.com>";

    QTest::newRow("',' needs quoting") 
        << "Wizard, Of Oz"
        << "wizard@oz.com"
        << "Wizard, Of Oz"
        << "wizard@oz.com"
        << "\"Wizard, Of Oz\" <wizard@oz.com>";

    QTest::newRow("'.' needs quoting") 
        << "O. Wizard"
        << "wizard@oz.com"
        << "O. Wizard"
        << "wizard@oz.com"
        << "\"O. Wizard\" <wizard@oz.com>";

    /* Honestly, I don't know what to do about this...
    QTest::newRow("'\\' needs quoting") 
        << "Wizard\\Oz"
        << "wizard@oz.com"
        << "Wizard\\Oz"
        << "wizard@oz.com"
        << "\"Wizard\\Oz\" <wizard@oz.com>";
    */
}

void tst_QMailAddress::constructor2()
{
    QFETCH( QString, arg1 );
    QFETCH( QString, arg2 );

    QMailAddress addr(arg1, arg2);
    QTEST( addr.name(), "name" );
    QTEST( addr.address(), "address" );
    QTEST( addr.toString(), "to_string" );
}

void tst_QMailAddress::name()
{
    // Tested-by: constructor1, constructor2
}

void tst_QMailAddress::address()
{
    // Tested-by: constructor1, constructor2
}

void tst_QMailAddress::isGroup()
{
    // Tested-by: groupMembers
}

void tst_QMailAddress::groupMembers_data()
{
    QTest::addColumn<QString>("input");
    QTest::addColumn<QString>("input_name");
    QTest::addColumn<QString>("input_address");
    QTest::addColumn<QString>("name");
    QTest::addColumn<QString>("address");
    QTest::addColumn<QString>("to_string");
    QTest::addColumn<bool>("is_group");
    QTest::addColumn<QStringList>("member_names");
    QTest::addColumn<QStringList>("member_addresses");

    QTest::newRow("Empty")
        << QString()
        << QString()
        << QString()
        << QString()
        << QString()
        << QString()
        << false
        << QStringList()
        << QStringList();

    QTest::newRow("Non-group ctor1")
        << QString("Wizard Of Oz <wizard@oz.com>")
        << QString()
        << QString()
        << QString("Wizard Of Oz")
        << QString("wizard@oz.com")
        << QString("Wizard Of Oz <wizard@oz.com>")
        << false
        << QStringList()
        << QStringList();

    QTest::newRow("Non-group ctor2")
        << QString()
        << QString("Wizard Of Oz")
        << QString("wizard@oz.com")
        << QString("Wizard Of Oz")
        << QString("wizard@oz.com")
        << QString("Wizard Of Oz <wizard@oz.com>")
        << false
        << QStringList()
        << QStringList();

    QTest::newRow("Non-group with comma in name ctor1")
        << QString("\"Wizard, Of Oz\" <wizard@oz.com>")
        << QString()
        << QString()
        << QString("Wizard, Of Oz")
        << QString("wizard@oz.com")
        << QString("\"Wizard, Of Oz\" <wizard@oz.com>")
        << false
        << QStringList()
        << QStringList();

    QTest::newRow("Non-group with comma in name ctor2")
        << QString()
        << QString("\"Wizard, Of Oz\"")
        << QString("wizard@oz.com")
        << QString("Wizard, Of Oz")
        << QString("wizard@oz.com")
        << QString("\"Wizard, Of Oz\" <wizard@oz.com>")
        << false
        << QStringList()
        << QStringList();

    QTest::newRow("Non-group with comma in address ctor1")
        << QString("Wizard Of Oz <wizard(is,allowed here?)@oz.com>")
        << QString()
        << QString()
        << QString("Wizard Of Oz")
        << QString("wizard(is,allowed here?)@oz.com")
        << QString("Wizard Of Oz <wizard(is,allowed here?)@oz.com>")
        << false
        << QStringList()
        << QStringList();

    QTest::newRow("Non-group with comma in address ctor2")
        << QString()
        << QString("Wizard Of Oz")
        << QString("wizard(is,allowed here?)@oz.com")
        << QString("Wizard Of Oz")
        << QString("wizard(is,allowed here?)@oz.com")
        << QString("Wizard Of Oz <wizard(is,allowed here?)@oz.com>")
        << false
        << QStringList()
        << QStringList();

    QTest::newRow("Group-of-zero ctor1")
        << QString("Wizards:;")
        << QString()
        << QString()
        << QString("Wizards")
        << QString()
        << QString("Wizards: ;")
        << true
        << QStringList()
        << QStringList();

    /* It is not possible to create a group of zero via ctor2
    QTest::newRow("Group-of-zero ctor2")
        << ...
    */

    QTest::newRow("Group-of-one ctor1")
        << QString("Wizards: Wizard Of Oz <wizard@oz.com>;")
        << QString()
        << QString()
        << QString("Wizards")
        << QString("Wizard Of Oz <wizard@oz.com>")
        << QString("Wizards: Wizard Of Oz <wizard@oz.com>;")
        << true
        << ( QStringList() << "Wizard Of Oz" )
        << ( QStringList() << "wizard@oz.com" );

    /* It is not possible to create a group of one via ctor2
    QTest::newRow("Group-of-one ctor2")
        << ...
    */

    QTest::newRow("Group ctor1")
        << "Wizard Group:Wizard Of Oz <wizard@oz.com>, Rincewind <wizzard@uu.edu>;" 
        << QString()
        << QString()
        << "Wizard Group"
        << "Wizard Of Oz <wizard@oz.com>, Rincewind <wizzard@uu.edu>" 
        << "Wizard Group: Wizard Of Oz <wizard@oz.com>, Rincewind <wizzard@uu.edu>;"
        << true
        << ( QStringList() << "Wizard Of Oz" << "Rincewind" ) 
        << ( QStringList() << "wizard@oz.com" << "wizzard@uu.edu" );

    QTest::newRow("Group ctor2")
        << QString()
        << "Wizard Group"
        << "Wizard Of Oz <wizard@oz.com>, Rincewind <wizzard@uu.edu>" 
        << "Wizard Group"
        << "Wizard Of Oz <wizard@oz.com>, Rincewind <wizzard@uu.edu>" 
        << "Wizard Group: Wizard Of Oz <wizard@oz.com>, Rincewind <wizzard@uu.edu>;"
        << true
        << ( QStringList() << "Wizard Of Oz" << "Rincewind" ) 
        << ( QStringList() << "wizard@oz.com" << "wizzard@uu.edu" );

    QTest::newRow("Group with quoted member names ctor1")
        << "Wizard Group: \"Wizard Of Oz\" <wizard@oz.com>, \"Rincewind\" <wizzard@uu.edu>;" 
        << QString()
        << QString()
        << "Wizard Group"
        << "\"Wizard Of Oz\" <wizard@oz.com>, \"Rincewind\" <wizzard@uu.edu>" 
        << "Wizard Group: \"Wizard Of Oz\" <wizard@oz.com>, \"Rincewind\" <wizzard@uu.edu>;"
        << true
        << ( QStringList() << "Wizard Of Oz" << "Rincewind" ) 
        << ( QStringList() << "wizard@oz.com" << "wizzard@uu.edu" );

    QTest::newRow("Group with quoted member names ctor2")
        << QString()
        << "Wizard Group"
        << "\"Wizard Of Oz\" <wizard@oz.com>, \"Rincewind\" <wizzard@uu.edu>" 
        << "Wizard Group"
        << "\"Wizard Of Oz\" <wizard@oz.com>, \"Rincewind\" <wizzard@uu.edu>" 
        << "Wizard Group: \"Wizard Of Oz\" <wizard@oz.com>, \"Rincewind\" <wizzard@uu.edu>;"
        << true
        << ( QStringList() << "Wizard Of Oz" << "Rincewind" ) 
        << ( QStringList() << "wizard@oz.com" << "wizzard@uu.edu" );
}

void tst_QMailAddress::groupMembers()
{
    QFETCH( QString, input );
    QFETCH( QString, input_name );
    QFETCH( QString, input_address );

    QMailAddress addr;
    if ( !input.isEmpty() )
        addr = QMailAddress( input );
    else
        addr = QMailAddress( input_name, input_address );

    QTEST( addr.name(), "name" );
    QTEST( addr.address(), "address" );
    QTEST( addr.toString(), "to_string" );
    QTEST( addr.isGroup(), "is_group" );

    QStringList names, addresses;
    foreach (const QMailAddress& member, addr.groupMembers()) {
        names.append(member.name());
        addresses.append(member.address());
    }

    QTEST( names, "member_names" );
    QTEST( addresses, "member_addresses" );
}

void tst_QMailAddress::notGroup()
{
    QString input("\"Wizard: Oz\" <wizard@oz.com>,Rincewind <wizzard@uu.edu>");

    QList<QMailAddress> list(QMailAddress::fromStringList(input));
    QCOMPARE( list.count(), 2 );

    QCOMPARE( list.at(0).isGroup(), false );
    QCOMPARE( list.at(0).name(), QString("Wizard: Oz") );
    QCOMPARE( list.at(0).address(), QString("wizard@oz.com") );

    QCOMPARE( list.at(1).isGroup(), false );
    QCOMPARE( list.at(1).name(), QString("Rincewind") );
    QCOMPARE( list.at(1).address(), QString("wizzard@uu.edu") );
}

void tst_QMailAddress::toString()
{
    // Tested-by: constructor1, constructor2
}

void tst_QMailAddress::isPhoneNumber_data()
{
    QTest::addColumn<QString>("address");
    QTest::addColumn<bool>("phoneNumber");

    QTest::newRow("simple") 
        << "32199000" << true;

    QTest::newRow("legal white space") 
        << "32 19 9000" << true;

    QTest::newRow("area code") 
        << "(07) 3219 9000" << true;

    QTest::newRow("country code") 
        << "+61 7 3219 9000" << true;

    QTest::newRow("mobile") 
        << "0404 444 444" << true;

    QTest::newRow("dashed") 
        << "1800-123-321" << true;

    QTest::newRow("accepted chars") 
        << "+01 2#3 45* 678X90" << true;

    QTest::newRow("extension 1") 
        << "(07) 3219 9000 (x100)" << true;

    QTest::newRow("extension 2") 
        << "(07) 3219 9000 (P100)" << true;

    QTest::newRow("extension 3") 
        << "(07) 3219 9000 w100" << true;

    QTest::newRow("extension 4") 
        << "(07) 3219 9000,100" << true;

    // Perhaps this should be parsed?
    QTest::newRow("alphanumeric") 
        << "1800-Reverse" << false;

    QTest::newRow("email") 
        << "1800@123321" << false;

    QTest::newRow("illegal white space 1") 
        << " 3219\t9000" << false;

    QTest::newRow("illegal white space 2") 
        << " 3219\n9000" << false;

    QTest::newRow("garbage 1") 
        << "[1800 123 321]" << false;

    QTest::newRow("garbage 2") 
        << "1800 123 321:" << false;

    QTest::newRow("garbage 3") 
        << "1800_123_321" << false;

    QTest::newRow("rejected chars 1") 
        << "A" << false;

    QTest::newRow("rejected chars 2") 
        << "@" << false;

    QTest::newRow("rejected chars 3") 
        << "&" << false;
}

void tst_QMailAddress::isPhoneNumber()
{
    QFETCH( QString, address );

    QMailAddress addr = QMailAddress(QString(), address);
    QTEST(addr.isPhoneNumber(), "phoneNumber");
}

void tst_QMailAddress::isEmailAddress_data()
{
    QTest::addColumn<QString>("address");
    QTest::addColumn<bool>("emailAddress");

    QTest::newRow("simple") 
        << "fred@example.net" << true;

    QTest::newRow("white space 1") 
        << " fred@example.net   " << true;

    QTest::newRow("white space 2") 
        << "\t\n  \tfred@example.net\r \n" << true;

    QTest::newRow("multi-part domain") 
        << "fred@mail-machine-1.example.net.au" << true;

    QTest::newRow("multi-part name") 
        << "fred.smith@example.net" << true;

    QTest::newRow("short") 
        << "x@y.zz" << true;

    QTest::newRow("missing mailbox name") 
        << "@example.net" << false;

    QTest::newRow("missing domain") 
        << "fred@" << false;

    QTest::newRow("single part domain") 
        << "fred@example" << false;
}

void tst_QMailAddress::isEmailAddress()
{
    QFETCH( QString, address );

    QMailAddress addr = QMailAddress(QString(), address);
    QTEST(addr.isEmailAddress(), "emailAddress");
}

void tst_QMailAddress::toStringList_data()
{
    QTest::addColumn<QList<QMailAddress> >("address_list");
    QTest::addColumn<QStringList>("string_list");

    QTest::newRow("Empty")
        << QList<QMailAddress>()
        << QStringList();

    QTest::newRow("Single plain address")
        << ( QList<QMailAddress>() 
                << QMailAddress("wizard@oz.com") )
        << ( QStringList() 
                << "wizard@oz.com" );

    QTest::newRow("Single named address")
        << ( QList<QMailAddress>() 
                << QMailAddress("\"Wizard, Of Oz\" <wizard@oz.com>") )
        << ( QStringList() 
                << "\"Wizard, Of Oz\" <wizard@oz.com>" );

    QTest::newRow("Single group address")
        << ( QList<QMailAddress>() 
                << QMailAddress("Wizard Group: \"Wizard, Of Oz\" <wizard@oz.com>, Rincewind <wizzard@uu.edu>;") )
        << ( QStringList() 
                << "Wizard Group: \"Wizard, Of Oz\" <wizard@oz.com>, Rincewind <wizzard@uu.edu>;" );

    QTest::newRow("Multiple named addressses")
        << ( QList<QMailAddress>() 
                << QMailAddress("\"Wizard, Of Oz\" <wizard@oz.com>")
                << QMailAddress("Rincewind <wizzard@uu.edu>") )
        << ( QStringList() 
                << "\"Wizard, Of Oz\" <wizard@oz.com>"
                << "Rincewind <wizzard@uu.edu>" );

    QTest::newRow("Multiple group addresses")
        << ( QList<QMailAddress>() 
                << QMailAddress("Wizard Group: \"Wizard, Of Oz\" <wizard@oz.com>, Rincewind <wizzard@uu.edu>;")
                << QMailAddress("Witch Group: Wicked Witch (East) <eastwitch@oz.com>, \"Wicked Witch, South\" <southwitch@oz.com>;") )
        << ( QStringList() 
                << "Wizard Group: \"Wizard, Of Oz\" <wizard@oz.com>, Rincewind <wizzard@uu.edu>;"
                << "Witch Group: Wicked Witch (East) <eastwitch@oz.com>, \"Wicked Witch, South\" <southwitch@oz.com>;" );

    QTest::newRow("Multiple mixed addresses")
        << ( QList<QMailAddress>() 
                << QMailAddress("Wizard Group: \"Wizard, Of Oz\" <wizard@oz.com>, Rincewind <wizzard@uu.edu>;")
                << QMailAddress("Dorothy <dot2000@kansas.net>")
                << QMailAddress("Witch Group: Wicked Witch (East) <eastwitch@oz.com>, \"Wicked Witch, South\" <southwitch@oz.com>;") )
        << ( QStringList() 
                << "Wizard Group: \"Wizard, Of Oz\" <wizard@oz.com>, Rincewind <wizzard@uu.edu>;"
                << "Dorothy <dot2000@kansas.net>"
                << "Witch Group: Wicked Witch (East) <eastwitch@oz.com>, \"Wicked Witch, South\" <southwitch@oz.com>;" );
}

void tst_QMailAddress::toStringList()
{
    QFETCH( QList<QMailAddress>, address_list );
    QTEST( QMailAddress::toStringList(address_list), "string_list");
}

void tst_QMailAddress::fromStringList1_data()
{
    QTest::addColumn<QString>("string_list");
    QTest::addColumn<QList<QMailAddress> >("address_list");

    QTest::newRow("Empty")
        << QString()
        << QList<QMailAddress>();

    QTest::newRow("Single plain address")
        << "wizard@oz.com"
        << ( QList<QMailAddress>() 
                << QMailAddress("wizard@oz.com") );

    QTest::newRow("Single named address")
        << "\"Wizard, Of Oz\" <wizard@oz.com>"
        << ( QList<QMailAddress>() 
                << QMailAddress("\"Wizard, Of Oz\" <wizard@oz.com>") );

    QTest::newRow("Single group address")
        << "Wizard Group: \"Wizard, Of Oz\" <wizard@oz.com>, Rincewind <wizzard@uu.edu>;"
        << ( QList<QMailAddress>() 
                << QMailAddress("Wizard Group: \"Wizard, Of Oz\" <wizard@oz.com>, Rincewind <wizzard@uu.edu>;") );

    QTest::newRow("Multiple plain addressses, comma-separated")
        << ( QStringList() 
                << "wizard@oz.com"
                << "wizzard@uu.edu" ).join(",")
        << ( QList<QMailAddress>() 
                << QMailAddress("wizard@oz.com")
                << QMailAddress("wizzard@uu.edu") );

    QTest::newRow("Multiple plain addressses, semicolon-separated")
        << ( QStringList() 
                << "wizard@oz.com"
                << "wizzard@uu.edu" ).join(";")
        << ( QList<QMailAddress>() 
                << QMailAddress("wizard@oz.com")
                << QMailAddress("wizzard@uu.edu") );

    QTest::newRow("Multiple plain addressses, whitespace-separated")
        << ( QStringList() 
                << "wizard@oz.com"
                << "wizzard@uu.edu" ).join(" ")
        << ( QList<QMailAddress>() 
                << QMailAddress("wizard@oz.com")
                << QMailAddress("wizzard@uu.edu") );

    QTest::newRow("Multiple named addressses, comma-separated")
        << ( QStringList() 
                << "\"Wizard, Of Oz\" <wizard@oz.com>"
                << "Rincewind <wizzard@uu.edu>" ).join(",")
        << ( QList<QMailAddress>() 
                << QMailAddress("\"Wizard, Of Oz\" <wizard@oz.com>")
                << QMailAddress("Rincewind <wizzard@uu.edu>") );

    QTest::newRow("Multiple named addressses, semicolon-separated")
        << ( QStringList() 
                << "\"Wizard, Of Oz\" <wizard@oz.com>"
                << "Rincewind <wizzard@uu.edu>" ).join(";")
        << ( QList<QMailAddress>() 
                << QMailAddress("\"Wizard, Of Oz\" <wizard@oz.com>")
                << QMailAddress("Rincewind <wizzard@uu.edu>") );

    QTest::newRow("Multiple named addressses, whitespace-separated")
        << ( QStringList() 
                << "\"Wizard, Of Oz\" <wizard@oz.com>"
                << "Rincewind <wizzard@uu.edu>" ).join(" ")
        << ( QList<QMailAddress>() 
                << QMailAddress("\"Wizard, Of Oz\" <wizard@oz.com>")
                << QMailAddress("Rincewind <wizzard@uu.edu>") );

    QTest::newRow("Multiple group addresses, comma-separated")
        << ( QStringList() 
                << "Wizard Group: \"Wizard, Of Oz\" <wizard@oz.com>, Rincewind <wizzard@uu.edu>;"
                << "Witch Group: Wicked Witch (East) <eastwitch@oz.com>, \"Wicked Witch, South\" <southwitch@oz.com>;" ).join(",")
        << ( QList<QMailAddress>() 
                << QMailAddress("Wizard Group: \"Wizard, Of Oz\" <wizard@oz.com>, Rincewind <wizzard@uu.edu>;")
                << QMailAddress("Witch Group: Wicked Witch (East) <eastwitch@oz.com>, \"Wicked Witch, South\" <southwitch@oz.com>;") );

    QTest::newRow("Multiple group addresses, semicolon-separated")
        << ( QStringList() 
                << "Wizard Group: \"Wizard, Of Oz\" <wizard@oz.com>, Rincewind <wizzard@uu.edu>;"
                << "Witch Group: Wicked Witch (East) <eastwitch@oz.com>, \"Wicked Witch, South\" <southwitch@oz.com>;" ).join(";")
        << ( QList<QMailAddress>() 
                << QMailAddress("Wizard Group: \"Wizard, Of Oz\" <wizard@oz.com>, Rincewind <wizzard@uu.edu>;")
                << QMailAddress("Witch Group: Wicked Witch (East) <eastwitch@oz.com>, \"Wicked Witch, South\" <southwitch@oz.com>;") );

    QTest::newRow("Multiple group addresses, whitespace-separated")
        << ( QStringList() 
                << "Wizard Group: \"Wizard, Of Oz\" <wizard@oz.com>, Rincewind <wizzard@uu.edu>;"
                << "Witch Group: Wicked Witch (East) <eastwitch@oz.com>, \"Wicked Witch, South\" <southwitch@oz.com>;" ).join(" ")
        << ( QList<QMailAddress>() 
                << QMailAddress("Wizard Group: \"Wizard, Of Oz\" <wizard@oz.com>, Rincewind <wizzard@uu.edu>;")
                << QMailAddress("Witch Group: Wicked Witch (East) <eastwitch@oz.com>, \"Wicked Witch, South\" <southwitch@oz.com>;") );

    QTest::newRow("Multiple mixed addresses, comma-separated")
        << ( QStringList() 
                << "Wizard Group: \"Wizard, Of Oz\" <wizard@oz.com>, Rincewind <wizzard@uu.edu>;"
                << "gandalf@whitewizard.org"
                << "Dorothy <dot2000@kansas.net>"
                << "Witch Group: Wicked Witch (East) <eastwitch@oz.com>, \"Wicked Witch, South\" <southwitch@oz.com>;" ).join(",")
        << ( QList<QMailAddress>() 
                << QMailAddress("Wizard Group: \"Wizard, Of Oz\" <wizard@oz.com>, Rincewind <wizzard@uu.edu>;")
                << QMailAddress("gandalf@whitewizard.org")
                << QMailAddress("Dorothy <dot2000@kansas.net>")
                << QMailAddress("Witch Group: Wicked Witch (East) <eastwitch@oz.com>, \"Wicked Witch, South\" <southwitch@oz.com>;") );

    QTest::newRow("Multiple mixed addresses, semicolon-separated")
        << ( QStringList() 
                << "Wizard Group: \"Wizard, Of Oz\" <wizard@oz.com>, Rincewind <wizzard@uu.edu>;"
                << "gandalf@whitewizard.org"
                << "Dorothy <dot2000@kansas.net>"
                << "Witch Group: Wicked Witch (East) <eastwitch@oz.com>, \"Wicked Witch, South\" <southwitch@oz.com>;" ).join(";")
        << ( QList<QMailAddress>() 
                << QMailAddress("Wizard Group: \"Wizard, Of Oz\" <wizard@oz.com>, Rincewind <wizzard@uu.edu>;")
                << QMailAddress("gandalf@whitewizard.org")
                << QMailAddress("Dorothy <dot2000@kansas.net>")
                << QMailAddress("Witch Group: Wicked Witch (East) <eastwitch@oz.com>, \"Wicked Witch, South\" <southwitch@oz.com>;") );

    QTest::newRow("Multiple mixed addresses, whitespace-separated")
        << ( QStringList() 
                << "Wizard Group: \"Wizard, Of Oz\" <wizard@oz.com>, Rincewind <wizzard@uu.edu>;"
                << "gandalf@whitewizard.org"
                << "Dorothy <dot2000@kansas.net>"
                << "Witch Group: Wicked Witch (East) <eastwitch@oz.com>, \"Wicked Witch, South\" <southwitch@oz.com>;" ).join(" ")
        << ( QList<QMailAddress>() 
                << QMailAddress("Wizard Group: \"Wizard, Of Oz\" <wizard@oz.com>, Rincewind <wizzard@uu.edu>;")
                << QMailAddress("gandalf@whitewizard.org")
                << QMailAddress("Dorothy <dot2000@kansas.net>")
                << QMailAddress("Witch Group: Wicked Witch (East) <eastwitch@oz.com>, \"Wicked Witch, South\" <southwitch@oz.com>;") );
}

void tst_QMailAddress::fromStringList1()
{
    QFETCH( QString, string_list );
    QTEST( QMailAddress::fromStringList(string_list), "address_list");
}

void tst_QMailAddress::fromStringList2()
{
    // Tested-by: fromStringList1
}

void tst_QMailAddress::removeComments_data()
{
    QTest::addColumn<QString>("input");
    QTest::addColumn<QString>("output");

    QTest::newRow("Empty")
        << QString()
        << QString();

    QTest::newRow("No comments")
        << "\"Wizard, Of Oz\" <wizard@oz.com>"
        << "\"Wizard, Of Oz\" <wizard@oz.com>";

    QTest::newRow("Leading comment")
        << "(comment) \"Wizard, Of Oz\" <wizard@oz.com>"
        << "\"Wizard, Of Oz\" <wizard@oz.com>";

    QTest::newRow("Trailing comment")
        << "\"Wizard, Of Oz\" <wizard@oz.com> (comment)"
        << "\"Wizard, Of Oz\" <wizard@oz.com>";

    QTest::newRow("Interspersed comments")
        << "\"Wizard, Of Oz\"(comment) <wizard@(comment)oz.com>"
        << "\"Wizard, Of Oz\" <wizard@oz.com>";

    QTest::newRow("Nested comments")
        << "\"Wizard, Of Oz\"(comment(comment)) <wizard((comment)comment)@oz.com>"
        << "\"Wizard, Of Oz\" <wizard@oz.com>";
}

void tst_QMailAddress::removeComments()
{
    QFETCH( QString, input );
    QTEST( QMailAddress::removeComments(input), "output" );
}

void tst_QMailAddress::removeWhitespace_data()
{
    QTest::addColumn<QString>("input");
    QTest::addColumn<QString>("output");

    QTest::newRow("Empty")
        << QString()
        << QString();

    QTest::newRow("No whitespace")
        << "wizard@oz.com"
        << "wizard@oz.com";

    QTest::newRow("Leading whitespace")
        << "\t\n wizard@oz.com"
        << "wizard@oz.com";

    QTest::newRow("Trailing whitespace")
        << "wizard@oz.com   \t  "
        << "wizard@oz.com";

    QTest::newRow("Interspersed whitespace")
        << "wizard @\n\toz . com"
        << "wizard@oz.com";

    QTest::newRow("Quoted whitespace")
        << "\"wizard \" @ \"oz.com\t\n\""
        << "\"wizard \"@\"oz.com\t\n\"";

    QTest::newRow("Comment whitespace")
        << "wizard(Of Oz) @ oz.com((\t))"
        << "wizard(Of Oz)@oz.com((\t))";
}

void tst_QMailAddress::removeWhitespace()
{
    QFETCH( QString, input );
    QTEST( QMailAddress::removeWhitespace(input), "output" );
}


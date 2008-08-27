/***************************************************************************
**
** Copyright (C) 2008 Openmoko Inc.
**
** This software is licensed under the terms of the GNU General Public
** License (GPL) version 2 or at your option any later version.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
***************************************************************************/

/*
 * Test SMS PDU parsing
 */

#include <QtopiaApplication>
#include <QtTest/QtTest>

#include <qtopiaphone/qsmsmessage.h>

class TestPDUParsing : public QObject {
    Q_OBJECT
private slots:
    void testPDUParsing_data();
    void testPDUParsing();
};

void TestPDUParsing::testPDUParsing_data()
{
    QTest::addColumn<QByteArray>("pdu");
    QTest::addColumn<QString>("text");
    QTest::addColumn<QString>("sender");

    QTest::newRow("1") << QByteArray::fromHex("07918167830071F1040BD0C7F7FBCC2E0300008080203201208078D0473BED2697D9F3B20E442DCFE9A076793E0F9FCBA07B9A8E0691C3EEF41C0D1AA3C3F2F0985E96CF75A00EE301E22C1C2C109B217781642E50B87E76816433DD0C066A81E60CB70B347381C2F5B30B") << QString::fromUtf8("Påmindelse: Test message with danish characters: æøå ÆØÅ, lør. 2. aug. 23:30 - søn. 3. aug.") << QString::fromLatin1("Google");
    QTest::newRow("2") << QByteArray::fromHex("0791447758100650040DD0F334FC1CA6970100008080312170224008D4F29CDE0EA7D9") << QString::fromLatin1("Testmail") << QString::fromLatin1("sipgate@");
    QTest::newRow("3") << QByteArray::fromHex("0791446742949940040BD0C7F7FBCC2E0300008080919075106935D2723BED2697E53A10BD3CA787400010B55E0605EB67502C078AC1743059B80D6A8162311D4C166E8350D7B77C9D02") << QString::fromLatin1("Reminder: test! @ Tue Aug 19 10:02am - 11:02am (Work)") << QString::fromLatin1("Google");
    QTest::newRow("4") << QByteArray::fromHex("07918167830071F1040BD0C7F7FBCC2E030000808010800120804AD0473BED2697D9F3B20E644CCBDBE136835C6681CCF2B20B147381C2F5B30B04C3E96630500B1483E96030501A34CDB7C5E9B71B847AB2CB2062987D0E87E5E414") << QString::fromUtf8("Påmindelse: Firmamøde, fre. 1. aug. 08:30 - 10:00 i Symbion (Ole Dalgaard)") << QString("Google");
    QTest::newRow("5") << QByteArray::fromHex("07919471071600000412D0D4B03C6D6E25DDE63700008080221233758056B2970C040ABAE5757319947683C86539A858D5816438D0B8ECA683E0F237A89976D7E9651068F99ED3CB6EF67B5E0611CBF4709A9D769BDFA0BA9B5E96EB402B5A2E76BBC564B4598D367301") << QString::fromLatin1("2/2  Anrufe in der EU: 28 cent pro Minute  Kostenlose Detailinfo unter: +491771243543.") << QString::fromLatin1("Tarif-Info");
    QTest::newRow("6") << QByteArray::fromHex("079194710756040000099194719719F14100808022029153808CD37419840E8BCB6E90BB5C2F83826E79DD5CD6811433D0142E0F8FD165C5758F6697DDA069BA0C5AD172B1DB2D97C3C562B61BADA648B641C1FA9C1D76934141F73C7C2ED3CB783AA89DA68354206A784E") << QString::fromUtf8("Sie haben neue Anrufe: \n3 Sprache\nWählen Sie +49177998116745\nIm Ausland Ansagetext mit * Tas") << QString::fromLatin1("+491779911");
    QTest::newRow("7") << QByteArray::fromHex("07918896532430280406918816880000806051606514235FC3309B0D42BEDB6590380F22A7C3ECB4FB0CE2AD7C20DEF85D77D3E579D0F84D2E836839900FC403C1D16F7719E47E837CA01D681866B341ECF738CC06A9EB733A889C0EB341ECF738CC06C1D16F7719E47EBB") << QString::fromLatin1("Call Home by dialing <+> <country code 49 > < phone no > ; Call local just dial local phone no") << QString::fromLatin1("+886188");
    QTest::newRow("9") << QByteArray::fromHex("0791947107560400000C9188267815940700008080705115818068CC325D4F2F83826E79DD5C06BDD1EE32A8194EB3C56F7CCB191EA3E5E9319AAE5BE1703619EE568BD1723798027673C1702E180E14ABE96A314531ED06ADDF737AD9CD7ECFCB72D0B42CB7A7C7655012") << QString::fromLatin1("Letzte Anrufe ohne Mailbox-Nachricht:+886287514970\n07.08.08 15:51\nEin kostenloser Service I") << QString::fromLatin1("+886287514970");


}

void TestPDUParsing::testPDUParsing()
{
    QFETCH(QByteArray, pdu);
    QFETCH(QString, text);
    QFETCH(QString, sender);

    QSMSMessage message = QSMSMessage::fromPdu(pdu);
    QCOMPARE(message.text(), text);
    QCOMPARE(message.sender(), sender);
}

QTEST_MAIN(TestPDUParsing)
#include "pdutest.moc"

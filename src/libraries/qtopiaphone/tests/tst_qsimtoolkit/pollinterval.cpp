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

#include "tst_qsimtoolkit.h"

#ifndef SYSTEMTEST

// Test encoding and decoding of EVENT DOWNLOAD envelopes based on the
// Test encoding and decoding of POLL INTERVAL commands based on the
// GCF test strings in GSM 51.010, section 27.22.4.6 - POLL INTERVAL
void tst_QSimToolkit::testEncodePollInterval_data()
{
    QSimToolkitData::populateDataPollInterval();
}
void tst_QSimToolkit::testEncodePollInterval()
{
    QFETCH( QByteArray, data );
    QFETCH( QByteArray, resp );
    QFETCH( int, resptype );
    QFETCH( int, duration );
    QFETCH( int, options );

    // Output a dummy line to give some indication of which test we are currently running.
    qDebug() << "";

    // Check that the command PDU can be parsed correctly.
    QSimCommand decoded = QSimCommand::fromPdu(data);
    QVERIFY( decoded.type() == QSimCommand::PollInterval );
    QVERIFY( decoded.destinationDevice() == QSimCommand::ME );
    QCOMPARE( (int)decoded.duration(), duration );

    // Check that the original command PDU can be reconstructed correctly.
    QByteArray encoded = decoded.toPdu( (QSimCommand::ToPduOptions)options );
    QCOMPARE( encoded, data );

    // Check that the terminal response PDU can be parsed correctly.
    QSimTerminalResponse decodedResp = QSimTerminalResponse::fromPdu(resp);
    QVERIFY( data.contains( decodedResp.commandPdu() ) );
    QVERIFY( decodedResp.duration() == 20000 );
    QVERIFY( decodedResp.result() == (QSimTerminalResponse::Result)resptype );
    QVERIFY( decodedResp.causeData().isEmpty() );
    QVERIFY( decodedResp.cause() == QSimTerminalResponse::NoSpecificCause );

    // Check that the original terminal response PDU can be reconstructed correctly.
    QCOMPARE( decodedResp.toPdu(), resp );
}

// Test that POLL INTERVAL commands can be properly delivered to a client
// application and that the client application can respond appropriately.
void tst_QSimToolkit::testDeliverPollInterval_data()
{
    QSimToolkitData::populateDataPollInterval();
}
void tst_QSimToolkit::testDeliverPollInterval()
{
    QFETCH( QByteArray, data );
    QFETCH( QByteArray, resp );
    QFETCH( int, resptype );
    QFETCH( int, duration );
    QFETCH( int, options );

    Q_UNUSED(resptype);

    // Output a dummy line to give some indication of which test we are currently running.
    qDebug() << "";

    // Clear the client/server state.
    server->clear();
    deliveredCommand = QSimCommand();

    // Compose and send the command.
    QSimCommand cmd;
    cmd.setType( QSimCommand::PollInterval );
    cmd.setDuration( duration );
    server->emitCommand( cmd );

    // Wait for the command to arrive in the client.
    QVERIFY( QFutureSignal::wait( this, SIGNAL(commandSeen()), 100 ) );

    // Verify that the command was delivered exactly as we asked.
    QVERIFY( deliveredCommand.type() == cmd.type() );
    QVERIFY( deliveredCommand.duration() == cmd.duration() );
    QCOMPARE( deliveredCommand.toPdu( (QSimCommand::ToPduOptions)options ), data );

    // The response should have been sent immediately.
    QCOMPARE( server->responseCount(), 1 );
    QCOMPARE( server->envelopeCount(), 0 );
    QCOMPARE( server->lastResponse(), resp );
}

#endif // !SYSTEMTEST

// Populate data-driven tests for POLL INTERVAL from the GCF test cases
// in GSM 51.010, section 27.22.4.6.
void QSimToolkitData::populateDataPollInterval()
{
    QTest::addColumn<QByteArray>("data");
    QTest::addColumn<QByteArray>("resp");
    QTest::addColumn<int>("resptype");
    QTest::addColumn<int>("duration");
    QTest::addColumn<int>("options");

    static unsigned char const data_1_1_1[] =
        {0xD0, 0x0D, 0x81, 0x03, 0x01, 0x03, 0x00, 0x82, 0x02, 0x81, 0x82, 0x84,
         0x02, 0x01, 0x14};
    static unsigned char const resp_1_1_1[] =
        {0x81, 0x03, 0x01, 0x03, 0x00, 0x82, 0x02, 0x82, 0x81, 0x83, 0x01, 0x00,
         0x84, 0x02, 0x01, 0x14};
    QTest::newRow( "POLL INTERVAL 1.1.1 - GCF 27.22.4.6" )
        << QByteArray( (char *)data_1_1_1, sizeof(data_1_1_1) )
        << QByteArray( (char *)resp_1_1_1, sizeof(resp_1_1_1) )
        << 0x0000       // Command performed successfully
        << 20000        // Duration
        << (int)( QSimCommand::NoPduOptions );
}

include(tst_qobexauthenticationresponse_p.pro)
TEMPLATE=app
CONFIG+=qtopia unittest


QTOPIA*=comm
MODULES*=openobex
get_sourcepath(qtopiacomm)

TARGET=tst_qobexauthenticationresponse_p

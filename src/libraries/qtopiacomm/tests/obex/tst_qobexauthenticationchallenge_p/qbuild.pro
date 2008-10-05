include(tst_qobexauthenticationchallenge_p.pro)
TEMPLATE=app
CONFIG+=qtopia unittest


QTOPIA*=comm
MODULES*=openobex
get_sourcepath(qtopiacomm)

TARGET=tst_qobexauthenticationchallenge_p

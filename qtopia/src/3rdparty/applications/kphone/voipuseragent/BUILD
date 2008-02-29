Instructions for building the kphone-derived code from a Qtopia Free Edition
package.

Unpack the Qtopia tarball then change to the directory it creates

    tar zxf /path/to/qtopia-free-src-2.2.0.tar.gz
    cd qtopia-free-2.2.0

Tell configure that it can build Phone Edition. It can't really (because
there's missing files) but we can build part of the Phone Edition.

    echo "free pda phone" >qtopia/.configureoptions

Configure for phone, enabling voip. Feel free to use a different commandline
(eg. -xplatform linux-sharp-g++) but be sure to pass -edition phone and
-enable-voip or you won't be able to build the kphone-derived code.

    ./configure -qpe '-edition phone -enable-voip'
    make

The make process will fail while building Qtopia. That's to be expected
because you can't actually build Phone Edition from a free package.
Once the make stops, we can go and manually build the libs required for
the kphone-derived code.

    . ./setQpeEnv
    cd $QPEDIR/src/libraries/qtopia
    make
    cd $QPEDIR/src/libraries/qtopia1
    make
    cd $QPEDIR/src/libraries/qtopia2
    make
    cd $QPEDIR/src/3rdparty/libraries/openssl/crypto
    make
    cd $QPEDIR/src/3rdparty/libraries/openssl/ssl
    make
    cd $QPEDIR/src/3rdparty/applications/kphone/dissipate2
    make
    cd $QPEDIR/src/3rdparty/applications/kphone/gsm
    make
    cd $QPEDIR/src/3rdparty/applications/kphone/ilbc
    make
    cd $QPEDIR/src/3rdparty/applications/kphone/voipuseragent
    make

You should now have the Phone Edition version of libqpe/libqtopia/libqtopia2,
the openssl libsm, the kphone libs and the voipuseragent app.

voipuseragent is a gui-less app that is controlled through QCop.


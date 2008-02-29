#!/usr/bin/perl 
#
# Configure to build the Qtopia Environment / Qtopia Desktop / Qtopia SDKs
#
# Copyright 2003 Trolltech AS.  All rights reserved.
#
# This script is designed to create the required  makefiles for different
# qtopia products from the various qtopia packages under either Win32 or Unix.
#
# Currently configuring under Unix should still be using the ./configure shell
# script, but may eventually be replaced with this script which will be
# renamed to configure when that takes place.
#
# For windows, configure.pl is currently the configuration tool to use.
# It may be able to be converted to an exe using perl2exe to avoid
# a perl dependancy if we so desire.
# 
#  some example usage:
#       perl configure.pl -host unix -debug -v -errorlog \error.txt -product qtopiadesktop
#       perl configure.pl -host win32 -release -product sdk
#



#
# External perl modules
#
use File::Basename;
use File::Find;
use Getopt::Long;
use Cwd;



#
# Global variables, flags etc
#
my $MAKE;
my $COPY;
my $MKDIR;
my $DIR_SEPARATOR; # what type of path separator do we have
my $CMD_SWITCH_CHAR; # what char do we use for command switches
my $QPEDIR = $ENV{'QPEDIR'};
my $QTDIR = $ENV{'QTDIR'};
my $QMAKE = "qmake";
my $QMAKE_FLAGS = "";
my $TMAKE = $ENV{'TMAKE'};
my $TMAKE_FLAGS = "";
my $TMAKEDIR = $ENV{'TMAKEDIR'};
my $TMAKEPATH = $ENV{'TMAKEPATH'};
my $objectDirFlags = "";
my $objectDir = "";
my $tempDir = "";
my $binType = "shared";
my $releaseType = "release";
my $qtVersion = 2;
my $mangledBaseOutDir;
my $OBJSUBDIR = "";
my $MAKEFILE = "Makefile"; # what is the name of the make file to generate
my $TOPLEVEL_MAKEFILE;
my @components = qw(); # What components are to be compiled
my @componentDirs;
my $vcDspFlags = "";
my @targetSpecs;
my $verbose = "true";
my $errorOutput = "";
my $errorOutputAppend = "";
my @validPlatformList = qw();



#
# Exclude lists
#

# Components not to compile under windows
my @win32Excludes = qw (
	./src/3rdparty/applications/embeddedkonsole/embeddedkonsole.pro
	./src/3rdparty/applications/keypebble/keypebble.pro
	./src/3rdparty/tools/atd/atd.pro
	./src/applications/mediaplayer/mediaplayer.pro
	./src/applications/mediarecorder/mediarecorder.pro
	./src/3rdparty/libraries/gsm/gsm.pro
	./src/3rdparty/libraries/libavcodec/libavcodec.pro
	./src/3rdparty/libraries/libavformat/libavformat.pro
	./src/3rdparty/plugins/codecs/libmpeg3/libmpeg3.pro
	./src/3rdparty/plugins/codecs/libffmpeg/libffmpeg.pro
	./src/3rdparty/plugins/obex/obex.pro
	./src/3rdparty/plugins/obex/openobex/openobex.pro
	./src/3rdparty/plugins/textcodecs/jp/jp.pro
	./src/plugins/network/dialup/dialup.pro
	./src/plugins/codecs/wavplugin/wavplugin.pro
	./src/plugins/codecs/wavrecord/wavrecord.pro
	./src/applications/sysinfo/sysinfo.pro
	./src/games/go/go.pro
	./src/games/mindbreaker/mindbreaker.pro
	./src/settings/light-and-power/light-and-power.pro
	./src/settings/systemtime/systemtime.pro
	./src/settings/qipkg/qipkg.pro
	./src/tools/quickexec/quickexec.pro
	./src/single/single.pro
);

# Components not to compile under unix
my @unixExcludes = qw (
	./src/qtopiadesktop/systemtray/systemtray.pro
	./src/qtopiadesktop/externalplugins/outlook/outlook.pro
	./src/qtopiadesktop/externalplugins/outlook/calendar/calendar.pro
	./src/qtopiadesktop/externalplugins/outlook/contacts/contacts.pro
	./src/qtopiadesktop/externalplugins/outlook/todolist/todolist.pro
	./src/qtopiadesktop/dcopserver/dcopserver.pro
);

# Components which are always excluded
my @excludedComponents = qw (
	./src/libraries/qipkg/qipkg.pro
	./src/libraries/qsoap/qsoap.pro
	./src/libraries/qtopiadb/qtopiadb.pro
	./src/libraries/sql/sql.pro
	./src/3rdparty/plugins/codecs/libflash/libflash.pro
	./src/3rdparty/plugins/codecs/libmad/libmad.pro
	./src/plugins/calculator/conversion/conversion.pro
	./src/plugins/calculator/fraction/fraction.pro
	./src/plugins/sqldrivers/sqlite/sqlite.pro
	./src/applications/imageviewer2/imageviewer2.pro
	./src/applications/tableviewer/tableviewer.pro
	./src/settings/calibrate/calibrate.pro
	./src/settings/dictionary/dictionary.pro
	./src/tools/performance/performance.pro
	./src/qtopiadesktop/book/book.pro
	./src/qtopiadesktop/book/en/en.pro
	./src/qtopiadesktop/book/de/de.pro
	./src/qtopiadesktop/cppclient/cppclient.pro
	./src/qtopiadesktop/externalplugins/test/test.pro
	./src/qtopiadesktop/externalplugins
	./src/qtopiadesktop/externalplugins/outlook
	./src/qtopiadesktop/externalplugins/palm
	./src/qtopiadesktop/3rdparty
);

# Package files which are always excluded
my @excludedPackages = qw (
	./examples/application/example.control
	./examples/qpepim-addressbook/qpepim-abexample.control
	./help/de/qpe-help-de.control
	./help/helpbrowser.control
	./help/html/qpe-help-en.control
	./src/libraries/qipkg/C/busybox-0.60.3/debian/control
);


#
# Anomalies:
#
# potential depenancy order problem:
#	3rdparty/plugins/obex
#   - depends on -
#	3rdparty/plugins/obex/openobex
# solution:
#	3rdparty/plugins/obex/openobex
#   - should be moved to -
#	3rdparty/libraries/openobex
# missing, Makefile.linux references these but they do not exist:
#	$(QPEDIR)/help/qpe-help-en.control \
#	$(QPEDIR)/services/qpe-service-Receive.control \
#



#
# debugMsg()
#	Print a message if user requested to see extra messages
#
# Arguments:
#	String to print out if -v verbose option is set
#
sub debugMsg {
    if ( $verbose eq "true" ) {
	print @_;
	print "\n";
    }
}



#
# runCmd()
#	Executes a system command and in verbose mode prints the command being executed
#
# Arguments:
#	$cmdLine : Command line to execute
#
sub runCmd {
    my ( $cmdLine ) = @_;
    debugMsg("Running command line: $cmdLine");
    return `$cmdLine`;
}



#
# hostSlashify()
#	Ensure that the slashes of file name suit host
#
# Parameters: 
#	$file : File name to convert
#
sub hostSlashify {
    my ( $result ) = @_;
    if ( $opt_host eq "win32" ) {
	$result =~ s/\//\\/g;
    }
    return $result;
}



#
# perlSlashify()
#	Ensure that the slashes of file name / environment variable suit perl
#
# Parameters: 
#	$file : File name to convert
#
sub perlSlashify {
    my ( $result ) = @_;
    $result =~ s/\\/\//g;
    return $result;
}



#
# subtractArrays()
#	Subtract one array from another
#
# Parameters:
#	@origList : The array of strings to subtract from
#       @excludeList : The array of strings to remove from origList
#
# Returns a new array with the contents set to @origList excluding any items matching from @excludedList
#
sub subtractArrays {
    my ( $origList, $excludeList ) = @_;
    my @result = qw();
    my $foundflag;
    foreach $file (@$origList) {
	$foundflag = "false";
	foreach $profile ( @$excludeList ) {
	    if ( $profile eq "$file" ) {
		$foundflag = "true";
		last;
	    }
	}
	if ( $foundflag eq "false" ) {
	    push ( @result, $file );
	}
    }
    return @result;
}



#
# wanted()
#	Helper function needed for findFiles() function below
#
my $global_regex;
my @global_found;
sub wanted {
    my $file = $File::Find::name;
    if ( $file =~ $global_regex ) {
	push( @global_found, $file );
    }
}



#
# findFiles()
#	Find file in a given directory matching that match the given pattern
#
# Parameters :
#	$regex - file name regex to search on
#	$dir - directory to search and its subdirs to look for files in
#
# Returns array of matching files with the given $ext in the $dir directory and its subdirs
#
sub findFiles {
    my ( $ext, $dir ) = @_;
    $global_regex = $ext;
    @global_found = qw();
    find( \&wanted, $dir );
    return @global_found;
}



#
# grepFiles()
#	Searches array for items matching the regex and returns them
#
# Parameters:
#	$regex - file name regex to search on
#	@filelist - array of names to search
#
# Returns a new array of the matching items
#
sub grepFiles {
    my ( $regex, @filelist ) = @_;
    my @result = qw();
    foreach $file (@filelist) {
	if ( $file =~ /$regex/ ) {
	    push( @result, $file );
	}
    }
    return @result;
}



#
# reverseGrepFiles()
#	Searches array for items not matching the regex and returns them
#
# Parameters:
#	$regex - file name regex to search on
#	@filelist - array of names to search
#
# Returns a new array of the items which do not match the regex
#
sub reverseGrepFiles {
    my ( $regex, @filelist ) = @_;
    my @result = qw();
    foreach $file (@filelist) {
	if ( $file !~ /$regex/ ) {
	    push( @result, $file );
	}
    }
    return @result;
}



#
# fixPathname()
#	Removes trailing slashes and backslashes
#
# Parameters:
#	$path : Path to fix
#
# Returns the fixed path
#
sub fixPathname {
    my ( $path ) = @_;

    # strip trailing backslash
    if ( $path =~ /(.*)(\\)$/){
	$path = $1;
    }

    # strip trailing slash
    if ( $path =~ /(.*)(\/)$/){
	$path = $1;
    }

    return $path;
}



#
# storeLibraryVersion()
#	This function is used to detect the version number of a library and save that info in libvers.t
# 
# Parameters:
#	$file : The name of .pro file to process
#
my $libVerFileName = "$QPEDIR/src/libvers.t";
sub initializeLibraryVersions {
    open( LIB_VERFILE, "> $libVerFileName" ) || die "Unable to write to file $libVerFileName\n";
    print LIB_VERFILE "#\$\{\n";
    close( LIB_VERFILE );
}
sub closeLibraryVersions {
    open( LIB_VERFILE, ">> $libVerFileName" ) || die "Unable to write to file $libVerFileName\n";
    print LIB_VERFILE "#\$\}\n";
    close( LIB_VERFILE );
    debugMsg("Generated libvers.t");
}
sub storeLibraryVersion {
    # debugMsg("storeLibraryVersion");
    my ($fileName) = @_;
    my $libName = "";
    my $libVer = "";
    my $targetPrefix;

    if ($opt_product eq "qtopiadesktop") {
	$targetPrefix = "qdesktop:TARGET";
    } else {
	# by default match a TARGET that has no ":" prefix
	$targetPrefix = "\^\\s\*TARGET";
    }

    open( PROFILE, "<$fileName") || die "Could not open file $fileName!\n";
    while (<PROFILE>) {
	if ($_ =~ /(VERSION\s*=\s*)(\d)\.(\d)\.(\d)/){
	    $libVer = "$2$3$4";
	}
	if ($_ =~ /($targetPrefix\s*=\s*)(\w*-?\w*)/){
	    $libName = $2;
	}
	if (($libVer ne "") && ($libName ne "")) {
	    last;
	}
    }
    close( PROFILE );

    if ($libName eq "") {
	$libName = basename( $fileName, ".pro" );
    }

    open( LIB_VERFILE, ">> $libVerFileName" ) || die "Unable to write to file $libVerFileName\n";
    # we can't build freetype as a dll at the moment because it is
    # not supported by the version of freetype in Qt embeded
    if ($libName ne "freetype") {
	print LIB_VERFILE "\tProject(\"LIBS /= s/${libName}.lib/${libName}${libVer}.lib/\");\n";
	print LIB_VERFILE "\tProject(\"TMAKE_LIBS /= s/${libName}.lib/${libName}${libVer}.lib/\");\n";
    }
    close (LIB_VERFILE);
}



#
# detectHost()
#	Detect the host that this script is running on
#
# Returns either "win32" or "unix" depending on what it detects
#
sub detectHost {
    debugMsg("Detecting host");
    my $detectedHost = "";
    if ( $^O eq "MSWin32" || $^O eq "cygwin" ) {
	$detectedHost = "win32";
    } elsif ( $^O eq "linux" ) {
	$detectedHost = "unix";
    } else { 
	die "Can't autodetect the host. Looking for MSWin32, cygwin or linux. Host = $^O.\n" .
	    "Please explicitly set a -host paramater.\n";
    }
    return $detectedHost;
}



#
# validateHost()
#	Check host setting is valid, else abort
#
# Parameters :
#	$host: Host setting
#
sub validateHost {
    debugMsg("Validating host");
    my ( $host )  = @_;
    if ( $host eq "win32" || $host eq "unix" ) {
	return;
    } else {
	die "Invalid host setting, -host $host specified, but expecting either win32 or unix\n";
    }
}



#
# initializeHostSettings()
#
sub initializeHostSettings {
    debugMsg("Initializing host settings");
    if ($opt_host eq "win32") {
	$COPY = "copy ";
	$MAKE = "nmake ";
	$MKDIR = "mkdir ";
        $DIR_SEPARATOR = "\\";
        $CMD_SWITCH_CHAR = "/";
    } else {
	$COPY = "cp ";
        $MAKE = "make ";
	$MKDIR = "mkdir -p ";
        $DIR_SEPARATOR = "/";
        $CMD_SWITCH_CHAR = "-";
    }
}



#
# detectValidPlatformSettings()
#
sub detectValidPlatformSettings {
    my $configsDir = "";
    if ( $TMAKEDIR ) {
	$configsDir = "$TMAKEDIR\/lib\/qws";
    } else {
	$configsDir = "$QPEDIR\/src\/configs";
    }
    my @configs = findFiles( "linux-.*-g\\+\\+\$", $configsDir );
    my @newConfigs = qw();
    foreach $plat ( @configs ) {
	$plat = basename( $plat );
	$plat =~ s/linux-//g;
	$plat =~ s/(-static|-shared)(-debug)?//g;
	$plat =~ s/-g\+\+//g;
	push ( @validPlatformList, basename( $plat ) );
    }
#    foreach $plat ( @validPlatformList ) {
#	print "config option: $plat\n";
#    }
}



#
# validatePlatformSetting()
#
sub validatePlatformSetting {
    foreach $plat ( @validPlatformList ) {
	if ( $opt_platform eq $plat ) {
	    return "true";
	}
    }
    return "false";
}



#
# initializePlatformSettings()
#
sub initializePlatformSettings {
    detectValidPlatformSettings();
    if ( !$opt_platform ) {
	$opt_platform = "generic";
    }
# Does not work on windows qtopia desktop where platform is not used
#    if ( validatePlatformSetting() eq "false" ) {
#	die "The -platform setting is invalid, there are no associated config files for $opt_platform";
#    }
    # Setup button settings for specified device platform
    if ( -e "$QPEDIR/etc/defaultbuttons-$opt_platform.conf" ) {
	runCmd( "$COPY $QPEDIR/etc/defaultbuttons-$opt_platform.conf $QPEDIR/etc/defaultbuttons.conf" );
    } else {
	print "Could not find a device button mappings file for platform $opt_platform\n";
    }
    $mangledBaseOutDir = "\-$opt_host\-qt$qtVersion\-$opt_product$DIR_SEPARATOR$opt_platform\-$binType\-$releaseType$DIR_SEPARATOR";
}



#
# validateQtVersion()
#	Check if installed Qt version matches the desired Qt version number, if not then abort
#
# Parameters :
#	$checkForQtVersion: Qt Major Version number to check Qt headers contain
#
sub validateQtVersion {
    debugMsg("Validating Qt version");
    my ( $checkForQtVersion ) = @_;

    # check to make sure the qtdir set by the user is correct
    if (!$QTDIR) {
      die "Your QTDIR environment variable must be set to a qt $checkForQtVersion branch.\n";
    }

    my $qglobalFile = "$QTDIR/include/qglobal.h";

    open( QTVERSIONFILE, "<$qglobalFile" ) || 
	die "Could not open file '$qglobalFile'. Please set your QTDIR correctly.\n";
    my $actualVersion=0;
    while (<QTVERSIONFILE>) {
	if ($_ =~ /(VERSION_STR.*")(\d)(.*")/){
	    $actualVersion=$2;
	}
    }
    close( QTVERSIONFILE );

    if ($actualVersion != $checkForQtVersion) {
	die "Expecting the Qt branch to be set to Qt $checkForQtVersion.\n" .
	    "The QTDIR is set to the Qt $actualVersion branch.\n";
    }
}



#
# validateOptions()
#	Check if options supplied are valid, if not then abort
#
sub validateOptions {
    debugMsg("Validating options");

    if ($opt_debug && $opt_release) { 
	die "Choose either debug or release option but not both";
    }

    if ($opt_shared && $opt_static) { 
	die "Choose either shared or static option but not both";
    }

    if ($opt_host ne "unix" && $opt_kde3) { 
	die "KDE option is only valid for UNIX builds";
    }

    if ($opt_c && $opt_product eq "qtopiadesktop") {
	die "You can only choose qtopiadesktop or a specific directory but not both";
    }

    validateQtVersion($qtVersion);
}



#
# usage()
#
# Prints the usage of this script giving the list of switches, flags etc.
#
sub usage {
    print "Usage configure  [-v] [-debug|-release] [-static|-shared] [-dsp] [-console]\n" .
	"		    [-platform (sharp|ipaq|mips)]\n" . 
	"                   [-product (qtopiadesktop|sdk|qtopia)]\n" .
	"                   [-kde3] [-host (win32|unix)] [-rpath <lib dir>]\n" .
	"                   [-errorlog <errorfile>]\n" .
	"\n" .
	" Option         Meaning\n" .
	" ------         ----------\n" .
	" v              Be verbose used to debugging this script and your settings\n" .
	" c              Rebuild profile of specified path. eg libraries/qtopiacalc\n" .
	" debug          Build a debug version of product\n" .
	" release*       Build a release version of product\n" .
	" static         Build statically linked objects\n" .
	" shared*	 Build shared objects\n" .
	" dsp            Under windows create dsp files" .
	" console        Under windows build applications as console applications\n".
	" platform       Type of cross compiled target; eg sharp, ipaq or mips.\n" .
	"                There must be an associated file found at\n" .
	"                \$QPEDIR/src/configs.\n" .
	" product        What type of product to build \n" .
	"                If product is qtopiadesktop a build for qtopiadesktop will only\n" .
	"                occur if source is available. This requires the QTDIR\n" . 
	"                environment variable to be set to a QT 3.x branch.\n" .
	" kde3	         Enable KDE 3.x integration. Currently only useful for\n" .
	"                QtopiaDesktop under unix. This requires KDE 3.x headers and\n" .
	"                libraries to be installed.\n" .
	" host           Host platform that you are compiling on; either win32 or unix.\n" . 
	"                If not specified, will autodetect the host\n" .
	" rpath          Sets the rpath variable for linking\n" .
	"                to the provided <lib dir> (Unix only).\n" .
	"                This is useful for building rpms.\n" .
	" errorlog       Supply the name of a file to use as an error log\n" .
	"\n" .
	"\n" .
	"      Items marked with a * are enabled by default\n" .
	"      Items marked with a # are required\n" .
	"\n";
}



#
# parseOptions()
#	Parse the option provided by the user, if not possible then abort
#
sub parseOptions {
    # Read our options
    @optl = ( "v", "c:s", "debug", "makefile:s", "qt3", "release","product:s", "host:s", 
	    "platform:s", "static", "shared", "dsp", "console", "errorlog:s", "rpath:s", "kde" );

    if ( !GetOptions @optl ) {
        usage();
        die "Exiting due to error reading command line options";
    }

    if (!$opt_v) {
	$verbose = "false";
    }

    debugMsg("Parsing options");

    if (!$opt_host) {
	$opt_host = detectHost();
    }

    validateHost( $opt_host );
    debugMsg( "Using $opt_host host settings" );

    if (!$opt_product) {
	$opt_product = "qtopia";
    }

    if ($opt_errorlog) {
	$errorOutput = "> $opt_errorlog";
	$errorOutputAppend = ">> $opt_errorlog"
    }

    if ($opt_makefile) {
	# keep our sanity when sharing a directory with *nix
	$MAKEFILE = $opt_makefile;
    }

    $TOPLEVEL_MAKEFILE = perlSlashify("${QPEDIR}/src/${MAKEFILE}");

    if ($opt_debug) {
	$releaseType = "debug";
    }

    if ($opt_static) {
	$binType = "static";
    }

    if ($opt_qt3 || $opt_product eq "qtopiadesktop") {
	$qtVersion = 3;
    }

    validateOptions();
}



sub initializeQMakeSettings {
    $QMAKE_FLAGS = "";
    my $QMAKE_DEFINES = "";
    my $QMAKE_CONFIG = "$releaseType warn_on";

    if ( $opt_host eq "win32" ) {
	$QMAKE_DEFINES .= " QTOPIA_DLL QTOPIAPIM_DLL";
    } else {
	if ( !$opt_rpath ) {
	    $opt_rpath = "$QPEDIR/lib";
	}
	$QMAKE_FLAGS .= "\"QMAKE_LFLAGS=-Wl,-rpath,$opt_rpath\"";
    }

    if ( $opt_kde ) {
	$QMAKE_CONFIG .= " kde";
    } else {
	push (@excludedComponents,  "./src/qtopiadesktop/externalplugins/kde/kde.pro");
    }

    if ($opt_product eq "qtopiadesktop") {
	$QMAKE_DEFINES .= " QTOPIA_DESKTOP";
	$QMAKE_CONFIG .= " qdesktop thread qt3";
    }

    if ($opt_host eq "win32") {
	if ($opt_console) {
	    $QMAKE_CONFIG .= " console";
	}
	if ($opt_static) {
	    $QMAKE_DEFINES .= " QT_NODLL";
	} else {
	    $QMAKE_DEFINES .= " QT_DLL";
	}
    }

    $QMAKE_FLAGS .= " \"DEFINES+=$QMAKE_DEFINES\" \"CONFIG-=debug\" \"CONFIG-=release\" \"CONFIG+=$QMAKE_CONFIG\" ";
}



sub initializeTMakeSettings {
    if (!$TMAKEPATH && !$TMAKEDIR) {
	# This script should eventually do what the configure shell script does and
	# use Makefile.in files from the package to gerenate Makefiles when tmake is not available
	die "You must have \$TMAKEPATH or \$TMAKEDIR set.";
    }

    if ($TMAKEDIR) {
	if ($opt_host eq "win32") {
	    if ($opt_sdk) {
		$TMAKEPATH = $TMAKEDIR . "\/lib\/qws\/win32-msvc";
	    } else {
		$TMAKEPATH = $TMAKEDIR . "\/lib\/win32-msvc";
	    }
	} else {
	    $TMAKEPATH = $TMAKEDIR . "\/lib\/qws\/linux-" . $opt_platform . "-g++";
	}
    }

    if ($opt_host eq "win32" && $opt_sdk && $TMAKEPATH !~ /qws\/win32-msvc/) {
	die "Expecting TMAKEPATH to contain qws/win32-msvc but it doesn't. It is currently set to $TMAKEPATH.\n";
    }

    if (!$TMAKE) {
	$TMAKE = "tmake";
    }
    $TMAKE .= " -$opt_host \"TMAKEPATH = $TMAKEPATH\"";

    $objectDir = "$DIR_SEPARATOR.obj$mangledBaseOutDir";
    $tempDir = "$DIR_SEPARATOR.moc$mangledBaseOutDir";

    # Original shell script which is one part of shadow builds
    #    MOC_DIR=.moc
    #    OBJECTS_DIR=.obj
    #    tmake $MAGICPATH $STATIC $SINGLE $VCONFIG $QCONFIGARG $ECONFIG LIBS+="$EXTRALIBS" \
    #    OBJECTS_DIR="\$(OBJECTS_DIR)" MOC_DIR="\$(MOC_DIR)" INCLUDEPATH+="\$(MOC_DIR)" \
    #	-t $H/qt/tmake/propagate.t $f.pro >Makefile.in

    # Shadow builds do not work yet
    # my $outputDirs = "\"OBJECTS_DIR=\"\\\$(OBJECTS_DIR)\" MOC_DIR=\"\\\$(MOC_DIR)\" INCLUDEPATH+=\"\\\$(MOC_DIR)\"";
    my $outputDirs = "";

    $TMAKE_FLAGS = "";
    my $TMAKE_DEFINES = "";
    my $TMAKE_CONFIG = "qt$qtVersion qtopia multiprocess dynamic";

    if ($opt_product eq "qtopiadesktop") {
	$TMAKE_DEFINES .= " QTOPIA_DESKTOP";
	$TMAKE_CONFIG .= " qdesktop thread qt3";
    } else {
	$TMAKE_CONFIG .= " embedded";
    }

    if ($opt_host eq "win32") {
	if ($opt_console) {
	    $TMAKE_CONFIG .= " console";
	}
	if ($opt_static) {
	    $TMAKE_DEFINES .= " QT_NODLL";
	} else {
	    $TMAKE_DEFINES .= " QT_DLL";
	}
	if (!$opt_qt3) {
	    # Disable threading when using Qt2.3
	    $TMAKE_FLAGS .= " \"CONFIG -= thread\" \"TMAKE_LFLAGS += /NODEFAULTLIB:libc\"";
	}
    }

    $TMAKE_FLAGS .= " \"DEFINES += $TMAKE_DEFINES\" \"CONFIG += $TMAKE_CONFIG $releaseType\" $outputDirs";
}



#
# Setup either qmake or tmake options, which ever is needed
# (perhaps this should be turned in to a perl map)
#
my $qtmake_cmd;
my $qtmake_cmd_args;
my $qtmake_dsp_lib_args;
my $qtmake_dsp_app_args;
sub initializeMakeMake {
    my ( $option ) = @_;
    if ( $option eq "qmake" ) {
	$qtmake_cmd = $QMAKE;
	$qtmake_cmd_args = "$QMAKE_FLAGS";
	$qtmake_dsp_lib_args = "-tp vc";
	$qtmake_dsp_app_args = "-tp vc";
    } elsif ( $option eq "tmake" ) {
	$qtmake_cmd = "$TMAKE";
	$qtmake_cmd_args = "$TMAKE_FLAGS";
	$qtmake_dsp_lib_args = "-t vcgeneric";
	$qtmake_dsp_app_args = "-t vcapp";
    } else {
	die "Don't know how to use $option make make\n";
    }
}



#
# generateMakefile()
#
#
sub generateMakefile {
    my ( $item, $type ) = @_;

    if ( -e $item ) {
	my $pwd = getcwd();
	chdir dirname( $item );
	$item = basename( $item );
	# run tmake or qmake
	if ($opt_dsp) {
	    my $dspArgs;
	    if ( $type eq "lib" ) {
		$dspArgs = $qtmake_dsp_lib_args;
	    } else {
		$dspArgs = $qtmake_dsp_app_args;
	    }
	    my $dspFileName = $item;
	    $dspFileName =~ s/\.pro//;
	    $dspFileName = "$dspFileName" . ".dsp";
	    runCmd( "$qtmake_cmd $dspArgs $qtmake_cmd_args -o \"$dspFileName\" \"$item\"" );
	}
	runCmd( "$qtmake_cmd $qtmake_cmd_args -o \"$MAKEFILE\" \"$item\"" );
	chdir $pwd;
    } else {
	debugMsg("Missing .pro file: $item");
    }

    # Show dots as each component is configured
    if ($verbose eq "false") {
       print ".";
    }
}



#
# syncQtopia()
#
sub syncQtopia {
    system "$QPEDIR/bin/syncqtopia" || die "Unable to sync header files\n";
}



my @item_list = qw();
my @type_list = qw();
my @make_list = qw();
sub saveGenerateMakefileArgs {
    my ( $item, $type, $make ) = @_;
    push ( @item_list, $item );
    push ( @type_list, $type );
    push ( @make_list, $make );
}



sub generateSavedMakefiles {
    my $i = 0;
    foreach $item ( @item_list ) {
	initializeMakeMake( $make_list[ $i ] );
	generateMakefile( $item, $type_list[ $i ] );
	$i++;
    }
    # Done
    debugMsg("$TOPLEVEL_MAKEFILE has been made.");
    print "\n\nYou can now run \"$MAKE $CMD_SWITCH_CHAR" . "f ". hostSlashify($TOPLEVEL_MAKEFILE). "\"\n";
}



#
# outputMakeVariable()
#	Generate the Makefile definition using the given variable
#	name and list of items to be in variable
#
# Parameters:
#	$type : Describes if its an app or lib type (needed for dsp files and lib versioning)
#	$variableName : Makefile variable name to generate 
#	@list : Items to be in variable
#
# Returns Makefile defition for $variableName
#
sub outputMakeVariable {
    my ( $make, $type, $variableName, @list ) = @_;
    my $result = "$variableName =";
    foreach $item (@list) {
	if ( $type eq "other" ) {
	    $item =~ s/\.//; # strip off leading .
	    $item = "\$(QPEDIR)" . $item; 
	} else {
	    if ( $type eq "lib" ) {
		storeLibraryVersion( $item );
	    }
#	    generateMakefile( $item, $type );
	    saveGenerateMakefileArgs( $item, $type, $make );
	    $item =~ s/\.\/src\///; # strip off leading ./
	    $item = dirname( $item );
	}
	$result = $result . " \\\n\t" . hostSlashify($item);
    }
    $result = $result . "\n\n";
    return $result;
}




sub generateComponentLists {
    debugMsg("Generating component lists");

    $allCmds = "";

    chdir( $QPEDIR );

    if ( $opt_c ) {
	# generateMakefile( $opt_c, $type );
	die "-c option not implemented\n";
    }

    # build a list of all excluded components
    my @allExcludedComponents = @excludedComponents;
    if ($opt_host eq "win32") {
	@allExcludedComponents = ( @allExcludedComponents, @win32Excludes );
    } else {
	@allExcludedComponents = ( @allExcludedComponents, @unixExcludes );
    }
    if ( !$opt_kde ) {
	push ( @allExcludedComponents, "./src/qtopiadesktop/externalplugins/kde/kde.pro" );
    }

    if ($opt_product eq "qtopiadesktop") {

	# build a list of all QtopiaDesktop .pro files found under $QPEDIR/src/qtopiadesktop
	my @qdprofiles  = findFiles( "^\.\/src\/qtopiadesktop\/.*\.pro\$", "./" );
	@componentprofiles = subtractArrays( \@qdprofiles, \@allExcludedComponents );
	my @pluginProfiles  = grepFiles( ".*\/externalplugins\/.*", @componentprofiles );
	my @applicationProfiles  = reverseGrepFiles( ".*\/externalplugins\/.*", @componentprofiles );

	$allCmds .= outputMakeVariable( "tmake", "lib", "OTHER_LIBS",	( "./src/libraries/qtopiapim/qtopiapim.pro" ) );
	$allCmds .= outputMakeVariable( "qmake", "lib", "COMPONENTS",	@pluginProfiles );
	$allCmds .= outputMakeVariable( "qmake", "app", "APPS", 	@applicationProfiles ); # QTOPIA_DESKTOP
	$allCmds .= outputMakeVariable( "tmake", "lib", "CORE_LIB_1",	( "./src/libraries/qtopia/qtopia.pro" ) );

    } else {

	# build a list of all files found in $QPEDIR (so that findFiles is only called once)
	my @allfiles = findFiles( ".*", "./" );
	my @allprofiles = grepFiles( "^\.\/src\/.*\.pro\$", @allfiles );
	my @origprofiles = @allprofiles;
	@allprofiles = subtractArrays( \@allprofiles, \@allExcludedComponents );
	@allprofiles = reverseGrepFiles( ".*\.indent\.pro\$|.*libraries\/qtopia\/qtopia\.pro\$|.*libraries\/qtopia1\/qtopia1\.pro\$", @allprofiles );
	my @libProfiles = grepFiles( ".*\/libraries\/.*", @allprofiles );
	my @componentprofiles  = reverseGrepFiles( ".*\/libraries\/.*", @allprofiles );
	@componentprofiles  = reverseGrepFiles( ".*\/qtopiadesktop\/.*", @componentprofiles );
	my @pluginProfiles  = grepFiles( ".*\/plugins\/.*", @componentprofiles );
	my @applicationProfiles  = reverseGrepFiles( ".*\/plugins\/.*", @componentprofiles );

	# build a list of control files
	my @allcontrols = grepFiles( ".*\.control\$", @allfiles );
	my @extracontrols = qw();
	foreach $file (@allcontrols) {
	    if ( $file =~ /\.\/src\/.*/ ) {
		my $condir = dirname( $file );
		my $foundflag = "false";
		foreach $profile ( @origprofiles ) {
		    if ( dirname($profile) eq $condir ) {
			$foundflag = "true";
			last;
		    }
		}
		if ( $foundflag eq "false" ) {
		    push( @extracontrols, $file );
		}
	    } else {
		push( @extracontrols, $file );
	    }
	}

	# output and gerenate Makefiles
	$allCmds .= outputMakeVariable( "tmake", "lib",   "OTHER_LIBS",    @libProfiles );
	$allCmds .= outputMakeVariable( "tmake", "lib",   "COMPONENTS",    @pluginProfiles );
	$allCmds .= outputMakeVariable( "tmake", "app",   "APPS",          @applicationProfiles );
	$allCmds .= outputMakeVariable( "tmake", "lib",   "CORE_LIB_1",    ( "./src/libraries/qtopia/qtopia.pro" ) );
	$allCmds .= outputMakeVariable( "tmake", "other", "EXTRAPACKAGES", @extracontrols );
    }

    # Special rules for the core libraries which must be built first
    my $coreLibCmds = "";
    if ( $opt_host eq "win32" ) {
	$coreLibCmds = $coreLibCmds .
	    "\$(OTHER_LIBS): \$(CORE_LIB_1)\n".
	    "\n".
	    "LIBS=\$(CORE_LIB_1) \$(OTHER_LIBS)\n";
    } else {
	$coreLibCmds = $coreLibCmds .
	    "CORE_LIB_2= \\\n".
	    "	libraries/qtopia1\n".
	    "\n".
	    "\$(CORE_LIB_2): \$(CORE_LIB_1)\n".
	    "\n".
	    "\$(OTHER_LIBS): \$(CORE_LIB_2)\n".
	    "\n".
	    "LIBS=\$(CORE_LIB_1) \$(CORE_LIB_2) \$(OTHER_LIBS)\n";
	saveGenerateMakefileArgs( "./src/libraries/qtopia1/qtopia1.pro", "lib", "tmake" );
    }

    my $makeForEach = "";
    my $makePackagesCmd = "";

    if ( $opt_host eq "win32" ) {
	$makePackagesCmd = "\techo Making packages not supported under windows\n";
        $makeForEach =	"\techo cd %1 > \$(QPEDIR)\\src\\make_c.bat\n".
			"\techo \$(MAKE) %2 >> \$(QPEDIR)\\src\\make_c.bat\n".
			"\tfor %D in ( \$(LIBS) \$(COMPONENTS) \$(APPS) ) do call \$(QPEDIR)\\src\\make_c.bat \$(QPEDIR)\\src\\%D \$@";
    } else {
	$makePackagesCmd = "\tmkdir -p \$(QPEDIR)/ipkg\n".
	    "\t$makeForEach\n".
	    "\tfor ctrl in \$(EXTRAPACKAGES); do cd \$(QPEDIR)/ipkg; ../bin/mkipks \$\$ctrl; done\n".
	    "\tcd \$(QPEDIR)/ipkg; ../scripts/mkPackages\n";
        $makeForEach = "\tfor dir in \$(LIBS) \$(COMPONENTS) \$(APPS); do cd \$(QPEDIR)$DIR_SEPARATOR\$\$dir; \$(MAKE) \$@ || exit 1; done";
    }

    my $uiHeaderRules =	"UI_HEADERS = \\\n".
	"\t\$(QPEDIR)\/include\/qtopia\/private\/categoryeditbase_p.h \\\n".
	"\t\$(QPEDIR)\/include\/qtopia\/private\/categoryedit_p.h\n\n".
	"\$(UI_HEADERS):\n".
	"\tsyncqtopia\n";

    # Build and output the top level Makefile
    open (OUTPUT, ">$TOPLEVEL_MAKEFILE")  ||  die "Unable to write to $TOPLEVEL_MAKEFILE\n";
    print OUTPUT "default:  dynamic\n\n".
	"$allCmds$coreLibCmds\n".
	"dynamic: \$(APPS) \$(COMPONENTS)\n\n".
	"$uiHeaderRules\n".
	"\$(COMPONENTS): \$(LIBS)\n\n".
	"\$(APPS): \$(LIBS) \$(UI_HEADERS)\n\n".
	"\$(LIBS) \$(COMPONENTS) \$(APPS):\n\tcd \$(QPEDIR)$DIR_SEPARATOR"."src$DIR_SEPARATOR\$@\n\t\$(MAKE)\n\n".
	"clean:\n$makeForEach\n\n".
	"lupdate:\n$makeForEach\n\n".
	"lrelease:\n$makeForEach\n\n".
	"ipkg:\n$makePackagesCmd\n\n".
	"\n".
	"default: FORCE\n\n".
	"dynamic: FORCE\n\n".
	"\$(LIBS): FORCE\n\n".
	"\$(APPS): FORCE\n\n".
	"\$(COMPONENTS): FORCE\n\n".
	"clean: FORCE\n\n".
	"FORCE:\n\n".
	".PHONY: default dynamic \$(LIBS) \$(APPS) \$(COMPONENTS) clean\n";
    close(OUTPUT);
}



####################################
# Start of main logic for configure
####################################

parseOptions();
initializeHostSettings();
initializePlatformSettings();
initializeTMakeSettings();
initializeQMakeSettings();
initializeLibraryVersions();
syncQtopia();
generateComponentLists();
closeLibraryVersions();
generateSavedMakefiles();
exit 0;


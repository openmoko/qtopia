#! /usr/bin/perl -w

# This file is used to create the make files under WIN32
# 
#  typical usage:
#       perl configure.pl -win32 -debug -v -errorlog \error.txt

use File::Basename;
use Getopt::Long;


# by default match a TARGET that has no ":" prefix
my $targetConfig ="^\s*";

# targetInfo(file)
# This function is used to detect the target name, type and version
#
# Returns an array with target name, target type and target version 
#    in that o rder 

#   Target name returned is empty if any of  name, type or version are missing
#

sub targetInfo{
    my ($fileName)  = @_;
    my $targetName ="";
    my $targetVersion ="";
    my $targetType ="";
    my $targetPrefix = $targetConfig . "TARGET";
    open (PROFILE, "<$fileName") || die "Could not open file $fileName!\n";

    while (<PROFILE>){
	if ($_ =~ /(VERSION\s*=\s*)(\d)\.(\d)\.(\d)/){
	    $targetVersion = "$2$3$4";
	}
	# This will end up as "qdeskop:TARGET" when buiding the qtopia desktop
	if ($_ =~ /($targetPrefix\s*=\s*)(\w*-?\w*)/){
	    $targetName = $2;
	}
	if ($_ =~ /(TEMPLATE\s*=\s*)(\w*)/){
	    $targetType = $2;
	}

	if (($targetVersion ne "") && ($targetName ne "") && ($targetType ne "")){
	    close(PROFILE);
            return ($targetName, $targetType, $targetVersion);
	}
    }

    close(PROFILE);
    if ($targetName eq ""){
	$targetName = basename($fileName, ".pro");
    }
    if (($targetName ne "") && ($targetType ne "")){
	return ($targetName, $targetType, $targetVersion);
    }else{
	print "Missing target type for $fileName\n";
        return  qw ("" "" "");
    }
}

my $TMAKE;
my $TMAKEDIR = $ENV{'TMAKEDIR'};
my $TMAKEPATH = $ENV{'TMAKEPATH'};
my $QPEDIR = $ENV{'QPEDIR'};
my $tmakePathFlags ="";
my $objectDirFlags ="";
my $tmakeConfigs ="";
my $objectDir = "";
my $tempDir = "";
my $qtVersion ="qt2";
my $componentType = "win32";
my $QDESKTOP = "";
# what is the name of the make file to generate
my $MAKEFILE = "Makefile"; 
#what is our plaform , either win32 or unix
my $platform;
# What components are not ported yet
my @unported;
my $distCmds;
my $allCmds;
my $cleanCmds;

 
# Read our options

@optl = ("v", "c:s", "debug", "makefile:s", "qt3", "release","qtopiadesktop", "unix", "win32", "static", "dsp", "errorlog:s");

die "Usage configure  [v] (-win32|-unix) [-debug|-release] [dsp] [static] [-errorlog <errorfile>]\n\n" .
	"\t Option          Meaning\n" .
	"\t ------          ----------\n" .
	"\t  v              Be verbose used to debugging this script and your settings\n" .
	"\t  c              Rebuild profile of specified path. eg libraries/qtopiacalc\n" .
	"\t  debug          Build a debug version of qtopia\n" .
	"\t  release        Build a release version of qtopia\n" .
	"\t  qtopiadesktop  Build for qtopiadesktop\n" .
	"\t  unix           Run configure in Unix mode\n" .
	"\t  win32          Run configure in Windows mode\n" .
	"\t  dsp            Create dsp files: Not yet completed\n" .
	"\t  errorlog       Supply the name of a file to use as an error log\n"
  unless ((GetOptions @optl)  && ($opt_win32 || $opt_unix)); 


if ($opt_debug && $opt_release){   
    die "Choose either debug or release option but not both";
}

if ($opt_win32 && $opt_unix){
    die "Choose either win32 or unix option but not both";
}

if ($opt_debug){
  $release = "debug";
}else{
  $release = "release";
}

my $binType;
if ($opt_static){
   $binType="static_";
}else{
   $binType="";
}

if ($opt_qt3){
    $qtVersion = "qt3";
}

if ($opt_c && $opt_qtopiadesktop ) {
    die "You can only choose qtopiadesktop or a specific directory but not both";
}

if($opt_qtopiadesktop){
    $qtVersion = "qt3";
    $tmakeConfigs = $tmakeConfigs . "qdesktop thread qt3";
    $QDESKTOP="/desktop";
    $targetConfig = "qdesktop:";
    if(!$opt_win32){ 
	$componentType="desktop";
    }else{
	$componentType="win32desktop";
    }
}



if ($opt_win32){
  $platform ="win32";
  $MAKE = "nmake ";
  if ($binType ne ""){
    $objectDir ="msvc/obj$QDESKTOP/$binType".$release;
  }else{  
    $objectDir ="msvc/obj$QDESKTOP/$release";
  }
  $tmpDir ="msvc/moc$QDESKTOP/$release";

  if (!$opt_qtopiadesktop){
      if ($TMAKEPATH =~ /qws/){
	# TMAKEPATH may not be pointing to right qws directory eg
	#    e:\tmake\lib\qws\linux-g++	
	$tmakePathFlags = "\"TMAKEPATH = $TMAKEPATH/../win32-msvc\"";
      }else{
	# assume user has TMAKEPATH set to something like
	#    e:\tmake\lib\win32-msvc	
	$tmakePathFlags = "\"TMAKEPATH = $TMAKEPATH/../qws/win32-msvc\"";
      }
  }
}else{
  $platform = "unix";
  $MAKE = "make ";
}

  if ($opt_v) {
    print  "TMAKEPATH=$TMAKEPATH\n";
  }

if ($ENV{'TMAKE'}){
   $TMAKE ="$ENV{'TMAKE'} -". $platform. " $tmakePathFlags";
   
}else{
    $TMAKE ="tmake  -". $platform. " $tmakePathFlags";
}

if ($opt_v){
  $verbose = "true";
}else{
  $verbose = "false";
}

if ($ENV{"DISTDIR"}){
    $distDir = $ENV{"DISTDIR"};
}else{
    if ($opt_win32){
	$distDir = "c:\\qtopiaDist";
    }else{
	$distDir = "~/qtopiaDist";
    }
}
my $errorOutput;
my $errorOutputAppend;

if ($opt_errorlog){
  $errorOutput = "> $opt_errorlog";
  $errorOutputAppend = ">> $opt_errorlog"
}else{
  $errorOutput = "";
  $errorOutputAppend = "";
}

# shadow builds do not work under windows at the moment
# if ($objectDir ne ""){
#   $objectDirFlags = " \"OBJECTS_DIR = $objectDir\" \"MOC_DIR = $tempDir\" \"INCLUDEPATH += $tempDir\" ";
# }

#if (!$opt_c){
#    system "syncqtopia" || die "Unable to sync header files \n";
#}

my $base;
my $proFile;
my $item;

# Find out what components we are to build
my $componentList;
my @components;
my $showComponentType = "show" . $componentType . "components";
if (!$opt_c){
    my $cmdLine;
    if ($opt_win32){
       $cmdLine = "$MAKE /F $QPEDIR/src/Makefile.linux $showComponentType";
    }else{
       $cmdLine = "$MAKE -f $QPEDIR/src/Makefile.linux $showComponentType";
    }
    if ($opt_v){
	print "Running command $cmdLine \n";
    }
    $componentList = `$cmdLine`;
    @components = split(/[ \t\n]/, $componentList);
}else{
    @components =  ("$opt_c"); 
}
 

if (!$opt_c){
    # Find out what the version number of the libraries are

    # This will not be needed for qmake ie qte3.0
    my $libraryList = $componentList; 
    my @libraryComponents = split(/[ \t\n]/, $libraryList);
    my $libVerFileName ="$QPEDIR/src/libvers.t";
    my $libVer;
    my $libName;
    my $libType;
    my $cmdLine;
    my $cmdResult;
    my @targetSpecs;

    # This is a temporary measure to make it easier to compile, by avoiding components
    #    that have not been ported to WIN32 yet. These will be looked at after 
    #    the core of Qtopia has been ported
    if ($opt_win32){
	@unported = qw (3rdparty/applications/embeddedkonsole
		       3rdparty/applications/keypebble
		       3rdparty/tools/atd
		       applications/mediaplayer
		       applications/mediarecorder
		       3rdparty/plugins/codecs/libmpeg3
		       3rdparty/plugins/codecs/libffmpeg
		       3rdparty/plugins/obex
		       3rdparty/plugins/obex/openobex
		       3rdparty/plugins/textcodecs/jp
		       plugins/network/dialup		 
		       plugins/codecs/wavplugin
		       plugins/codecs/wavrecord
		       applications/sysinfo
		       games/go
		       games/mindbreaker
		       games/wordgame
		       settings/light-and-power
		       settings/systemtime
		       settings/qipkg
		       tools/quickexec
		       single);
    }else{
	@unported = ();
    }

    open (LIB_VERFILE, "> $libVerFileName") || die "Unable to write to file $libVerFileName\n";
    print LIB_VERFILE "#\$\{\n";
    for $item (@libraryComponents){
	if ($item =~ /(libraries|plugin)/){
	    if ($item =~ /(.*\/)([^\/]*$)/){
		$base = $1.$2;
		$proFile = $2.".pro";
	    }else{
		$base = $item;
		$proFile = $item.".pro";
	    }
	    if (-e "$base/$proFile"){ 
		@targetSpecs = targetInfo("$base/$proFile");
		$libName = $targetSpecs[0];
		$libVer = $targetSpecs[2];   
		# we can't build freetype as a dll at the moment because it is
		#  not supported by the version of freetype in Qt embeded
		if (($libName ne "") && ($libName ne "freetype")){
		    print LIB_VERFILE "\tProject(\"LIBS /= s/${libName}.lib/${libName}${libVer}.lib/\");\n";
		}
	    }
	}
    }
    print LIB_VERFILE "#\$\}\n";
    close (LIB_VERFILE);
}

if (!$opt_qtopiadesktop){
    $TMAKE_FLAGS = "\"DEFINES +=QWS\" \"CONFIG += $qtVersion qtopia embedded \" $objectDirFlags"; 
}else{
    $TMAKE_FLAGS = "\"DEFINES +=QTOPIA_DESKTOP\" \"CONFIG += $qtVersion  qtopia \" $objectDirFlags"; 
}
if ($opt_win32 && $opt_debug){
  $TMAKE_FLAGS = $TMAKE_FLAGS." \"CONFIG += console\"";
}

if ($opt_win32){
    if ($opt_static){
       $TMAKE_FLAGS = $TMAKE_FLAGS." \"DEFINES += QT_NODLL \"";
    }else{
	$TMAKE_FLAGS = $TMAKE_FLAGS." \"DEFINES += QT_DLL \"";
	if (!$opt_qt3){
	  $TMAKE_FLAGS = $TMAKE_FLAGS." \"CONFIG -= thread \" \"TMAKE_LFLAGS += /NODEFAULTLIB:libc \"" ; # threads are broken under Qt2.3
	    if ($opt_debug) {
		$TMAKE_FLAGS = $TMAKE_FLAGS." \"TMAKE_CXXFLAGS += -MDd \" \"TMAKE_CFLAGS += -MDd \"";
	    } else {
		$TMAKE_FLAGS = $TMAKE_FLAGS." \"TMAKE_CXXFLAGS += -MD \" \"TMAKE_CFLAGS +=-MD \"";
	    }
	}
    }
   $TMAKE_FLAGS = $TMAKE_FLAGS." \"DEFINES += UNICODE \"";
}

$TMAKE_FLAGS = $TMAKE_FLAGS." \"CONFIG += $tmakeConfigs ". $release. "\"";

if ($opt_v){
  print "Make = $MAKE\n";
  print "Makefile = $MAKEFILE\n";
}



# Create makefiles needed 
if (!$opt_c){ 
    print "Generating makefiles\n";
    open (OUTPUT, ">$MAKEFILE")  ||  die "Unable to write to $MAKEFILE\n";
}else{
    print "Generating makefiles\n";
}

# commands to generate all targets
$allCmds = "";

# commands to clean all targets
$cleanCmds ="";
my $IS_LIB;

# what flags are needed to build plugins
my $pluginFlags;

my $runCompiler;
my $unportedItem;
my $baseWin32;
for $item (@components){
    if ($item =~ /(.*\/)([^\/]*$)/){
	$base = $1.$2;
	$proFile = $2.".pro";
    }else{        
	$base = $item;
	$proFile = $item.".pro";
    }
    $baseWin32 = join("\\", split(/\//, $base));
    if ($opt_v){ 
	print "Looking to create $QPEDIR/src/$base/$MAKEFILE\n";
	print "Changing into directory " . "$QPEDIR/src/$base\n";
    }else{
       print ".";
    }
    chdir("$QPEDIR/src/$base");	
    if (-e $proFile && $base ne "qt"){        
	$cmdLine = "$TMAKE \"CONFIG += multiprocess\" $TMAKE_FLAGS -o $MAKEFILE $proFile";
	if ($opt_v){
	    print "Running command line: $cmdLine\n";
	} 
        $result = `$cmdLine`;
        if ($objectDir ne ""){ 
	    if ($opt_win32){
		$allCmds = $allCmds."\t-mkdir $QPEDIR\\src\\$baseWin32\\$objectDir\n";
	    }else{
		$allCmds = $allCmds."\t-mkdir $QPEDIR/src/$base/$objectDir\n";
	    }
	}
        if ($tempDir ne ""){ 
	    if ($opt_win32){
		$allCmds = $allCmds."\t-mkdir $QPEDIR\\src\\$base\\$tempDir\n";
	    }else{
		$allCmds = $allCmds."\t-mkdir $QPEDIR/src/$base/$tempDir\n";
	    }
	}
	# has this component been ported yet?
	$runCompiler = 1;
	foreach $unportedItem (@unported){
	    if ($unportedItem eq  $item){
		$runCompiler = 0;
		print "\nDisabled compile of $item\n"
	    }
	}
	if ($runCompiler){
	    if ($opt_win32){
		$allCmds = $allCmds."\tcd $QPEDIR\\src\\$baseWin32\n";
		$allCmds = $allCmds."\t$MAKE /F $MAKEFILE $errorOutputAppend\n";
	    }else{
		$allCmds = $allCmds."\t$MAKE -C $QPEDIR/src/$base -f $MAKEFILE $errorOutputAppend\n";
	    }
	}
	if ($opt_win32){
            $cleanCmds = $cleanCmds."\tcd $QPEDIR\\src\\$baseWin32\n\t$MAKE /F $MAKEFILE clean $errorOutputAppend\n";
	}else{
            $cleanCmds = $cleanCmds."\t$MAKE -C $QPEDIR/src/$base -f $MAKEFILE clean $errorOutputAppend\n";
	}
    }else{
	if ($verbose eq "true"){
	    if ($opt_win32){
		print "no file $proFile not found in $QPEDIR\\src\\$baseWin32\n";
	    }else{
		print "no file $proFile not found in $QPEDIR/src/$base\n";
	    }
	}
    }
}


if (!$opt_c ){
    if ($opt_errorlog){
       # clean the error log first if it is  used
       $allCmds = "\techo \"\" $errorOutput\n".$allCmds;
       $cleanCmds = "\techo \"\" $errorOutput\n".$cleanCmds;
    }
    if ($opt_qtopiadesktop){
	if ($opt_win32){
	    $allCmds =$allCmds."\tcd \$(QPEDIR)\\src\\qtopiadesktop\n" .
	                       "\tconfigure.bat\n" .
	                       "\tconfigure.bat\n" .
			       "\t$MAKE";
	}else{
	  $allCmds = $allCmds."\tcd \$(QPEDIR)/src/qtopiadesktop\n" .
	                       "\tconfigure\n";
	}
    }
   $allCmds = "\tsyncqtopia\n". $allCmds; 
    print OUTPUT "all:\n$allCmds\n\nclean:\n$cleanCmds";
    print "\n";
    close (OUTPUT);
}
exit 0;



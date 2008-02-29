package Qtopia::Vars;

use strict;
use warnings;

use File::Basename;
use Qtopia::Paths;

require Exporter;
our @ISA = qw(Exporter);

# Items to export into callers namespace by default. Note: do not export
# names by default without a very good reason. Use EXPORT_OK instead.
# Do not simply export all your public functions/methods/constants.

# This allows declaration	use Qtopia::opt ':all';
# If you do not need this, moving things directly into @EXPORT or @EXPORT_OK
# will save memory.
our %EXPORT_TAGS = ( 'all' => [ qw(
) ] );

our @EXPORT_OK = ( @{ $EXPORT_TAGS{'all'} } );

our @EXPORT = qw(
    $isWindows
    $isMac
    $qtopiaVersionStr
    $qtVersionStr
    $shadow
    check_script
    configopt
    script_name

    $depotpath
);

our $VERSION = '0.01';

# imported variables
our $depotpath;


use constant TRACE => 0;

# Platform detection
our $isWindows = ( $^O eq "MSWin32" || $^O eq "cygwin" );
our $isMac = ( $^O eq "darwin" );
our $qtopiaVersionStr;
our $qtVersionStr;
our @configureoptions;
our $shadow;

# Check for a bug in perl on RedHat 9.1
if ( !$isWindows ) {
    TRACE and print "Qtopia::Vars::perl_bug_test()\n";
    my $testfile = "/etc/passwd";
    open IN, "$testfile" or die "Cannot read $testfile\n";
    my $common_perl_bug_test = <IN>;
    close IN;
    unless ( $common_perl_bug_test =~ /[^\s]+/ ) {
        warn "WARNING: Your perl has a bug with regular expressions and UTF-8\n";
        # remove the UTF8 bit from the LANG environment variable and relaunch
        my $lang = $ENV{LANG};
        if ( !$lang ) {
            die "ERROR: ".basename($0)." cannot work around the bug.\n";
        }
        unless ( $lang =~ s/\.UTF-8// ) {
            die "ERROR: ".basename($0)." cannot work around the bug.\n";
        }
        warn "WARNING: ".basename($0)." will attempt to work around the bug by changing your\n".
             "         LANG variable from ".$ENV{LANG}." to $lang.\n";
        $ENV{LANG} = $lang;
        exec($0, @ARGV);
        die "ERROR Could not relaunch ".basename($0)."\n";
    }
}

sub check_script
{
    TRACE and print "Qtopia::Vars::check_script()\n";
    my ( $script, $path, $arg ) = @_;
    #print "check_script $script $path ".(defined($arg)?$arg:"")."\n";
    my $compiled_code = 0;
    if ( $script =~ /\.exe$/i ) {
        $compiled_code = 1;
    }
    if ( $compiled_code && defined($arg) && $arg eq "-nop" ) {
	exit 0;
    }
    if ( configopt("depot") && $compiled_code && !defined($ENV{QTOPIA_NO_PERL}) ) {
        #print "running the perl script ".fixpath("$path/".script_name($script))."\n";
        $script = basename($script);
        $script =~ s/\.exe$//;
        my $ret = system("perl $path/$script ".(scalar(@ARGV)?"\"":"").join("\" \"", @ARGV).(scalar(@ARGV)?"\"":""));
        exit $ret;
    }
}

# Is a particular value in the .configureoptions file
sub configopt
{
    TRACE and print "Qtopia::Vars::configopt()\n";
    if ( ! $depotpath ) {
        #Qtopia::Paths::get_vars();
        die "You must use Qtopia::Paths and call get_paths() before using Qtopia::Vars\n";
    }
    if ( ! @configureoptions ) {
        # Load the .configureoptions file
        my $file = $depotpath."/.configureoptions";
        open IN, $file or die "Can't open $file\n";
        @configureoptions = split(/\s+/, scalar(<IN>));
        close IN;
    }
    my ( $opt ) = @_;
    if ( grep { $_ eq $opt } @configureoptions ) {
	return 1;
    }
    return 0;
}

sub script_name
{
    TRACE and print "Qtopia::Vars::script_name()\n";
    my ( $script ) = @_;
    $script = basename($script);
    my $len = length $script;
    my $compiled_code = 0;
    if ( basename($script) =~ /\.exe$/i ) {
        $compiled_code = 1;
    }
    if ( $compiled_code ) {
	$len -= 4;
    }
    return substr($script, 0, $len);
}

# Make this file require()able.
1;
__END__

=head1 NAME

Qtopia::Vars - Global variables for Qtopia

=head1 SYNOPSIS

    use Qtopia::Vars;
    if ( $isWindows ) {
        print "Windows\n";
    }

=head1 DESCRIPTION

This is just a global variable holder.

=head2 EXPORT

    $isWindows
    $isMac
    $qtopiaVersionStr
    $qtVersionStr
    $shadow
    check_script
    configopt
    script_name

=head1 AUTHOR

Trolltech AS

=head1 COPYRIGHT AND LICENSE

Copyright (C) 2006 TROLLTECH ASA. All rights reserved.

=cut

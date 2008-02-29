package Qtopia::Vars;

use strict;
use warnings;

use File::Basename;
use Qtopia::Paths;
use Qtopia::File;
use Carp;
#perl2exe_include Carp::Heavy
$Carp::CarpLevel = 1;

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
    open IN, "$testfile" or die "Can't read $testfile";
    my $common_perl_bug_test = <IN>;
    close IN;
    unless ( $common_perl_bug_test =~ /[^\s]+/ ) {
        warn "WARNING: Your perl has a bug with regular expressions and UTF-8\n";
        # remove the UTF8 bit from the LANG environment variable and relaunch
        my $lang = $ENV{LANG};
        if ( !$lang ) {
            die "ERROR: ".basename($0)." cannot work around the bug.\n";
        }
        unless ( $lang =~ s/\.UTF-?8//i ) {
            die "ERROR: ".basename($0)." cannot work around the bug.\n";
        }
        warn "WARNING: ".basename($0)." will attempt to work around the bug by changing your\n".
             "         LANG variable from ".$ENV{LANG}." to $lang.\n";
        $ENV{LANG} = $lang;
        exec($0, @ARGV);
        die "ERROR Could not relaunch ".basename($0);
    }

    # RedHat upgraded their perl so that it does not trigger the test above but now
    # it overruns output data, potentially leading to an unexplained segfault.
    # Try to detect RedHat 9 and just force the fix.
    my $redhat_verfile = "/etc/redhat-release";
    if ( -f $redhat_verfile ) {
        open IN, $redhat_verfile or die "Can't read $redhat_verfile";
        $_ = <IN>;
        close IN;
        if ( /release 9/ ) {
            my $lang = $ENV{LANG};
            if ( $lang =~ /\.UTF-?8/i ) {
                warn "WARNING: Your perl has a bug with output and UTF-8\n";
                # remove the UTF8 bit from the LANG environment variable and relaunch
                unless ( $lang =~ s/\.UTF-?8//i ) {
                    die "ERROR: ".basename($0)." cannot work around the bug.\n";
                }
                warn "WARNING: ".basename($0)." will attempt to work around the bug by changing your\n".
                     "         LANG variable from ".$ENV{LANG}." to $lang.\n";
                $ENV{LANG} = $lang;
                exec($0, @ARGV);
                die "ERROR Could not relaunch ".basename($0)."\n";
            }
        }
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
    if ( configopt("depot") && $compiled_code && !$ENV{QTOPIA_NO_PERL} ) {
        $script = script_name($script);
        if ( $script eq "configure" ) {
            print "USING PERL\n";
        }
        #print "running the perl script ".fixpath("$path/$script")."\n";
        my $ret = system("perl $path/$script ".(scalar(@ARGV)?"\"":"").join("\" \"", @ARGV).(scalar(@ARGV)?"\"":""));
        exit $ret;
    } else {
        #print "NOT running the perl script ".fixpath("$path/$script")."\n";
    }
}

# Is a particular value in the .configureoptions file
sub configopt
{
    TRACE and print "Qtopia::Vars::configopt()\n";
    if ( ! $depotpath ) {
        #Qtopia::Paths::get_vars();
        croak "You must use Qtopia::Paths and call get_paths() before using Qtopia::Vars";
    }
    if ( ! @configureoptions ) {
        # Load the .configureoptions file
        my $file = $depotpath."/.configureoptions";
        open IN, $file or die "Can't read $file";
        @configureoptions = split(/\s+/, scalar(<IN>));
        close IN;
        if ( !(grep { $_ eq "depot" } @configureoptions) && -f "$depotpath/LICENSE.GPL" ) {
            push(@configureoptions, "free");
        }
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

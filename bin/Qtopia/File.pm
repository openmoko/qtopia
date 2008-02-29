package Qtopia::File;

use strict;
use warnings;

use File::Basename;
use File::stat;
use File::Path;
use File::Glob;
use File::Copy;
use Qtopia::Vars;

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
    symlink_file
    rmrf
    resolveHeader
    needCopy
    overwriteIfChanged
    fixpath

    $shadow
);

our $VERSION = '0.01';

# imported variables
our $shadow;


# file is symlinked to dest
sub symlink_file
{
    my ($file, $dest, $copy) = @_;

    if ( !defined($copy) && ! -e $file ) {
        warn "WARNING: $file does not exist!\n";
        return;
    }

    my ( $ext ) = ( $file =~ /.*\.(.*)/ );
    if ( !$copy && defined($ext) && $ext eq "h" ) {
        #debugMsg( "magic_include " . $file . " " . $dest );
        my $print_line = "#include \"$file\"\n";
	my $delete_dest = 1;
	if ( -f $dest && ! -l $dest ) {
	    open DEST, "<$dest" or die "Cannot open $dest\n";
	    my $line = <DEST>;
	    close DEST;
	    if ( $line eq $print_line ) { 
		# Don't update the file (it hasn't changed)
		return;
	    }
	}

	if ( -d $dest && ! -l $dest ) {
	    rmrf($dest);
	} else {
	    unlink $dest;
	}

        mkpath(dirname($dest));
	open DEST, ">$dest" or die "Cannot open $dest\n";
	print DEST $print_line;
	close DEST;
        my $s = stat($file);
        if ( defined($s) ) {
            my $now = $s->mtime;
            utime $now, $now, $dest;
        }
    } else {
	if (-d $dest && ! -l $dest) {
	    rmrf($dest);
	} else {
	    unlink $dest;
	}

        mkpath(dirname($dest));
	if ( $isWindows || $copy ) {
	    for ( $file, $dest ) {
		$_ = fixpath($_);
	    }
            #debugMsg( "copy " . $file . " " . $dest );
	    copy($file,$dest);
	} else {
            #debugMsg( "ln -sf " . $file . " " . $dest );
	    system("ln", "-sf", "$file", "$dest");
	}
    }
}

# rm -rf in perl (so that it works in Windows too)
sub rmrf
{
    my ( $path ) = @_;
    for ( glob("$path/*") ) {
	if ( -d $_ && ! -l $_ ) {
	    rmrf($_);
	} else {
	    unlink($_);
	}
    }
    rmdir $path;
}

# Track down a "real" header
sub resolveHeader
{
    my ( $file ) = @_;
    for (;;) {
	open IN, $file or die "Can't read $file\n";
	my $orig = <IN>; 
	if ( !defined($orig) ) {
	    warn "Qtopia::File::resolveHeader() undefined value read from $file\n";
	    return $file;
	}
	chomp $orig;
	close IN;
	if ( $orig =~ /^#include "(.*)"\s*$/ ) {
            my $nf = $1;
            if ( substr($1,0,1) ne "/" ) {
                $nf = dirname($file)."/".$1;
            }
            if ( -e $nf ) {
                $file = $nf;
            } else {
                last;
            }
        } else {
            last;
        }
    }
    return $file;
}

sub needCopy
{
    my ( $srcfile, $dest ) = @_;
    my $src_s = stat($srcfile);
    my $dest_s = stat($dest);
    return ( ! -f $srcfile || ! -f $dest || !defined($src_s) || !defined($dest_s) || $src_s->mtime > $dest_s->mtime );
}

sub overwriteIfChanged
{
    my ( $source, $dest ) = @_;
    open IN, "<$source" or die "Can't open $source\n";
    my $file1 = join("", <IN>);
    close IN;
    my $overwrite = 1;
    if ( -f "$dest" ) {
        open IN, "<$dest" or die "Can't open $dest\n";
        my $file2 = join("", <IN>);
        close IN;
        if ( $file1 eq $file2 ) {
            $overwrite = 0;
        }
    }
    if ( $overwrite ) {
        open OUT, ">$dest" or die "Can't open $dest\n";
        print OUT $file1;
        close OUT;
    }
    unlink $source;
}

sub fixpath
{
    my ( $path ) = @_;
    if ( $isWindows && $path ) {
        $path =~ s,/,\\,g;
        $path = lc(substr($path, 0, 1)).substr($path, 1);
    }
    return $path;
}

sub unixpath
{
    my ( $path ) = @_;
    if ( $isWindows && $path ) {
        $path =~ s,\\,/,g;
        $path = lc(substr($path, 0, 1)).substr($path, 1);
    }
    return $path;
}

# Make this file require()able.
1;
__END__

=head1 NAME

Qtopia::File - File management routines for Qtopia

=head1 SYNOPSIS

    use Qtopia::File;
    symlink_file($source, $dest, $copy);
    rmrf($dir);
    resolveHeader($file);
    needCopy($source, $dest);
    overwriteIfChanged($source, $dest);
    fixpath($path);

=head1 DESCRIPTION

This module contains helper functions that do file management operations.
The funtions here work on Windows and Unix.

=head2 EXPORT

    symlink_file
    rmrf
    resolveHeader
    needCopy
    overwriteIfChanged
    fixpath

=head1 AUTHOR

Trolltech AS

=head1 COPYRIGHT AND LICENSE

Copyright (C) 2006 TROLLTECH ASA. All rights reserved.

=cut

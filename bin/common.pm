# Common perl functions
#
# Copyright 2004 Trolltech AS.  All rights reserved.
#

# Perl Module boilerplate
#package main;
#use Exporter;
#@ISA = ('Exporter');
#@EXPORT = qw(&debugMsg &symlink_file &rmrf);
#1;

# Print a message if user requested to see extra messages
sub debugMsg
{
    if ( $opt_verbose ) {
	print @_;
	print "\n";
    }
}

# file is symlinked to dest
sub symlink_file
{
    my ($file, $dest) = @_;
    # relative paths if we're not on Windows or Shadow building
    if ( !$isWindows && !$shadow ) {
	my $targetDir = $dest;
	$targetDir =~ s,^$outpath/?,,;

	my $count = ($targetDir =~ tr,/,,);
	my $p = "";
	for (my $i = 0;$i < $count;$i++) {
	    $p = $p."../";
	}
	$file =~ s,^$outpath/?,,;
	$file = $p.$file;
    }

    my ( $ext ) = ( $file =~ /.*\.(.*)/ );
    if ( defined($ext) && $ext eq "h" ) {
	debugMsg( "magic_include " . $file . " " . $dest );
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

	open DEST, ">$dest" or die "Cannot open $dest\n";
	print DEST $print_line;
	close DEST;
    } else {
	if (-d $dest && ! -l $dest) {
	    rmrf($dest);
	} else {
	    unlink $dest;
	}

	if ( $isWindows ) {
	    for ( $file, $dest ) {
		s,/,\\,g;
	    }
	    debugMsg( "copy " . $file . " " . $dest );
	    copy($file,$dest);
	} else {
	    debugMsg( "ln -sf " . $file . " " . $dest );
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

# Load the .configureoptions file
open IN, "$depotpath/.configureoptions" or die "Can't open $depotpath/.configureoptions\n";
my @configureoptions = split(/\s+/, scalar(<IN>));
close IN;

# Is a particular value in the .configureoptions file
sub configopt
{
    my ( $opt ) = @_;
    if ( grep { $_ eq $opt } @configureoptions ) {
	return 1;
    }
    return 0;
}

# Make this file require()able
1;


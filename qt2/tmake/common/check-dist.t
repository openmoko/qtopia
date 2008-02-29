#!
#! This is a template for checking that a distribution is consistent.
#!
#! Usage:
#!	cd $QTDIR/src
#!	tmake qtx11.pro -t check-dist
#!

#${
    @f = split(/\s+/, Project("HEADERS","SOURCES"));
    foreach ( @f ) { $f_dict{$_} = 1; }
    @ff = find_files(".","\.cpp|\.h",1);
    push(@ff,find_files("../include","\.cpp|\.h",1)) if ( ! $is_unix );
    foreach ( @ff ) { $ff_dict{$_} = 1; }
    $e = 0;
    foreach ( @f ) {
	if ( ! $ff_dict{$_} ) {
	    print "File $_ not found\n";
	    $e++;
	}
    }
    foreach ( @ff ) {
	if ( ! $f_dict{$_} ) {
	    print "File $_ should not be in the distribution\n";
	    $e++;
	}
    }
    if ( $e > 0 ) {
	print STDERR "$e errors found.\n";
    } else {
	print STDERR "No errors found.\n";
    }
#$}

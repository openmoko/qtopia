package Qtopia::Opt;

use strict;
use warnings;

use File::Basename;
use Getopt::Long;
use FileHandle;
use Qtopia::Cache;

require Exporter;
our @ISA = qw(Exporter);

# Items to export into callers namespace by default. Note: do not export
# names by default without a very good reason. Use EXPORT_OK instead.
# Do not simply export all your public functions/methods/constants.

# This allows declaration	use Qtopia::Opt ':all';
# If you do not need this, moving things directly into @EXPORT or @EXPORT_OK
# will save memory.
our %EXPORT_TAGS = ( 'all' => [ qw(
) ] );

our @EXPORT_OK = ( @{ $EXPORT_TAGS{'all'} } );

our @EXPORT = qw(
    opt_sanity_check
    opt_get_options
    opt_apply_defaults
    opt_resolve
    opt_print_autodetect
    set_optvar
    opt
    add_separator
    add_note
);

our $VERSION = '0.01';

# opt variables
our %optvar_storage;
our @ordered_optvar_keys;

# help variables
our $optiondesc;
our $optionhelp;
our $optionstar;
our $optiondef;
our $optionavail;

my $cols = $ENV{COLUMNS};
$cols = 80 unless ( $cols );
# the width of the first column (calculated in _init_formats)
my $fcwidth;

# Do a sanity check on the opt_ variables
sub opt_sanity_check()
{
    OPT: for my $optname ( keys %optvar_storage ) {
        my $optref = $optvar_storage{$optname};
        if ( scalar(keys %$optref) == 0 ) {
            print "$optname has no attributes!\n";
            next OPT;
        }
        if ( $optref->{"role"} && $optref->{"role"} eq "placeholder" ) {
            # don't check placeholders because they won't have anything set
            next OPT;
        }
        my $setref = $optref->{"set"};
        my $unsetref = $optref->{"unset"};
        my $arg = $optref->{"arg"};
        my $type = $optref->{"type"};
        if ( !defined($setref) && !defined($unsetref) ) {
            print "$optname has no set/get attribute!\n";
            next OPT;
        }
        for my $set ( qw(set unset) ) {
            my $ref = eval '$'.$set.'ref';
            if ( defined($ref) ) {
                if ( ref($ref) eq "ARRAY" ) {
                    if ( scalar(@$ref) != 2 ) {
                        print "$optname->$set must be a 2-element array!\n";
                    }
                } else {
                    print "$optname->$set is not an array!\n";
                }
            }
        }
    }
}

# Get options and print help
sub opt_get_options
{
    my $dohelp = 1;
    for ( @_ ) {
        if ( $_ eq "nohelp" ) {
            $dohelp = 0;
        }
    }

    # Setup the GetOptions info
    my @optl;
    for my $optname ( keys %optvar_storage ) {
        my $optref = $optvar_storage{$optname};
        my $vis = _resolve_to_scalar($optref->{"visible"});
        if ( !defined($vis) || $vis || 1 ) { #$optref->{"silentignore"} ) {
            for my $set ( qw(set unset) ) {
                my $ref = $optref->{"$set"};
                if ( defined($ref) && ref($ref) eq "ARRAY" ) {
                    my $paramstring = $$ref[0];
                    if ( index($paramstring, "%") != -1 ) {
                        my $paramname = $optname;
                        $paramname =~ s/_/-/g;
                        $paramstring =~ s/%/$paramname/g;
                    }
                    my $funcref = $optref->{$set."func"};
                    if ( !defined($funcref) ) {
                        if ( !defined($optref->{"value"}) ) {
                            if ( defined($optref->{"type"}) && $optref->{"type"} eq '@' ) {
                                my $listref = [];
                                $optref->{"value"} = $listref;
                                $funcref = $listref;
                            } elsif ( defined($optref->{"type"}) && $optref->{"type"} eq 'multi-value' ) {
                                $funcref = sub { opt($optname) = $_[1]; opt($optname) =~ s/,/ /g; };
                            } else {
                                # FIXME should this really be here?
                                $optref->{"value"} = undef;
                            }
                        }
                        if ( !defined($funcref) ) {
                            if ( $set eq "set" ) {
                                $funcref = \$optref->{"value"};
                            } else {
                                $funcref = sub { opt($optname) = 0 };
                            }
                        }
                    }
                    #print "Adding -$paramstring for $optname\n";
                    push(@optl, $paramstring => $funcref);
                    # Aliases (for backwards compatibility)
                    my $aliasref = $optref->{$set."aliases"};
                    if ( defined($aliasref) && ref($aliasref) eq "ARRAY" ) {
                        for my $alias ( @$aliasref ) {
                            push(@optl, $alias => $funcref);
                        }
                    }
                }
            }
        }
    }
    Getopt::Long::Configure("bundling_override");
    my $ok = GetOptions(@optl);

    # check input against "available", if present
    AVAILCHECK: for my $optname ( keys %optvar_storage ) {
        my $optref = $optvar_storage{$optname};
        if ( defined($optref->{"value"}) && $optref->{"available"} && ref($optref->{"available"}) ne "" ) {
            my @available = _resolve_to_array($optref->{"available"});
            AVAILWORD: for my $word ( split(/ /, $optref->{"value"}) ) {
                if ( $word ) {
                    for my $a ( @available ) {
                        if ( $word eq $a ) {
                            next AVAILWORD;
                        }
                    }
                    warn "Invalid value for option \"$optname\": $word\n";
                    $ok = 0;
                    next AVAILCHECK;
                }
            }
        }
    }

    if ( !$dohelp ) {
        return $ok;
    }
    if ( !$ok || (exists($optvar_storage{"help"}) && opt("help")) ) {
        get_help();
    }
}

sub get_help()
{
    # print out some help
    my @helpinfo;
    for my $optname ( @ordered_optvar_keys ) {
        if ( $optname =~ /^---/ ) {
            push(@helpinfo, $optname);
            next;
        }
        my $optref = $optvar_storage{$optname};
        my $vis = _resolve_to_scalar($optref->{"visible"});
        if ( !defined($vis) || $vis ) {
            my @info = ();
            my $count = 0;
            for my $set ( qw(set unset) ) {
                my $ref = $optref->{$set};
                if ( defined($ref) ) {
                    my $paramstring = @$ref[0];
                    my $paramhelp = @$ref[1];
                    my @paramavail = ();
                    my $paramdef = "";
                    my $paramstar = 0;
                    if ( $paramhelp eq "hidden" ) {
                        next;
                    }
                    if ( index($paramstring, "%") != -1 ) {
                        my $paramname = $optname;
                        $paramname =~ s/_/-/g;
                        $paramstring =~ s/%/$paramname/g;
                    }
                    if ( $paramstring =~ s/=.*// ) {
                        my $arg = $optref->{"arg"};
                        if ( !defined($arg) ) {
                            $arg = "arg";
                        }
                        $paramstring .= " $arg";
                        push(@paramavail, _resolve_to_array($optref->{"available"}));
                        $paramdef = _resolve_to_scalar($optref->{"default"});
                        $paramdef = "" if ( !defined($paramdef) );
                        if ( $optref->{"type"} && $optref->{"type"} eq "multi-value" ) {
                            $paramdef =~ s/\s+/,/g;
                        }
                    } else {
                        my $def = _resolve_to_scalar($optref->{"default"});
                        if ( ($set eq "set" && $def) ||
                             ($set eq "unset" && defined($def) && !$def) ) {
                            $paramstar = 1;
                        }
                    }
                    if ( exists($optvar_storage{"help"}) && opt("help") ) {
                        push(@info, "-$paramstring", $paramhelp, $paramstar, $paramdef, join(", ", @paramavail));
                    } else {
                        push(@info, "-$paramstring");
                    }
                    $count++;
                }
            }
            if ( scalar(@info) == 1 ) {
                push(@helpinfo, "[ ".$info[0]." ]");
            } elsif ( scalar(@info) == 2 ) {
                push(@helpinfo, "[ ".$info[0]." | ".$info[1]." ]");
            } else {
                push(@helpinfo, "---") if ( $count == 2 );
                push(@helpinfo, @info);
                push(@helpinfo, "---") if ( $count == 2 );
            }
        }
    }
    # Setup the terminal-width-dependant formats;
    if ( exists($optvar_storage{"help"}) && opt("help") ) { 
        _init_formats(@helpinfo);
        print "Usage:  ".basename($0)." [options]

The defaults (*) are usually acceptable. A plus (+) denotes a default
value that needs to be evaluated. If the evaluation succeeds, the
feature is included. Here is a short explanation of each option:

";

        my $spacer = 0;
        while ( @helpinfo ) {
            $optiondesc = shift(@helpinfo);
            if ( $optiondesc =~ s/^---// ) {
                if ( $optiondesc ) {
                    $spacer = 0;
                    format_name STDOUT "NOTE";
                    write;
                } elsif ( !$spacer ) {
                    $spacer = 1;
                    print "\n";
                }
                next;
            }
            $spacer = 0;
            if ( length($optiondesc) < $fcwidth ) {
                $optiondesc .= " ";
                while ( length($optiondesc) < $fcwidth ) {
                    $optiondesc .= ".";
                }
            }
            $optionhelp = shift(@helpinfo);
            $optionstar = shift(@helpinfo)?"*":" ";
            $optiondef = shift(@helpinfo);
            $optionavail = shift(@helpinfo);
            format_name STDOUT "LONGHELP";
            write;
        }
        print "\n";
    } else {
        my $header = "Usage:  ".basename($0)." ";
        my $leader = "                  ";
        my $col = length($header);
        print $header;
        for my $option ( @helpinfo ) {
            if ( $option =~ s/^---// ) {
                # skip notes
                next;
            }
            if ( length($option) + $col > ($cols-1) ) {
                print "\n$leader";
                $col = length($leader);
            }
            print "$option ";
            $col += length($option) + 1;
        }
        print "\n\n        Pass -help for a detailed explanation of each option.\n\n";
    }

    exit 2;
}

# Apply defaults
sub opt_apply_defaults
{
    for my $optname ( @_, keys %optvar_storage ) {
        my $optref = $optvar_storage{$optname};
        my $auto = _resolve_to_scalar($optref->{"autodep"});
        my $def = _resolve_to_scalar($optref->{"default"});
        if ( !defined($optref->{"value"}) && defined($def) ) {
            if ( $optref->{"type"} && $optref->{"type"} eq "multi-value" ) {
                $def =~ s/,/ /g;
            }
            if ( !defined($auto) || $auto ) {
                $optref->{"value"} = $def;
                my $sa = _resolve_to_scalar($optref->{"showauto"});
                my $vis = _resolve_to_scalar($optref->{"visible"});
                if ( (!defined($vis) || $vis) && $sa ) {
                    $optref->{"auto"} = 1;
                }
            }
        }
    }
}

# Return the value that the option would have, if defaults had been applied
sub opt_resolve($)
{
    my ( $optname ) = @_;
    my $optref = $optvar_storage{$optname};
    my $auto = _resolve_to_scalar($optref->{"autodep"});
    if ( defined($optref->{"value"}) ) {
        return $optref->{"value"};
    }
    if ( !defined($optref->{"value"}) && defined($optref->{"default"}) ) {
        my $defref = $optref->{"default"};
        my $def;
        if ( ref($defref) ne "CODE" ) {
            $def = $defref;
        } elsif ( !defined($auto) || $auto ) {
            if ( ref($defref) eq "CODE" ) {
                $def = &$defref();
            }
        }
        if ( $def && $optref->{"type"} && $optref->{"type"} eq "multi-value" ) {
            $def =~ s/,/ /g;
        }
        return $def;
    }
    return undef;
}

# Print out the autodetect message
sub opt_print_autodetect()
{
    my $haveauto = 0;
    for my $optname ( @ordered_optvar_keys ) {
        if ( $optname =~ /^---/ ) {
            next;
        }
        my $optref = $optvar_storage{$optname};
        my $setref = $optref->{"set"};
        my $unsetref = $optref->{"unset"};
        my $line;
        my $value = $optref->{"value"};
        if ( !defined($value) ) {
            $value = "";
        }
        if ( $optref->{"type"} && $optref->{"type"} eq "multi-value" ) {
            $value =~ s/\s+/,/g;
        }
        if ( ($setref && index($$setref[0], "=") == -1) ||
             ($unsetref && index($$unsetref[0], "=") == -1) ) {
            if ( $value ) {
                $line = $$setref[0];
            } else {
                $line = $$unsetref[0];
            }
            if ( !$line ) {
                next;
            }
            my $paramname = $optname;
            $paramname =~ s/_/-/g;
            $line =~ s/%/$paramname/g;
        } elsif ( $setref && index($$setref[0], "=") != -1 ) {
            $line = substr($$setref[0], 0, index($$setref[0], "="))." ";
            my $paramname = $optname;
            $paramname =~ s/_/-/g;
            $line =~ s/%/$paramname/g;
            if ( defined($optref->{"type"}) && $optref->{"type"} eq '@' ) {
                if ( @$value ) {
                    $line .= "'".join("' '", @$value)."'";
                } else {
                    $line = undef;
                }
            } else {
                if ( $value ) {
                    $line .= "'".$value."'";
                } else {
                    $line = undef;
                }
            }
        } else {
            next;
        }
        if ( !defined($line) ) {
            next;
        }
        $line =~ s/%/$optname/;
        if ( $optref->{"auto"} ) {
            if ( !$haveauto ) {
                $haveauto = 1;
                print "The following default/detected values have been used:\n";
            }
            print "    -$line\n";
        }
    }
    if ( $haveauto ) {
        print "\n";
    }
}

# Create an opt_ variable
sub set_optvar($$)
{
    my ( $optname, $hashref ) = @_;
    if ( exists($optvar_storage{$optname}) ) {
        die "opt var $optname already exists!\n";
    }
    $optvar_storage{$optname} = $hashref;
    push(@ordered_optvar_keys, $optname);
}

# Shorthand to access the value
sub opt : lvalue
{
    if ( scalar(@_) < 1 ) {
        die "You must supply a name to opt()!\n";
    }
    my $optname = $_[0];
    my $key = "value";
    if ( scalar(@_) == 2 ) {
        $key = $_[1];
    }
    if ( !exists($optvar_storage{$optname}) ) {
        die "opt var $optname does not exist!\n";
    }
    my $optref = $optvar_storage{$optname};
    $optref->{$key};
}

sub add_separator()
{
    if ( $ordered_optvar_keys[$#ordered_optvar_keys] ne "---" ) {
        push(@ordered_optvar_keys, "---");
    }
}

sub add_note($)
{
    my ( $note ) = @_;
    add_separator();
    push(@ordered_optvar_keys, "---$note");
    add_separator();
}

# Dump the opt_ variables to config.cache
sub write_config_cache()
{
    my %ignored_attributes = map { $_ => 1 } qw(set unset setfunc unsetfunc setaliases arg visible autodep);
    my $ret = "";
    OPT: for my $optname ( keys %optvar_storage ) {
        my $noted = 0;
        my $optref = $optvar_storage{$optname};
        ATTR: for my $attribute ( grep { !exists($ignored_attributes{$_}) } keys %$optref ) {
            my $ref = $optref->{$attribute};
            my $value;
            if ( !defined($ref) ) {
                $value = "undef";
            } elsif ( ref($ref) eq "" ) {
                $value = $ref;
            } elsif ( ref($ref) eq "ARRAY" ) {
                for ( my $i = 0; $i < scalar(@$ref); $i++ ) {
                    $value = $$ref[$i];
                    $value = "undef" unless defined($value);
                    $noted = 1;
                    $value =~ s/\n/\\n/g;
                    $ret .= "opt.$optname.$attribute.\[$i\]=$value\n";
                }
                next ATTR;
            } elsif ( ref($ref) eq "CODE" ) {
                $value = &$ref();
            } else {
                $value = $ref;
            }
            if ( defined($value) ) {
                $noted = 1;
                $value =~ s/\n/\\n/g;
                $ret .= "opt.$optname.$attribute=$value\n";
            }
        }
        if ( !$noted ) {
            # Add a bogus entry for this one so it appears in config.cache
            $ret .= "opt.$optname.value=undef\n";
        }
    }
    Qtopia::Cache::replace("opt", $ret);
    return $ret;
}

# Load the opt_ variables from config.cache
sub read_config_cache()
{
    # clear out anything that's in there now
    %optvar_storage = ();

    my @cache = Qtopia::Cache::load("opt");
    for ( @cache ) {
        if ( /^opt\.([^\.=]+)\.([^\.=]+)([^=]*)=(.*)/ ) {
            my $optname = $1;
            my $attribute = $2;
            my $extra = $3;
            my $value = $4;
            $value =~ s/\\n/\n/g;
            if ( $value eq "undef" ) {
                $value = undef;
            }
            my $optref = $optvar_storage{$optname};
            if ( !$optref ) {
                $optref = $optvar_storage{$optname} = +{};
                push(@ordered_optvar_keys, $optname);
            }
            if ( $extra ) {
                # currently we only support @
                if ( $extra =~ /\.\[([0-9]+)\]/ ) {
                    my $i = $1;
                    my $ref = $optref->{$attribute};
                    if ( !$ref ) {
                        $ref = $optref->{$attribute} = [];
                    }
                    $$ref[$i] = $value;
                }
            } else {
                $optref->{$attribute} = $value;
                if ( $attribute eq "type" && $value eq '@' ) {
                    if ( ref($optref->{"value"}) ne "ARRAY" ) {
                        $optref->{"value"} = [];
                    }
                }
            }
        }
    }
}

sub _resolve_to_scalar
{
    my ( $ref ) = @_;

    my $ret;

    if ( !defined($ref) ) {
        $ret = undef;
    } elsif ( ref($ref) eq "ARRAY" ) {
        $ret = join(" ", @$ref);
    } elsif ( ref($ref) eq "CODE" ) {
        $ret = &$ref();
    } else {
        $ret = $ref;
    }

    $ret;
}

sub _resolve_to_array
{
    my ( $ref ) = @_;

    my @ret;

    if ( !defined($ref) ) {
        @ret = ();
    } elsif ( ref($ref) eq "CODE" ) {
        @ret = &$ref();
    } elsif ( ref($ref) eq "ARRAY" ) {
        @ret = @$ref;
    } else {
        push(@ret, $ref);
    }

    @ret;
}

sub _init_formats
{
    my @helpinfo = @_;

    # work out a good width for the first column
    $fcwidth = 24;

    my $qscr = $cols / 4;
    if ( $qscr > 24 ) {
        while ( @helpinfo ) {
            $optiondesc = shift(@helpinfo);
            next if ( $optiondesc =~ s/^---// );

            $optionhelp = shift(@helpinfo);
            $optionstar = shift(@helpinfo);
            $optiondef = shift(@helpinfo);
            $optionavail = shift(@helpinfo);

            if ( length($optiondesc) > $fcwidth ) {
                $fcwidth = length($optiondesc);
            }
        }
        if ( $fcwidth > $qscr ) {
            $fcwidth = $qscr;
        }
    }

    my $scwidth = $cols - $fcwidth - 7;
    my $fmt = "format LONGHELP =\n".
              '  @ ^'.'<'x($fcwidth).' ^'.'<'x($scwidth)."\n".
              '  $optionstar, $optiondesc,  $optionhelp'."\n".
              '~~   ^'.'<'x($fcwidth-1).' ^'.'<'x($scwidth)."\n".
              '     $optiondesc,            $optionhelp'."\n".
              '~     '.' 'x($fcwidth-1).' Available: ^'.'<'x($scwidth-11)."\n".
              '                             $optionavail'."\n".
              '~~    '.' 'x($fcwidth-1).' ^'.'<'x($scwidth)."\n".
              '                             $optionavail'."\n".
              '~     '.' 'x($fcwidth-1).' Default: ^'.'<'x($scwidth-9)."\n".
              '                             $optiondef'."\n".
              '~~    '.' 'x($fcwidth-1).' ^'.'<'x($scwidth)."\n".
              '                             $optiondef'."\n".
              ".\n";
    eval $fmt;
    die $@ if ( $@ );

    $fmt = "format NOTE =\n".
           '~~      ^'.'<'x($cols-10)."\n\$optiondesc\n.";
    eval $fmt;
    die $@ if ( $@ );

    # Only split on spaces, not the - character
    $: = " ";
}

# Make this file require()able.
1;
__END__

=head1 NAME

Qtopia::Opt - Option handling system

=head1 SYNOPSIS

    use Qtopia::Opt;
    set_optvar(+{
        ...
    });
    opt_get_options();
    opt_apply_defaults();
    opt_print_autodetect();

=head1 DESCRIPTION

Please see doc/src/buildsys/buildsystem-internals.qdoc for information about the opt system.

=head2 EXPORT

    opt_sanity_check
    opt_get_options
    opt_apply_defaults
    opt_resolve
    opt_print_autodetect
    set_optvar
    opt
    add_separator
    add_note

=head1 AUTHOR

Trolltech AS

=head1 COPYRIGHT AND LICENSE

Copyright (C) 2006 TROLLTECH ASA. All rights reserved.

=cut

package Qtopia::Cache;

use strict;
use warnings;

use Cwd;
use File::Basename;
use Qtopia::Paths;
use Qtopia::Vars;
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
    get_vars
) ] );

our @EXPORT_OK = ( @{ $EXPORT_TAGS{'all'} } );

our @EXPORT = qw(
    $depotpath
    $QPEDIR
    $qt_depot_path
    $QTEDIR
    $DQTDIR
);

our $VERSION = '0.01';

# imported variables
our $depotpath;
our $QPEDIR;
our $qt_depot_path;
our $QTEDIR;
our $DQTDIR;


use constant TRACE => 0;

sub read_config_cache()
{
    TRACE and print "Qtopia::Cache::read_config_cache()\n";
    if ( ! $QPEDIR ) {
        croak "You must load Qtopia::Paths and call get_paths() before using Qtopia::Cache\n";
    }
    my $config_cache_file = $QPEDIR."/config.cache";
    if ( ! -f $config_cache_file ) {
        croak "You must run configure before you can run ".basename($0)."\n";
    }
    open CACHE, "$config_cache_file" or die "Can't read $config_cache_file";
    my @cache_data = <CACHE>;
    chomp(@cache_data);
    close CACHE;
    return @cache_data;
}

sub write_config_cache(@)
{
    TRACE and print "Qtopia::Cache::write_config_cache()\n";
    my ( @cache ) = @_;

    if ( ! $QPEDIR ) {
        croak "You must load Qtopia::Paths and call get_paths() before using Qtopia::Cache\n";
    }
    my $config_cache_file = $QPEDIR."/config.cache";
    if ( -f $config_cache_file ) {
        unlink $config_cache_file or die $!;
    }
    open CACHE, ">".$config_cache_file or die "Can't write $config_cache_file";
    print CACHE join("\n", grep { ! /^\s*$/ } @cache)."\n";
    close CACHE;
}

sub replace($$)
{
    TRACE and print "Qtopia::Cache::replace()\n";
    my ( $prefix, $data ) = @_;
    if ( ! $QPEDIR ) {
        croak "You must load Qtopia::Paths and call get_paths() before using Qtopia::Cache\n";
    }
    my @cache;
    if ( -f $QPEDIR."/config.cache" ) {
        @cache = grep { ! /^\Q$prefix\E\./; } read_config_cache();
    }
    push(@cache, split(/\n/, $data));
    write_config_cache(@cache);
}

sub load
{
    TRACE and print "Qtopia::Cache::load()\n";
    my ( $prefix ) = @_;
    my @cache = read_config_cache();
    if ( $prefix ) {
        @cache = grep { /^\Q$prefix\E\./; } @cache;
    }
    return @cache;
}

# Make this file require()able.
1;
__END__

=head1 NAME

Qtopia::Cache - Support for reading/writing config.cache

=head1 SYNOPSIS

    use Qtopia::Cache;
    Qtopia::Cache::

=head1 DESCRIPTION

This is just a global variable holder.

=head2 EXPORT

    $QPEDIR
    $depotpath
    $QTEDIR
    $DQTDIR
    $qt_depot_path
    $cwd
    $isWindows
    $isMac

=head1 AUTHOR

Trolltech AS

=head1 COPYRIGHT AND LICENSE

Copyright (C) 2006 TROLLTECH ASA. All rights reserved.

=cut

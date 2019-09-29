########################################################################################################################
#
#  Usage: .\truncate_file.pl $PATH_TO_FILE
#
#  Truncates a file to 0 bytes
#  .\truncate_file.pl e:\PXE\Tests.module 
########################################################################################################################

use strict;
use warnings;

# a.k.a main
die "Usage $0 \$PATH_TO_FILE" if @ARGV != 1;

open(my $fh, '>', $ARGV[0]) or die "Could not create $ARGV[0]";
close $fh;
########################################################################################################################
#
#  Usage: .\generate_modules.pl $PATH_TO_OUTPUT_FILE $TEST_PREFIX /$TEST_SUFFIX $FIRST_TEST_NAME [... $NTH_TEST_NAME]
#
#  If you want to generate the modules for the GoodPriority and BadPriority tests in the file E:\PXE\Tests.module
#  .\generate_modules.pl E:\PXE\Tests.module GoodPriority BadPriority
########################################################################################################################

use strict;
use warnings;

# GenerateBuffer
#
# Creates a buffer containing commands to run all the tests received as input (3rd parameter)
#
# 1st Parameter: prefix appended to each line
# 2nd Parameter: suffix appended to each line (may be a simple /)
# 3rd Parameter: reference to an array of strings holding the names of the tests
# Return: buffer containing the commands
sub GenerateBuffer
{
    my $buffer = "";

    my $val = $_[2];

    $buffer .= "/log ON\n";
    $buffer .= "/loglevel 0\n";
    $buffer .= "/logcomp 8000\n";

    $buffer = $buffer . $_[0] . " $_ " . ((split('/', $_[1]))[-1] or "") . "\n" for (@$val);
    $buffer .= "/shutdown\n";

    return $buffer;
}

# a.k.a main
die "Usage $0 \$PATH_TO_OUTPUT_FILE \$TEST_PREFIX /\$TEST_SUFFIX \$FIRST_TEST_NAME [... \$NTH_TEST_NAME]" if @ARGV < 4;

my $pathToOutputFile = shift @ARGV;
my $testPrefix = shift @ARGV;
my $testSuffix = shift @ARGV;

my $dataBuffer = GenerateBuffer($testPrefix, $testSuffix, \@ARGV);

open(my $fh, '>', $pathToOutputFile) or die "Could not open $pathToOutputFile";
print $fh $dataBuffer;
close $fh;

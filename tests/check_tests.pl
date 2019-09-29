########################################################################################################################
#
#  Usage: .\checks_tests.pl $PATH_TO_FIRST_TEST [... $PATH_TO_NTH_TEST]
#
#  If you want to check the result of some test(s):
#  .\checks_tests.pl e:\WORKSPACE\PROJECTS\VS2015\HAL9000\TESTS\threads\priority-scheduler\CpuIntense.test e:\WORKSPACE\PROJECTS\VS2015\HAL9000\TESTS\threads\priority-scheduler\ThreadPriorityWakeup.test
#
#  This script is called by tokenize_log.pl so there should be no reason for it to be called on its own. It does not
#  function if the log was not parsed and the .result files were not generated;
########################################################################################################################

use strict;
use warnings;

# CheckFile
#
# Checks the result of the specified test (1st parameter) by comparing the generated .result file with the
# expected .test file. Returns 1 if the test succeeded and 0 if the test failed. Also, an .outcome file will be
# generated storing the final result (PASS or FAIL) and a detailed failure reason.
#
# 1st Parameter: A string specifying the full path of the test without the extesion
# Return: 1 if test succeeded, 0 if test failed, further information is available in the .outcome file
sub CheckFile
{
    my $result;
    my $baseFileName = $_[0];
    my $outComefileName = $baseFileName . '.outcome';
    my $expectedFileName = $baseFileName . '.test';
    my $resultFileName = $baseFileName . '.result';
    my $checkerFileName = $baseFileName . '.check';

    open(my $fh, '>', $outComefileName) or die "Could not create $outComefileName file!";
    open(my $rh, '<', $resultFileName) or print $fh, "[FAIL]Could not open result file\n", return 0;

    while (<$rh>)
    {
        $result = 0, print $fh $_ if $_ =~ /(\[(?>ERROR|CRITICAL)\].*)/;
        $result = 1, print $fh $_ if $_ =~ /(\[PASS\].*)/;

        last if defined $result;
    }

    close $fh;

    if (not defined $result)
    {
        my $checker = open($fh, '<', $checkerFileName) ? (close $fh, "perl $checkerFileName") : "fc";

        # do a compare
        system("$checker \"$resultFileName\" \"$expectedFileName\" > \"$outComefileName\"");

        $result = ($? == 0);
    }

    open($fh, '>>', $outComefileName) or die "Could not create $outComefileName file!";
    print $fh (($result == 1) ? "[PASS]\n" : "[FAIL]\n");

    return $result;
}

# a.k.a main
die "Script usage is $0 \$PATH_TO_FIRST_TEST [... \$PATH_TO_NTH_TEST]" if @ARGV < 1;

my $totalTests = @ARGV;
my $passedTests = 0;
my %categoryResults;
my @categoryArray;

print "\n\n";

foreach (@ARGV)
{
    my $test = $_;

    my $result = CheckFile $test;
    my $testName = (split(/\\/, $test))[-1];
    my $testCategory = (split(/\\/, $test))[-2];

    if (not defined $categoryResults{$testCategory})
    {
        push @{$categoryResults{$testCategory}}, 0, 0;
        push @categoryArray, $testCategory;
        print "\n";
    }

    $categoryResults{$testCategory}[0] += $result;
    $categoryResults{$testCategory}[1] += 1;

    printf("%-60s %12s\n", "$testName result is...", ($result == 0 ? "FAIL" : "PASS"));

    $passedTests += $result;
}

printf("\n%-60s %10U/%-10U\n", "Tests results:", $passedTests, $totalTests);
print "Results per test category\n";

foreach (@categoryArray)
{
    printf("%-60s %10U/%-10U\n", "Results for $_:", $categoryResults{$_}->[0], $categoryResults{$_}->[1]);
}

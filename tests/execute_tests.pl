########################################################################################################################
#
#  Usage: .\execute_tests.pl $PATH_TO_TESTS_FOLDER|$PATH_TO_TEST_FILE $PATH_TO_LOG $PATH_TO_VIX $PATH_TO_VMX $PATH_TO_PLACE_MODULE $CMD_PREFIX /$CMD_SUFFIX [$SECONDS_TO_WAIT_FOR_TERMINATION]
#
#  If you want to run all the threads test you will issue the following command:
#  .\execute_tests.pl E:\WORKSPACE\PROJECTS\VS2015\HAL9000\TESTS\THREADS\ E:\WORKSPACE\SERIAL.LOG C:\VMWARE\VIX E:\WORKSPACE\VMS\HAL9000.vmx E:\PXE\Tests.module /run 16
#
#  If you want to run a single test:
#  .\execute_tests.pl e:\WORKSPACE\PROJECTS\VS2015\HAL9000\TESTS\THREADS\priority-scheduler\ThreadPriorityWakeup.test E:\WORKSPACE\SERIAL.LOG C:\VMWARE\VIX E:\WORKSPACE\VMS\HAL9000.vmx E:\PXE\Tests.module /run 16
########################################################################################################################

use strict;
use warnings;

#suppress smartmatch warnings
no if $] >= 5.017011, warnings => 'experimental::smartmatch';

# CategorizeTests
#
# 1st Parameter: Path to tests folder (e:\WORKSPACE\PROJECTS\VS2015\HAL9000\TESTS\THREADS\)\
# 2nd Parameter: If 1 => will delete previous results, if 0 old results will remain
# Return: Reference to a hash table mapping each test to a category (determined by the folder in which the test was
#         found)
# Return: Reference to an array containing each test
sub CategorizeTests
{
    my @dirs;
    my %tests;
    my @allTests;

    push @dirs, $_[0];

    while (my $cdir = shift @dirs)
    {
        my $dirName = (split('/', $cdir))[-1];

        opendir(DIR, $cdir) or die "Cannot open directory $cdir\n";
        my @files = readdir(DIR);
        closedir(DIR);

        foreach my $file (@files)
        {
            my $fullPath = $cdir . '/' . $file;

            if (-d $fullPath and ($file !~ /^\.\.?$/))
            {
                push @dirs, $fullPath;
            }
            unlink($fullPath) if ($_[1] and ($file =~ /\.(?>outcome|result)$/));
            next if ($file !~ /\.test$/);

            my $testName = (split(/\./, lc $file))[0];

            $tests{$testName} = $dirName;
            push @allTests, $testName;
        }
    }

    return (\%tests, \@allTests);
}

# ParseLog
#
# Will parse the HAL9000 log (1st param) and will generate a .result file for each test(2nd param)
# found in the log in the appropriate folder (3rd param + 4th param)
#
# 1st Parameter: Path to the log
# 2nd Parameter: Reference to an array of tests
# 3rd Parameter: Base directory where all the tests are found (e:\WORKSPACE\PROJECTS\VS2015\HAL9000\TESTS\THREADS\)
# 4th Parameter: Reference to a hash table providing a mapping from tests to categories
# Return: VOID
sub ParseLog
{
    my $fh;
    my $curTest;

    open(my $lg, '<', $_[0]) or die sprintf("Could not open file %s!", $_[0]);

    while (<$lg>)
    {
        if (defined $curTest)
        {
            $curTest = undef, close $fh if $_ =~ /Test \[$curTest\] END!/i;
            print $fh $_ if $curTest;
        }

        next unless $_ =~ /Test \[(.*)\] START!/;

        # Check if test exists in known tests, if not, simply ignore it
        next unless lc $1 ~~ $_[1];

        $curTest = lc $1;

        my $testFile = $_[2] . '/' . $_[3]->{$curTest} . '/' . $1 . '.result';

        open($fh, '>', $testFile) or die "Could not create $testFile file!";
    }

    close($lg);
}

# RetrieveAllTests
#
# Given the path to the test folder or file will generate an array holding all the test names, an array holding the
# full paths to all the tests, the base directory to all the tests and a hash table containing a mapping between the
# test names and their categories.
#
# 1st Parameter: Path to the test folder/file
# Return: A reference to the array of test names
# Return: A reference to the array of full paths
# Return: Base directory of the tests folder
# Return: The hash table containing the mapping between the test and its category
sub RetrieveAllTests
{
    my $arrTests;
    my @fullPaths;
    my $baseDirectory;
    my $tests;
    my $filePath = $_[0];

    if (-d $filePath)
    {
        # If first argument is a directory => we have to collect all the tests to know what to run
        $baseDirectory = $filePath;

        ($tests, $arrTests) = CategorizeTests($baseDirectory, 1);

        foreach my $test (@$arrTests)
        {
            push @fullPaths, '"' . $baseDirectory . '\\' . $tests->{$test} . '\\' . $test . '"';
        }
    }
    else
    {
        die "First argument $filePath.test is not a file!\n" if not (-f $filePath . ".test");

        # Split into directory and filename
        ($baseDirectory, my $fileName) = $filePath =~ /(.*)\\(.*)/;

        my @arr;

        push @arr, lc $fileName; #(split(/\./, $fileName))[0];

        unlink($baseDirectory . '/' . $fileName . '.outcome');
        unlink($baseDirectory . '/' . $fileName . '.result');

        $arrTests = \@arr;

        push @fullPaths, $filePath;
    }

    return ($arrTests, \@fullPaths, $baseDirectory, $tests)
}

# a.k.a main
die "Script usage is $0 \$PATH_TO_TESTS_FOLDER|\$PATH_TO_TEST_FILE \$PATH_TO_LOG \$PATH_TO_VIX \$PATH_TO_VMX \$PATH_TO_PLACE_MODULE \$CMD_PREFIX /\$CMD_SUFFIX [\$SECONDS_TO_WAIT_FOR_TERMINATION]" if @ARGV < 7;

# Retrieve all tests we need to run
(my $arrTests, my $fullPaths, my $baseDirectory, my $tests) = RetrieveAllTests($ARGV[0]);

# Generate the corresponding .module files for the PXE boot and copy them to the PXE share
my $pxePath = '"' . $ARGV[4] . '"';
my $testPrefix = $ARGV[5];
my $testSuffix = $ARGV[6];

system("perl generate_modules.pl $pxePath $testPrefix $testSuffix @$arrTests");
die "Unable to generate .module files $pxePath" if ($? != 0);

my $vixPath = "\"" . $ARGV[2] . "\"";
my $vmxPath = "\"" . $ARGV[3] . "\"";
my $secsToTermination = $ARGV[7];

# Start the VM and wait for its completion
system("perl vm_operations.pl $vixPath $vmxPath START");
die "Unable to start VM at $vmxPath with $vixPath tool" if ($? != 0);

system("perl vm_operations.pl $vixPath $vmxPath WAIT $secsToTermination");

my $success = ($? == 0);

if (not $success)
{
    # will kill vm
    system("perl vm_operations.pl $vixPath $vmxPath TERMINATE");
    die "VM failed to complete in allocated time!\n";
}

# Parse the logs and output the .result files
ParseLog($ARGV[1], $arrTests, $baseDirectory, $tests);

# Compare the .result files with the .test files to generate the .outcome files
system("perl check_tests.pl @$fullPaths");
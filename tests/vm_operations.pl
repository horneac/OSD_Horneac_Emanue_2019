########################################################################################################################
#
#  Usage: .\vm_operations.pl $PATH_TO_VMRUN $PATH_TO_VMX [START|TERMINATE|WAIT] [$SECONDS_TO_WAIT_FOR_TERMINATION]
#
#  If you want to start a VM:
#  .\execute_tests.pl C:\VMWARE\VIX E:\WORKSPACE\VMS\HAL9000.vmx START
#
#  If you want to stop a VM:
#  .\execute_tests.pl C:\VMWARE\VIX E:\WORKSPACE\VMS\HAL9000.vmx TERMINATE
#
#  If you want to wait 30 seconds for a VM to terminate:
#  .\execute_tests.pl C:\VMWARE\VIX E:\WORKSPACE\VMS\HAL9000.vmx WAIT 30
########################################################################################################################

use strict;
use warnings;

#suppress smartmatch warnings
no if $] >= 5.017011, warnings => 'experimental::smartmatch';

# VmWaitForTermination
#
# Waits for the termination of the VM (2nd parameter) with a timeout (3rd parameter) using the
# VMRUN tool (1st parameter)
#
# 1st Parameter: Full path to the VMRUN tool
# 2nd Parameter: Full path to the .VMX file
# 3rd Parameter: Seconds to wait for VM termination;
# Return: VOID
sub VmWaitForTermination
{
    my @lines;
    my $startTime = time();
    my $running = 0;
    my $vmFullPath = quotemeta $_[1];
    my $toolFullPath = $_[0];
    my $dummyFile = "file.tmp";

    do
    {
        system("\"$toolFullPath\" -T ws list > $dummyFile");

        open(my $fh, '<', $dummyFile) or die "Could not open $dummyFile!\n";

        @lines = <$fh>;

        close($fh);

        # check every 5 seconds, no benefit of busy waiting
        sleep 5;
    } while (($running = /$vmFullPath/i ~~ @lines) and (time() - $startTime < $_[2]));

    unlink($dummyFile);

    return $running;
}

# VmTerminate
#
# Stops the VM (2nd parameter) using the VMRUN tool (1st parameter)
#
# 1st Parameter: Full path to the VMRUN tool
# 2nd Parameter: Full path to the .VMX file
# Return: VOID
sub VmTerminate
{
    my $toolFullPath = $_[0];
    my $vmFullPath = $_[1];

    system("\"$toolFullPath\" -T ws stop \"$vmFullPath\" hard");
}

# VmStart
#
# Starts the VM (2nd parameter) using the VMRUN tool (1st parameter)
#
# 1st Parameter: Full path to the VMRUN tool
# 2nd Parameter: Full path to the .VMX file
# Return: VOID
sub VmStart
{
    my $toolFullPath = $_[0];
    my $vmFullPath = $_[1];

    system("\"$toolFullPath\" -T ws start \"$vmFullPath\"");
}

# a.k.a. main
die "Usage $0 \$PATH_TO_VIX_TOOLS \$PATH_TO_VMX\n START|WAIT|TERMINATE [\$SECONDS]" if @ARGV < 3;

my $vmRunFullPath = $ARGV[0] . '\\' . 'vmrun.exe';

# escape special characters, else we cannot use in regex if path contains \ or . or any other special characters
my $fullPathToVmx = $ARGV[1];
my $secsToWait = @ARGV > 3 ? $ARGV[3] : 180;

{
    'WAIT'          => sub {exit VmWaitForTermination($vmRunFullPath, $fullPathToVmx, $secsToWait)},
    'TERMINATE'     => sub {exit VmTerminate($vmRunFullPath, $fullPathToVmx)},
    "START"         => sub {exit VmStart($vmRunFullPath, $fullPathToVmx)}
}->{$ARGV[2]}() // die "Unsupported command" . $ARGV[2];

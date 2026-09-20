#!/usr/bin/env PowerShell

# The usage message is placed here to serve as short script documentation, too.
$usage = `
'usage: .\compareLinNetLogs.ps1 <a.log> <b.log>
  Two linNet log files are compared without comparing the time stamps in
the files. All the rest needs to be identical to make the script succeed
and return 0. If differences are found then a short related status message
is written and 1 is returned. The calling script can evaluate the return
code with $LASTEXITCODE.
  Parameter a.log: A linNet log file
            b.log: A linNet log file, which is expected to be (nearly)
                   identical to a.log'
#
# Copyright (c) 2026, Peter Vranken
#
# This program is free software: you can redistribute it and/or modify it
# under the terms of the GNU General Public License as published by the
# Free Software Foundation, either version 3 of the License, or (at your
# option) any later version.
#
# This program is distributed in the hope that it will be useful, but
# WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
# General Public License for more details.
#
# You should have received a copy of the GNU General Public License along
# with this program. If not, see <http://www.gnu.org/licenses/>.

# Please note: On Windows machines, the use of PowerShell is initially hindered by two
# stupid settings, but normal user rights permit to fully enable its use. 
# 
# First, the use of PowerShell is by default restricted to interactive use but script
# execution is forbidden. To overcome this, open a PowerShell window interactively and
# type:
#
#   Set-ExecutionPolicy unrestricted -Scope CurrentUser
# 
# The effect of the command is permanent. You can close your PowerShell window and start
# using our PowerShell scripts.
#
# Second, the default operation on mouse-click is opening the script in a text editor
# instead of running the script in PowerShell. You should change the file name assciation,
# e.g., by typing "Default apps" in the start menu and looking for the settings of file
# type ".ps1". Associate either pwsh.exe or PowerShell.exe with this file type.
#
# See
# http://technet.microsoft.com/de-de/library/ee221100(en-us).aspx for syntax of Powershell
# scripts.

function endWithError
{
    Write-Host $usage;
    Write-Host "Press any key to continue ..."
    $dummy = $host.UI.RawUI.ReadKey("NoEcho,IncludeKeyDown")

    exit;
}

# Support help
if($args[0] -match '^$|^(-h|/h|/\?|--help)$') {endWithError}

# Limit the allowed number of parameters.
if ($args[1] -notmatch '.') {endWithError}
if ($args[2] -match '.') {endWithError}

# Regexp to match the time stamps from a linNet log, if the format is the default format.
$rx = '^[A-Z][a-z]{2}\s+\d{1,2}\s+\d\d:\d\d:\d\d\s+\d{4}\s+-\s+\d+\s+ms\s+-\s*'

# Read the files and replace all matches of the regexp with nothing.
$a = (Get-Content $args[0]) -replace $rx,''
$b = (Get-Content $args[1]) -replace $rx,''

# Get an array of differences. Each array entry has the properties
# InputObject and SideIndicator. Each modification occurs as two entries;
# InputObject is once from a and once from b. Insertions and deletions
# appear as a single entry (TBC).
$diff = Compare-Object $a $b

if ($null -eq $diff)
{
    #Write-Host ("compareLinNetLogs: Files " + $args[0] + " and " + $args[1] `
    #            + " are identical.")
    exit 0
}
else
{
    Write-Host ("compareLinNetLogs: " + [Math]::Ceiling($diff.Length / 2) + " differences" `
                + " found between " + $args[0] + " and " + $args[1])
    exit 1
}

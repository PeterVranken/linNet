# Use own script name as image name: This allows copying the script without modification
# next to each drawing. Rename the script same to the LaTeX source file and rebuilt it
# after modification by double-click on the PowerShell script aside of the LaTeX source.


function Capitalize-First {
    param([string]$text)

    if ([string]::IsNullOrEmpty($text)) {
        return $text
    }

    return $text.Substring(0,1).ToUpper() + $text.Substring(1)
}

$scriptFilePath = $MyInvocation.MyCommand.Path
$scriptPath = [System.IO.Path]::GetDirectoryName($scriptFilePath)
$baseName = [System.IO.Path]::GetFileNameWithoutExtension($scriptFilePath)
Write-Host "Build LaTeX graphics $scriptPath\$baseName.tex"

del "$baseName.log" 2> $null
pdflatex "-jobname=$baseName" -file-line-error -halt-on-error -quiet "$baseName.tex"

# The image is on a full A4 page. Extract the image area into a new PDF.
if ($LASTEXITCODE -eq 0) {
    pdfcrop "$baseName.pdf" "$baseName.pdf"
    if ($LASTEXITCODE -ne 0) {
        Write-Error "Error running LaTeX tool pdfcrop. Please see previous error messages."
    }
} else {
    Get-Content "$baseName.log"
#    Write-Error "Error running LaTeX. Please check your LaTeX log file $baseName.log."
}



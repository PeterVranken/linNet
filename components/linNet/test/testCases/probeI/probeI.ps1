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

# -jobname: To avoid a name clash with linNet's log-file of same name (which has precedence
# as it is part of the uploaded data).
latex -jobname=latex -quiet "$baseName.tex"

# DVI to SVG export.
if ($LASTEXITCODE -eq 0) {
    dvisvgm "latex.dvi" --no-fonts -o (Capitalize-First("$baseName.svg"))
    if ($LASTEXITCODE -ne 0) {
        Write-Error "Error running LaTeX tool dvisvgm. Please see previous error messages."
    }
} else {
    Write-Error "Error running LaTeX. Please check your LaTeX source."
}



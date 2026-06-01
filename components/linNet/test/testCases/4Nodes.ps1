# Use own script name as image name: This allows copying the script without modification
# next to each drawing. Rename the script same to the LaTeX source file and rebuilt it
# after modification by double-click on the PowerShell script aside of the LaTeX source.
$scriptFilePath = $MyInvocation.MyCommand.Path
$scriptPath = [System.IO.Path]::GetDirectoryName($scriptFilePath)
$baseName = [System.IO.Path]::GetFileNameWithoutExtension($scriptFilePath)
Write-Host "Build LaTeX graphics $scriptPath\$baseName.tex"

latex "$baseName.tex"

# DVI to SVG export.
if ($LASTEXITCODE -eq 0) {
    dvisvgm "$baseName.dvi" --no-fonts -o "$baseName.svg"
    if ($LASTEXITCODE -ne 0) {
        Write-Error "Error running LaTeX tool dvisvgm. Please see previous error messages."
    }
} else {
    Write-Error "Error running LaTeX. Please check your LaTeX source."
}

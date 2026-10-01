param(
    [string]$SourceText = "voyna_i_mir.txt"
)

$ErrorActionPreference = "Stop"
$ProjectRoot = $PSScriptRoot
$DataRoot = Join-Path $ProjectRoot "experiments\data"

function Take-Bytes {
    param([string]$Path, [int]$Bytes, [string]$OutPath)
    $all = [System.IO.File]::ReadAllBytes($Path)
    $count = [Math]::Min($Bytes, $all.Length)
    $slice = New-Object byte[] $count
    [System.Array]::Copy($all, $slice, $count)
    [System.IO.File]::WriteAllBytes($OutPath, $slice)
}

function New-RandomFile {
    param([int]$Bytes, [string]$OutPath)
    $rng = [System.Security.Cryptography.RandomNumberGenerator]::Create()
    $buffer = New-Object byte[] $Bytes
    $rng.GetBytes($buffer)
    [System.IO.File]::WriteAllBytes($OutPath, $buffer)
}

New-Item -ItemType Directory -Force -Path `
    (Join-Path $DataRoot "text"), `
    (Join-Path $DataRoot "source"), `
    (Join-Path $DataRoot "random"), `
    (Join-Path $DataRoot "precompressed") | Out-Null

$SourcePath = $SourceText
if (-not [System.IO.Path]::IsPathRooted($SourcePath)) {
    $SourcePath = Join-Path (Get-Location) $SourcePath
}

if (-not (Test-Path -LiteralPath $SourcePath -PathType Leaf)) {
    $SourcePath = Join-Path $ProjectRoot $SourceText
}
if (-not (Test-Path -LiteralPath $SourcePath -PathType Leaf)) {
    $TextDirectory = Join-Path $DataRoot "text"
    $FallbackText = Get-ChildItem -LiteralPath $TextDirectory -Filter "*.txt" -File |
        Where-Object { $_.Name -notmatch '^text_(1kb|100kb|1mb|full)\.txt$' } |
        Select-Object -First 1
    if ($FallbackText) {
        $SourcePath = $FallbackText.FullName
    }
    else {
        throw "Text file not found at '$SourceText'. Place voyna_i_mir.txt in the project root or pass -SourceText <path>."
    }
}

$TextDirectory = Join-Path $DataRoot "text"
Copy-Item -LiteralPath $SourcePath -Destination (Join-Path $TextDirectory "text_full.txt") -Force
Take-Bytes -Path $SourcePath -Bytes 1KB   -OutPath (Join-Path $TextDirectory "text_1kb.txt")
Take-Bytes -Path $SourcePath -Bytes 100KB -OutPath (Join-Path $TextDirectory "text_100kb.txt")
Take-Bytes -Path $SourcePath -Bytes 1MB   -OutPath (Join-Path $TextDirectory "text_1mb.txt")

$SourceFiles = Get-ChildItem -LiteralPath (Join-Path $ProjectRoot "src") -File |
    Where-Object { $_.Extension -in ".h", ".cpp" } |
    Sort-Object Name
$SourceStream = [System.IO.MemoryStream]::new()
try {
    foreach ($File in $SourceFiles) {
        $FileBytes = [System.IO.File]::ReadAllBytes($File.FullName)
        $SourceStream.Write($FileBytes, 0, $FileBytes.Length)
        $Separator = [System.Text.Encoding]::UTF8.GetBytes("`r`n")
        $SourceStream.Write($Separator, 0, $Separator.Length)
    }
    [System.IO.File]::WriteAllBytes((Join-Path $DataRoot "source\source_code.txt"), $SourceStream.ToArray())
}
finally {
    $SourceStream.Dispose()
}

New-RandomFile -Bytes 1KB   -OutPath (Join-Path $DataRoot "random\random_1kb.bin")
New-RandomFile -Bytes 100KB -OutPath (Join-Path $DataRoot "random\random_100kb.bin")
New-RandomFile -Bytes 1MB   -OutPath (Join-Path $DataRoot "random\random_1mb.bin")

Compress-Archive -LiteralPath (Join-Path $TextDirectory "text_full.txt") `
    -DestinationPath (Join-Path $DataRoot "precompressed\text_full.zip") -Force

Write-Host "Done. Files created:"
Get-ChildItem -Recurse -File $DataRoot | Select-Object FullName, Length
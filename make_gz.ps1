# Определяем исходную и целевую директории
$sourceDir = "HTML_data/"
$destinationDir = "m_IoTManager/data/"
$useList = $true

# Создаём целевую директорию, если её нет
if (!(Test-Path -Path $destinationDir)) {
    New-Item -ItemType Directory -Path $destinationDir | Out-Null
}

# Расширения файлов для компрессии
$fileExtensions = @(".htm", ".html", ".js", ".css")

$fileList = @(
    "condition.htm",
    "edit.htm",
    "graphs.htm",
    "help.htm",
    "home.htm",
    "index.htm",
    "IR_setup.htm",
    "other_setup.htm",
    "pin_setup.htm",
    "wifi_setup.htm",
    "ws2811.html",
    "ace.min.js",
    "chart.min.js",
    "condition.js",
    "graphs.js",
    "help.js",
    "helper_func.js",
    "home_ir.js",
    "IR_recieve.js",
    "other_setup.js",
    "pin_setup.js",
    "script_home.js",
    "style_generated.css",
    "wifi_setup.js",
    "ws2811.js",
    "ws2812_set.js"
)

# Variable to determine whether to use the list or not

Get-ChildItem -Path $sourceDir -Recurse -File | Where-Object {
    # $fileExtensions -contains $_.Extension.ToLower()
    $fileExtensions -contains $_.Extension.ToLower() -and (!$useList -or $fileList -contains $_.Name)
} | ForEach-Object {
    # Вычисляем относительный путь от исходной директории
    $relativePath = $_.FullName.Substring((Get-Location).Path.Length + $sourceDir.Length + 1)
    $gzippedFilePath = Join-Path $destinationDir ($relativePath + ".gz")

    Write-Output $relativePath
    # Гарантируем создание необходимых директории
    $directoryPath = [System.IO.Path]::GetDirectoryName($gzippedFilePath)
    if (-not (Test-Path $directoryPath)) {
        New-Item -ItemType Directory -Path $directoryPath -Force | Out-Null
    }

    $originalFileName = $_.FullName
    $gzippedFileName = $gzippedFilePath

    # Компрессия файла
    $inputFile = [System.IO.File]::OpenRead($originalFileName)
    $outputFile = [System.IO.File]::Create($gzippedFileName)
    $gzipStream = New-Object System.IO.Compression.GzipStream($outputFile, [System.IO.Compression.CompressionMode]::Compress)

    $buffer = New-Object byte[] 4096
    while (($bytesRead = $inputFile.Read($buffer, 0, $buffer.Length)) -gt 0) {
        $gzipStream.Write($buffer, 0, $bytesRead)
    }

    $gzipStream.Close()
    $inputFile.Close()
    $outputFile.Close()

    # Write-Output "Compressed: $originalFileName to $gzippedFileName"
}
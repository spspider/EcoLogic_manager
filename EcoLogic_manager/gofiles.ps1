# Delete the output.txt file if it exists
if (Test-Path "output.txt") {
    Remove-Item "output.txt"
}

Get-ChildItem -Recurse -Filter *.ino | ForEach-Object {
    if ($_ -is [io.fileinfo]) {
        $filePath = $_.FullName
        $fileContent = Get-Content $filePath -Raw
        $output = "file path:`n<$filePath>`n""" + 'content of file: """' + $fileContent + '"""'
        Add-Content -Path "output.txt" -Value $output
    }
}

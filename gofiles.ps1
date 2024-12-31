Get-ChildItem -Recurse -Filter *.ino | ForEach-Object {
    if ($_ -is [io.fileinfo]) {
        $filePath = $_.FullName
        $fileContent = Get-Content $filePath
        Write-Output "`n<$filePath>"
        Write-Output '"""'
        Write-Output $fileContent
        Write-Output '"""'
    }
}

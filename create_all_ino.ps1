# Set the directory to search for .ino files
$directory = ".\m_IoTManager\"

# Set the output file path
$outputFile = ".\combined_files.ino"

# Get all .ino files in the directory and its subdirectories
$inoFiles = Get-ChildItem -Path $directory -Filter *.ino -Recurse

# Initialize a variable to hold the combined content
$combinedContent = @()

# Loop through each .ino file and concatenate its content
foreach ($file in $inoFiles) {
    $content = Get-Content $file.FullName -Raw
    $combinedContent += $content
    $combinedContent += "`r`n"  # Ensure there's a newline separating content from different files
}

# Write each line of the combined content to the new .ino file
$combinedContent | Out-File $outputFile -Encoding ASCII

# Output the path of the created file to the console
Write-Host "Combined file created at: $outputFile"
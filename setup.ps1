# Don't allow our script to continue if any errors are observed
$ErrorActionPreference = "Stop"

$cwd = Get-Location
$external_dir = "$cwd\External"

Function download_archive
{
    Param
    (
        [String]
        $outputDir,
        [String]
        $outputDirName,
        [String]
        $outputArchiveName,
        [String]
        $repositoryURL
    )

    Write-Host "Testing path: $outputDir\$outputDirName"

    # Check that we have the library
    if (!(Test-Path "$outputDir\$outputDirName"))
    {
        try
        {
            Write-Host "Downloading library into folder $outputDirName"
            $WebClient = New-Object System.Net.WebClient
            $WebClient.DownloadFile("$repositoryURL", "$outputDir\archive.zip")
        }
        catch
        {
            Write-Host "Something went wrong when downloading $repositoryURL library" -ForegroundColor Red
            Write-Host $_
            Read-Host -Prompt "Press Enter to exit"
            exit
        }

        try
        {
            Write-Host "Unzipping library into $outputDir\$outputDirName ..."
            
            Expand-Archive "$outputDir\archive.zip" "$outputDir\."
            Rename-Item -Path "$outputDir\$outputArchiveName" -NewName "$outputDirName"
            Remove-Item -Path "$outputDir\archive.zip"
        }
        catch
        {
            Write-Host "Something went wrong when unzipping $outputArchiveName library" -ForegroundColor Red
            Write-Host $_
            Read-Host -Prompt "Press Enter to exit"
            exit
        }
    }
}

Write-Host "Executing windows\setup.ps1" -ForegroundColor Green
Write-Host "Current working directory: $cwd"

Write-Host "Running script for Windows desktop"

if (!(Test-Path "$external_dir"))
{	
    # Check that we have a 'external' folder
	Write-Host "External directory was not found, creating ..."
	New-Item -ItemType Directory -Path "$external_dir"
}

Write-Host "External directory: $external_dir"

download_archive $external_dir "CoralEngine" "CoralEngine-1.0.0" "https://github.com/GuusKemperman/CoralEngine/archive/refs/tags/v1.0.0.zip"

Write-Host "Setup script completed" -ForegroundColor Green
Read-Host -Prompt "Press Enter to exit"
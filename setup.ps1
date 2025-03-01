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

# download_archive $external_dir "sdl-windows" "SDL2-2.0.20" "https://www.libsdl.org/release/SDL2-devel-2.0.20-VC.zip"
# download_archive $external_dir "spdlog-windows" "spdlog-1.x" "https://github.com/gabime/spdlog/archive/refs/heads/v1.x.zip"
# download_archive $external_dir "gsl-windows" "GSL-main" "https://github.com/microsoft/GSL/archive/refs/heads/main.zip"
# download_archive $external_dir "crc-windows" "CRCpp-master" "https://github.com/d-bahr/CRCpp/archive/refs/heads/master.zip"
# download_archive $external_dir "glm-windows" "glm-master" "https://github.com/g-truc/glm/archive/refs/heads/master.zip"
# download_archive $external_dir "entt-windows" "entt-master" "https://github.com/skypjack/entt/archive/refs/heads/master.zip"
# download_archive $external_dir "stb-windows" "stb-master" "https://github.com/nothings/stb/archive/refs/heads/master.zip"
download_archive $external_dir "CoralEngine" "CoralEngine-main-lite" "https://github.com/GuusKemperman/CoralEngine/archive/refs/heads/main-lite.zip"

Write-Host "Setup script completed" -ForegroundColor Green
Read-Host -Prompt "Press Enter to exit"
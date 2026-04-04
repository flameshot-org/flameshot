[CmdletBinding()]
param(
    [string]$RepoRoot = (Resolve-Path (Join-Path $PSScriptRoot "..")).Path,
    [string]$ToolsRoot = (Join-Path (Resolve-Path (Join-Path $PSScriptRoot "..")).Path ".tools"),
    [string]$QtRoot = "C:\Qt",
    [string]$QtVersion = "6.9.3",
    [string]$QtArch = "win64_msvc2022_64",
    [string]$VcpkgCommit = "d5ec528843d29e3a52d745a64b469f810b2cedbf",
    [switch]$InstallWix
)

Set-StrictMode -Version Latest
$ErrorActionPreference = "Stop"

function Write-Step {
    param([string]$Message)

    Write-Host ""
    Write-Host "==> $Message" -ForegroundColor Cyan
}

function Write-Info {
    param([string]$Message)

    Write-Host "    $Message" -ForegroundColor DarkGray
}

function Test-Administrator {
    $identity = [Security.Principal.WindowsIdentity]::GetCurrent()
    $principal = New-Object Security.Principal.WindowsPrincipal($identity)
    return $principal.IsInRole([Security.Principal.WindowsBuiltInRole]::Administrator)
}

function Refresh-Path {
    $machine = [Environment]::GetEnvironmentVariable("Path", "Machine")
    $user = [Environment]::GetEnvironmentVariable("Path", "User")
    $env:Path = @($machine, $user) -join ";"
}

function Get-CommandPath {
    param([string[]]$Names)

    foreach ($name in $Names) {
        $command = Get-Command $name -ErrorAction SilentlyContinue | Select-Object -First 1
        if ($null -ne $command) {
            return $command.Source
        }
    }

    return $null
}

function Add-PathEntryForCurrentUser {
    param([string]$PathEntry)

    $current = [Environment]::GetEnvironmentVariable("Path", "User")
    $parts = @()
    if (-not [string]::IsNullOrWhiteSpace($current)) {
        $parts = $current.Split(";") | Where-Object { -not [string]::IsNullOrWhiteSpace($_) }
    }

    if ($parts -contains $PathEntry) {
        return
    }

    $updated = @($parts + $PathEntry) -join ";"
    [Environment]::SetEnvironmentVariable("Path", $updated, "User")
}

function Set-UserEnvVar {
    param(
        [string]$Name,
        [string]$Value
    )

    $current = [Environment]::GetEnvironmentVariable($Name, "User")
    if ($current -ne $Value) {
        [Environment]::SetEnvironmentVariable($Name, $Value, "User")
    }
}

function Test-WingetPackageInstalled {
    param([string]$PackageId)

    $output = & winget list --exact --id $PackageId --accept-source-agreements 2>$null | Out-String
    return $LASTEXITCODE -eq 0 -and $output -match [regex]::Escape($PackageId)
}

function Install-WingetPackage {
    param(
        [string]$PackageId,
        [string]$DisplayName,
        [string]$Override
    )

    if (Test-WingetPackageInstalled -PackageId $PackageId) {
        Write-Info "$DisplayName ja esta instalado."
        return
    }

    Write-Info "Instalando $DisplayName via winget..."

    $args = @(
        "install",
        "--exact",
        "--id", $PackageId,
        "--accept-package-agreements",
        "--accept-source-agreements",
        "--disable-interactivity",
        "--silent"
    )

    if (-not [string]::IsNullOrWhiteSpace($Override)) {
        $args += @("--override", $Override)
    }

    & winget @args
    if ($LASTEXITCODE -ne 0) {
        throw "Falha ao instalar o pacote '$PackageId' via winget."
    }

    Refresh-Path
}

function Get-VsWherePath {
    $candidates = @(
        (Join-Path ${env:ProgramFiles(x86)} "Microsoft Visual Studio\Installer\vswhere.exe"),
        (Join-Path ${env:ProgramFiles} "Microsoft Visual Studio\Installer\vswhere.exe")
    ) | Where-Object { Test-Path $_ }

    return $candidates | Select-Object -First 1
}

function Get-VisualStudioBuildToolsInstallationPath {
    $vswhere = Get-VsWherePath
    if (-not $vswhere) {
        return $null
    }

    $installPath = & $vswhere `
        -latest `
        -products Microsoft.VisualStudio.Product.BuildTools `
        -property installationPath

    $installPath = ($installPath | Out-String).Trim()
    if ([string]::IsNullOrWhiteSpace($installPath)) {
        return $null
    }

    return $installPath
}

function Get-VisualStudioInstallerSetupPath {
    $fallback = Join-Path ${env:ProgramFiles(x86)} "Microsoft Visual Studio\Installer\setup.exe"
    if (Test-Path $fallback) {
        return $fallback
    }

    $secondary = Join-Path ${env:ProgramFiles(x86)} "Microsoft Visual Studio\Installer\vs_installer.exe"
    if (Test-Path $secondary) {
        return $secondary
    }

    return $null
}

function Test-VisualStudioBuildToolsReady {
    $installPath = Get-VisualStudioBuildToolsInstallationPath
    if (-not $installPath) {
        return $false
    }

    $clCandidates = Get-ChildItem `
        -Path (Join-Path $installPath "VC\Tools\MSVC") `
        -Filter cl.exe `
        -Recurse `
        -ErrorAction SilentlyContinue

    return ($clCandidates | Measure-Object).Count -gt 0
}

function Repair-VisualStudioBuildTools {
    $installPath = Get-VisualStudioBuildToolsInstallationPath
    if (-not $installPath) {
        return
    }

    $setupExe = Get-VisualStudioInstallerSetupPath
    if (-not $setupExe) {
        throw "Nao foi possivel localizar o instalador do Visual Studio para reparar a carga de trabalho C++."
    }

    Write-Info "Build Tools encontrado sem compilador C++; adicionando workload nativa de desktop..."
    & $setupExe modify `
        --installPath $installPath `
        --add Microsoft.VisualStudio.Workload.VCTools `
        --includeRecommended `
        --passive `
        --norestart

    if ($LASTEXITCODE -ne 0) {
        throw "Falha ao reparar o Visual Studio Build Tools com a workload de C++."
    }
}

function Ensure-VisualStudioBuildTools {
    if (Test-VisualStudioBuildToolsReady) {
        Write-Info "Visual Studio Build Tools com C++ ja encontrado."
        return
    }

    if (Get-VisualStudioBuildToolsInstallationPath) {
        Repair-VisualStudioBuildTools
    }
    else {
        Install-WingetPackage `
            -PackageId "Microsoft.VisualStudio.2022.BuildTools" `
            -DisplayName "Visual Studio Build Tools 2022" `
            -Override "--wait --passive --norestart --add Microsoft.VisualStudio.Workload.VCTools --includeRecommended"
    }

    if (-not (Test-VisualStudioBuildToolsReady)) {
        throw "Visual Studio Build Tools nao foi detectado apos a instalacao."
    }
}

function Get-Python313Executable {
    $py = Get-CommandPath -Names @("py")
    if ($py) {
        try {
            $pythonExe = & $py -3.13 -c "import sys; print(sys.executable)"
            if ($LASTEXITCODE -eq 0 -and -not [string]::IsNullOrWhiteSpace(($pythonExe | Out-String).Trim())) {
                return ($pythonExe | Out-String).Trim()
            }
        }
        catch {
        }
    }

    $python = Get-CommandPath -Names @("python")
    if ($python) {
        try {
            $version = & $python -c "import sys; print(f'{sys.version_info.major}.{sys.version_info.minor}')"
            if ($LASTEXITCODE -eq 0 -and (($version | Out-String).Trim() -eq "3.13")) {
                return $python
            }
        }
        catch {
        }
    }

    return $null
}

function Ensure-Python313 {
    $pythonExe = Get-Python313Executable
    if ($pythonExe) {
        Write-Info "Python 3.13 ja encontrado em '$pythonExe'."
        return $pythonExe
    }

    Install-WingetPackage `
        -PackageId "Python.Python.3.13" `
        -DisplayName "Python 3.13" `
        -Override "InstallAllUsers=1 PrependPath=1 Include_launcher=1"

    $pythonExe = Get-Python313Executable
    if (-not $pythonExe) {
        throw "Python 3.13 nao foi encontrado apos a instalacao."
    }

    return $pythonExe
}

function Ensure-Aqt {
    param(
        [string]$PythonExe,
        [string]$AqtVenvPath
    )

    $venvPython = Join-Path $AqtVenvPath "Scripts\python.exe"
    if (-not (Test-Path $venvPython)) {
        Write-Info "Criando ambiente virtual para o aqtinstall em '$AqtVenvPath'..."
        & $PythonExe -m venv $AqtVenvPath | Out-Host
        if ($LASTEXITCODE -ne 0) {
            throw "Falha ao criar o ambiente virtual do aqtinstall."
        }
    }

    Write-Info "Atualizando pip e instalando aqtinstall..."
    & $venvPython -m pip install --upgrade pip aqtinstall | Out-Host
    if ($LASTEXITCODE -ne 0) {
        throw "Falha ao instalar o aqtinstall."
    }

    return $venvPython
}

function Ensure-Qt {
    param(
        [string]$AqtPython,
        [string]$QtInstallRoot,
        [string]$Version,
        [string]$Architecture
    )

    $qtVersionDir = Join-Path $QtInstallRoot $Version

    function Resolve-QtArchDir {
        param(
            [string]$VersionDir,
            [string]$PreferredArchitecture
        )

        $preferredDir = Join-Path $VersionDir $PreferredArchitecture
        if (Test-Path (Join-Path $preferredDir "lib\cmake\Qt6")) {
            return $preferredDir
        }

        $match = Get-ChildItem -Path $VersionDir -Directory -ErrorAction SilentlyContinue |
            Where-Object { Test-Path (Join-Path $_.FullName "lib\cmake\Qt6") } |
            Select-Object -First 1

        if ($match) {
            return $match.FullName
        }

        return $null
    }

    $qtArchDir = Resolve-QtArchDir -VersionDir $qtVersionDir -PreferredArchitecture $Architecture
    if ($qtArchDir) {
        Write-Info "Qt $Version ($Architecture) ja encontrado em '$qtArchDir'."
        return $qtArchDir
    }

    New-Item -ItemType Directory -Force -Path $QtInstallRoot | Out-Null

    Write-Info "Consultando arquiteturas disponiveis do Qt $Version..."
    $architectures = & $AqtPython -m aqt list-qt windows desktop --arch $Version
    if ($LASTEXITCODE -ne 0) {
        throw "Falha ao consultar as arquiteturas do Qt via aqtinstall."
    }

    $architectureList = ($architectures | Out-String)
    if ($architectureList -notmatch [regex]::Escape($Architecture)) {
        throw "A arquitetura '$Architecture' nao esta disponivel para o Qt $Version."
    }

    Write-Info "Instalando Qt $Version ($Architecture) com o modulo qtimageformats..."
    & $AqtPython -m aqt install-qt `
        --outputdir $QtInstallRoot `
        windows desktop $Version $Architecture `
        -m qtimageformats | Out-Host

    if ($LASTEXITCODE -ne 0) {
        throw "Falha ao instalar o Qt $Version."
    }

    $qtArchDir = Resolve-QtArchDir -VersionDir $qtVersionDir -PreferredArchitecture $Architecture
    if (-not $qtArchDir) {
        throw "Qt foi instalado, mas nenhuma pasta valida com 'lib\\cmake\\Qt6' foi encontrada em '$qtVersionDir'."
    }

    return $qtArchDir
}

function Ensure-Git {
    $gitExe = Get-CommandPath -Names @("git")
    if ($gitExe) {
        Write-Info "Git ja encontrado em '$gitExe'."
        return $gitExe
    }

    Install-WingetPackage -PackageId "Git.Git" -DisplayName "Git"

    $gitExe = Get-CommandPath -Names @("git")
    if (-not $gitExe) {
        throw "Git nao foi encontrado apos a instalacao."
    }

    return $gitExe
}

function Ensure-CMake {
    $cmakeExe = Get-CommandPath -Names @("cmake")
    if ($cmakeExe) {
        Write-Info "CMake ja encontrado em '$cmakeExe'."
        return $cmakeExe
    }

    Install-WingetPackage -PackageId "Kitware.CMake" -DisplayName "CMake"

    $cmakeExe = Get-CommandPath -Names @("cmake")
    if (-not $cmakeExe) {
        throw "CMake nao foi encontrado apos a instalacao."
    }

    return $cmakeExe
}

function Ensure-Wix {
    if (-not $InstallWix) {
        return
    }

    Install-WingetPackage -PackageId "WiXToolset.WiXToolset" -DisplayName "WiX Toolset"
}

function Ensure-Vcpkg {
    param(
        [string]$GitExe,
        [string]$RootPath,
        [string]$Commit
    )

    $parent = Split-Path -Parent $RootPath
    New-Item -ItemType Directory -Force -Path $parent | Out-Null

    if (-not (Test-Path (Join-Path $RootPath ".git"))) {
        Write-Info "Clonando vcpkg em '$RootPath'..."
        & $GitExe clone https://github.com/microsoft/vcpkg $RootPath | Out-Host
        if ($LASTEXITCODE -ne 0) {
            throw "Falha ao clonar o vcpkg."
        }
    }
    else {
        Write-Info "Repositorio vcpkg ja existe em '$RootPath'."
    }

    Write-Info "Fixando vcpkg no commit $Commit..."
    & $GitExe -C $RootPath fetch --tags --force origin | Out-Host
    if ($LASTEXITCODE -ne 0) {
        throw "Falha ao atualizar o repositorio vcpkg."
    }

    & $GitExe -C $RootPath checkout $Commit | Out-Host
    if ($LASTEXITCODE -ne 0) {
        throw "Falha ao mudar o vcpkg para o commit esperado."
    }

    $bootstrap = Join-Path $RootPath "bootstrap-vcpkg.bat"
    $vcpkgExe = Join-Path $RootPath "vcpkg.exe"

    if (-not (Test-Path $vcpkgExe)) {
        Write-Info "Executando bootstrap do vcpkg..."
        & $bootstrap -disableMetrics | Out-Host
        if ($LASTEXITCODE -ne 0) {
            throw "Falha no bootstrap do vcpkg."
        }
    }

    $opensslPort = "openssl-windows"
    if (-not (Test-Path (Join-Path $RootPath "ports\$opensslPort"))) {
        $opensslPort = "openssl"
    }

    Write-Info "Instalando $opensslPort:x64-windows..."
    & $vcpkgExe install "$opensslPort`:x64-windows" | Out-Host
    if ($LASTEXITCODE -ne 0) {
        throw "Falha ao instalar OpenSSL via vcpkg."
    }

    return $RootPath
}

if (-not (Test-Administrator)) {
    throw "Execute este script em um PowerShell aberto como Administrador."
}

$RepoRoot = (Resolve-Path $RepoRoot).Path
$ToolsRoot = [IO.Path]::GetFullPath($ToolsRoot)
$AqtVenv = Join-Path $ToolsRoot "aqt-venv"
$VcpkgRoot = Join-Path $ToolsRoot "vcpkg"

Write-Step "Validando prerequisitos basicos"

if (-not (Get-CommandPath -Names @("winget"))) {
    throw "winget nao foi encontrado. Atualize o App Installer da Microsoft Store antes de continuar."
}

Write-Info "Repositorio: $RepoRoot"
Write-Info "Ferramentas auxiliares: $ToolsRoot"

Write-Step "Instalando toolchain do Windows"
$null = Ensure-CMake
$gitExe = Ensure-Git
Ensure-VisualStudioBuildTools
Ensure-Wix

Write-Step "Instalando Python e aqtinstall"
$pythonExe = Ensure-Python313
$aqtPython = Ensure-Aqt -PythonExe $pythonExe -AqtVenvPath $AqtVenv

Write-Step "Instalando Qt"
$qtArchDir = Ensure-Qt `
    -AqtPython $aqtPython `
    -QtInstallRoot $QtRoot `
    -Version $QtVersion `
    -Architecture $QtArch

Write-Step "Instalando vcpkg e OpenSSL"
$vcpkgRoot = Ensure-Vcpkg -GitExe $gitExe -RootPath $VcpkgRoot -Commit $VcpkgCommit

Write-Step "Persistindo variaveis de ambiente"
$qt6Dir = Join-Path $qtArchDir "lib\cmake\Qt6"
$qtBinDir = Join-Path $qtArchDir "bin"
$opensslRoot = Join-Path $vcpkgRoot "installed\x64-windows"
$opensslBinDir = Join-Path $opensslRoot "bin"

Set-UserEnvVar -Name "Qt6_DIR" -Value $qt6Dir
Set-UserEnvVar -Name "VCPKG_ROOT" -Value $vcpkgRoot
Set-UserEnvVar -Name "OPENSSL_ROOT_DIR" -Value $opensslRoot
Set-UserEnvVar -Name "FLAMESHOT_QT_ROOT" -Value $qtArchDir
Add-PathEntryForCurrentUser -PathEntry $qtBinDir
Add-PathEntryForCurrentUser -PathEntry $opensslBinDir

Refresh-Path

Write-Step "Ambiente pronto"
$configureCommand = @(
    'cmake -S . -B build',
    '-G "Visual Studio 17 2022"',
    '-A x64',
    "-DCMAKE_TOOLCHAIN_FILE=""$vcpkgRoot\scripts\buildsystems\vcpkg.cmake""",
    "-DQt6_DIR=""$qt6Dir""",
    '-DENABLE_OPENSSL=ON',
    '-DUSE_PORTABLE_CONFIG=ON'
) -join " "

$buildCommand = 'cmake --build build --config Release'

Write-Host ""
Write-Host "Execute estes comandos em um novo PowerShell dentro do repositorio:" -ForegroundColor Green
Write-Host ""
Write-Host $configureCommand -ForegroundColor Yellow
Write-Host $buildCommand -ForegroundColor Yellow
Write-Host ""
Write-Host "Executavel esperado: build\src\Release\flameshot.exe" -ForegroundColor Green
if ($InstallWix) {
    Write-Host "Para gerar MSI depois do build: cpack -G WIX -B build\Package --config build\CPackConfig.cmake" -ForegroundColor Green
}

##############################################################################
# KingSejong 언어 설치 스크립트 (Windows PowerShell)
#
# 사용법:
#   .\install.ps1                    # 기본 빌드
#   .\install.ps1 -SystemInstall     # 시스템 설치
#   .\install.ps1 -Help              # 도움말
#
# 작성일: 2025-11-10
# PowerShell 5.1+ 필요
##############################################################################

param(
    [switch]$SystemInstall = $false,
    [string]$InstallPrefix = "",
    [int]$Jobs = 4,
    [switch]$NoTests = $false,
    [switch]$Clean = $false,
    [switch]$Help = $false
)

# 도움말
if ($Help) {
    Write-Host @"

KingSejong 언어 설치 스크립트 (Windows)

사용법:
  .\install.ps1 [옵션]

옵션:
  -SystemInstall          시스템 전역 설치 (C:\Program Files\KingSejong)
  -InstallPrefix PATH     사용자 지정 경로에 설치
  -Jobs N                 병렬 빌드 작업 수 (기본: 4)
  -NoTests                테스트 실행 건너뛰기
  -Clean                  기존 빌드 삭제 후 재빌드
  -Help                   이 도움말 표시

예제:
  .\install.ps1                                # 로컬 빌드
  .\install.ps1 -SystemInstall                 # 시스템 설치
  .\install.ps1 -InstallPrefix "C:\KingSejong" # 사용자 지정 경로
  .\install.ps1 -Jobs 8 -NoTests               # 빠른 빌드

"@
    exit 0
}

# 색상 함수
function Write-Info {
    param([string]$Message)
    Write-Host "[INFO] $Message" -ForegroundColor Blue
}

function Write-Success {
    param([string]$Message)
    Write-Host "[SUCCESS] $Message" -ForegroundColor Green
}

function Write-Warning2 {
    param([string]$Message)
    Write-Host "[WARNING] $Message" -ForegroundColor Yellow
}

function Write-Error2 {
    param([string]$Message)
    Write-Host "[ERROR] $Message" -ForegroundColor Red
}

# 배너
Write-Host ""
Write-Host "=================================" -ForegroundColor Cyan
Write-Host "  KingSejong 언어 설치 스크립트  " -ForegroundColor Cyan
Write-Host "=================================" -ForegroundColor Cyan
Write-Host ""

# 1. 관리자 권한 확인 (SystemInstall 시)
if ($SystemInstall) {
    $currentPrincipal = New-Object Security.Principal.WindowsPrincipal([Security.Principal.WindowsIdentity]::GetCurrent())
    $isAdmin = $currentPrincipal.IsInRole([Security.Principal.WindowsBuiltInRole]::Administrator)

    if (-not $isAdmin) {
        Write-Error2 "시스템 설치를 위해서는 관리자 권한이 필요합니다."
        Write-Info "PowerShell을 관리자 권한으로 실행하세요."
        exit 1
    }
}

# 2. 플랫폼 정보
Write-Info "플랫폼 감지 중..."
$OS = "Windows"
$Arch = $env:PROCESSOR_ARCHITECTURE
Write-Info "플랫폼: $OS ($Arch)"

# 3. 필수 도구 확인
Write-Info "필수 도구 확인 중..."

function Test-Command {
    param([string]$Command)
    try {
        if (Get-Command $Command -ErrorAction Stop) {
            return $true
        }
    }
    catch {
        return $false
    }
}

# Git 확인
if (-not (Test-Command "git")) {
    Write-Error2 "Git을 찾을 수 없습니다."
    Write-Info "Git for Windows를 설치하세요: https://git-scm.com/download/win"
    exit 1
}
Write-Success "Git 확인됨: $(Get-Command git | Select-Object -ExpandProperty Source)"

# CMake 확인
if (-not (Test-Command "cmake")) {
    Write-Error2 "CMake를 찾을 수 없습니다."
    Write-Info "CMake를 설치하세요: https://cmake.org/download/"
    exit 1
}
Write-Success "CMake 확인됨: $(Get-Command cmake | Select-Object -ExpandProperty Source)"

# Visual Studio / C++ 컴파일러 확인
$vsWhere = "${env:ProgramFiles(x86)}\Microsoft Visual Studio\Installer\vswhere.exe"
if (Test-Path $vsWhere) {
    $vsPath = & $vsWhere -latest -property installationPath
    if ($vsPath) {
        Write-Success "Visual Studio 확인됨: $vsPath"
    }
}
else {
    Write-Warning2 "Visual Studio를 찾을 수 없습니다. MinGW 또는 다른 컴파일러를 사용합니다."
}

# 4. CMake 버전 확인
$cmakeVersion = (cmake --version | Select-String -Pattern "\d+\.\d+\.\d+").Matches[0].Value
Write-Info "CMake 버전: $cmakeVersion (요구사항: 3.20+)"

# 5. 기존 빌드 정리
if ($Clean -and (Test-Path "build")) {
    Write-Warning2 "기존 빌드 디렉토리 삭제 중..."
    Remove-Item -Recurse -Force "build"
    Write-Success "빌드 디렉토리 삭제 완료"
}

# 6. 빌드 디렉토리 생성
Write-Info "빌드 디렉토리 준비 중..."
if (-not (Test-Path "build")) {
    New-Item -ItemType Directory -Path "build" | Out-Null
}
Set-Location "build"

# 7. CMake 설정
Write-Info "CMake 설정 중..."

$cmakeArgs = @()

# 설치 경로 설정
if ($SystemInstall) {
    $cmakeArgs += "-DCMAKE_INSTALL_PREFIX=C:\Program Files\KingSejong"
    Write-Info "시스템 설치 모드: C:\Program Files\KingSejong"
}
elseif ($InstallPrefix) {
    $cmakeArgs += "-DCMAKE_INSTALL_PREFIX=$InstallPrefix"
    Write-Info "사용자 지정 설치 경로: $InstallPrefix"
}

# Release 빌드
$cmakeArgs += "-DCMAKE_BUILD_TYPE=Release"

# Visual Studio Generator 선택
if (Test-Path $vsWhere) {
    $cmakeArgs += "-G"
    $cmakeArgs += "Visual Studio 17 2022"
    $cmakeArgs += "-A"
    $cmakeArgs += "x64"
}

# CMake 실행
& cmake .. $cmakeArgs

if ($LASTEXITCODE -ne 0) {
    Write-Error2 "CMake 설정 실패"
    Set-Location ..
    exit 1
}
Write-Success "CMake 설정 완료"

# 8. 빌드
Write-Info "빌드 중... (병렬 작업 수: $Jobs)"

& cmake --build . --config Release --parallel $Jobs

if ($LASTEXITCODE -ne 0) {
    Write-Error2 "빌드 실패"
    Set-Location ..
    exit 1
}
Write-Success "빌드 완료"

# 9. 테스트 실행
if (-not $NoTests) {
    Write-Info "테스트 실행 중..."
    & ctest --output-on-failure -C Release
    if ($LASTEXITCODE -eq 0) {
        Write-Success "모든 테스트 통과"
    }
    else {
        Write-Warning2 "일부 테스트 실패 (무시하고 계속)"
    }
}
else {
    Write-Warning2 "테스트 건너뛰기 (-NoTests)"
}

# 10. 설치
if ($SystemInstall -or $InstallPrefix) {
    Write-Info "설치 중..."

    & cmake --install . --config Release

    if ($LASTEXITCODE -eq 0) {
        Write-Success "설치 완료"

        if ($SystemInstall) {
            $installPath = "C:\Program Files\KingSejong\bin\kingsejong.exe"
        }
        else {
            $installPath = "$InstallPrefix\bin\kingsejong.exe"
        }

        Write-Info "실행 파일 위치: $installPath"

        # PATH 환경 변수 추가 제안
        Write-Info ""
        Write-Info "PATH에 추가하려면 다음 명령을 실행하세요:"
        if ($SystemInstall) {
            Write-Host '  setx PATH "%PATH%;C:\Program Files\KingSejong\bin"' -ForegroundColor Yellow
        }
        else {
            Write-Host "  setx PATH `"%PATH%;$InstallPrefix\bin`"" -ForegroundColor Yellow
        }
    }
    else {
        Write-Error2 "설치 실패"
        Set-Location ..
        exit 1
    }
}
else {
    Write-Info "로컬 빌드 완료 (설치하지 않음)"
    Write-Info "실행 파일 위치: $(Get-Location)\bin\Release\kingsejong.exe"
}

# 11. 설치 확인
Write-Info "설치 확인 중..."

Set-Location ..

if ($SystemInstall -or $InstallPrefix) {
    $kingsejongBin = "kingsejong.exe"
}
else {
    $kingsejongBin = ".\build\bin\Release\kingsejong.exe"
}

if (Test-Path $kingsejongBin) {
    Write-Host ""
    Write-Host "====================================" -ForegroundColor Green
    Write-Success "KingSejong 설치 성공! 🎉"
    Write-Host "====================================" -ForegroundColor Green
    Write-Host ""

    # 버전 확인
    try {
        $version = & $kingsejongBin --version 2>$null
        if (-not $version) { $version = "unknown" }
    }
    catch {
        $version = "unknown"
    }

    Write-Info "버전: $version"
    Write-Host ""

    # 사용법 안내
    Write-Host "사용법:" -ForegroundColor Cyan
    Write-Host ""

    if ($SystemInstall -or $InstallPrefix) {
        Write-Host "  kingsejong                    # REPL 시작"
        Write-Host "  kingsejong script.ksj         # 스크립트 실행"
        Write-Host "  kingsejong --help             # 도움말"
    }
    else {
        Write-Host "  .\build\bin\Release\kingsejong.exe                    # REPL 시작"
        Write-Host "  .\build\bin\Release\kingsejong.exe script.ksj         # 스크립트 실행"
        Write-Host "  .\build\bin\Release\kingsejong.exe --help             # 도움말"
    }

    Write-Host ""
    Write-Host "예제 실행:" -ForegroundColor Cyan
    Write-Host "  $kingsejongBin examples\hello.ksj"
    Write-Host ""
    Write-Info "문서: docs\INSTALL.md"
    Write-Info "튜토리얼: docs\TUTORIAL.md"
    Write-Info "예제: examples\"
    Write-Host ""
}
else {
    Write-Error2 "설치 확인 실패: 실행 파일을 찾을 수 없습니다"
    exit 1
}

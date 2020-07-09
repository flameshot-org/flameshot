; -- Example3.iss --
; Same as Example1.iss, but creates some registry entries too and allows the end
; use to choose the install mode (administrative or non administrative).

; SEE THE DOCUMENTATION FOR DETAILS ON CREATING .ISS SCRIPT FILES!

[Setup]
AppName=Flameshot
AppVersion=0.7.3
AppCopyright=NameCheap inc.
VersionInfoVersion=0.7.3
WizardStyle=modern
DefaultDirName={autopf}\Flameshot
DefaultGroupName=Flameshot
UninstallDisplayIcon={app}\flameshot.exe
Compression=lzma2
SolidCompression=yes
;OutputDir=userdocs:Inno Setup Examples Output
OutputBaseFilename=Flameshot-Setup
ChangesAssociations=yes
UserInfoPage=yes
PrivilegesRequiredOverridesAllowed=dialog
AppPublisher=Namecheap, Inc.
AppPublisherURL=https://www.namecheap.com/
; "C:\Program Files\Microsoft Platform SDK for Windows Server 2003 R2\Bin\signtool.exe" sign /f "C:\MY_CODE_SIGNING.PFX" /t http://timestamp.comodoca.com/authenticode /p MY_PASSWORD $f


[Files]
Source: "flameshot\*"; DestDir: "{app}"
Source: "flameshot\bearer\*"; DestDir: "{app}\bearer"
Source: "flameshot\iconengines\*"; DestDir: "{app}\iconengines"
Source: "flameshot\imageformats\*"; DestDir: "{app}\imageformats"
Source: "flameshot\platforms\*"; DestDir: "{app}\platforms"
Source: "flameshot\translations\*"; DestDir: "{app}\translations"

[Icons]
Name: "{commondesktop}\Flameshot"; Filename: "{app}\flameshot.exe"; WorkingDir: "{app}"
Name: "{group}\Flameshot"; Filename: "{app}\flameshot.exe"; WorkingDir: "{app}"
Name: "{group}\FlameShot Documentation"; Filename: "{app}\flameshot-documentation.pdf"; WorkingDir: "{app}"

; NOTE: Most apps do not need registry entries to be pre-created. If you
; don't know what the registry is or if you need to use it, then chances are
; you don't need a [Registry] section.


[UninstallRun]
Filename: "taskkill"; Parameters: "/im ""flameshot.exe"" /f"; Flags: runhidden

[Code]
function ShouldSkipPage(PageID: Integer): Boolean;
begin
  // User specific pages should be skipped in administrative install mode
  Result := IsAdminInstallMode and (PageID = wpUserInfo);
end;



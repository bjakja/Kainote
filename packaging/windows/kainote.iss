; Kainote installer. Built by build-installer.ps1, which supplies AppVersion,
; PayloadDir and OutputDir; associations.iss is generated from
; Kainote/FileTypes.h by gen_associations.py.
;
; Per-machine by design. That is only viable because Kainote no longer writes
; into its own directory -- see config::InitPaths. The installer deliberately
; does not ship portable.txt, so an installed copy uses %APPDATA% while the
; zip keeps writing beside the executable.

#ifndef AppVersion
  #error AppVersion must be passed with /DAppVersion=...
#endif
#ifndef PayloadDir
  #error PayloadDir must be passed with /DPayloadDir=...
#endif

[Setup]
; Never change AppId: it is what makes an upgrade an upgrade rather than a
; second entry in Add/Remove Programs.
AppId={{8C5E0F3A-6D21-4B7E-9A44-2F1C7E5B93D8}
AppName=Kainote
AppVersion={#AppVersion}
AppPublisher=Marcin Drob
AppPublisherURL=https://github.com/bjakja/Kainote
AppSupportURL=https://github.com/bjakja/Kainote/issues
AppUpdatesURL=https://github.com/bjakja/Kainote/releases
VersionInfoVersion={#AppVersion}

DefaultDirName={autopf}\Kainote
DefaultGroupName=Kainote
AllowNoIcons=yes
UsePreviousAppDir=yes
LicenseFile=..\..\LICENSE

; Machine-wide, so the HKLM associations below are writable.
PrivilegesRequired=admin
ArchitecturesAllowed=x64compatible
ArchitecturesInstallIn64BitMode=x64compatible

; Kainote links Shcore.lib and calls GetDpiForMonitor unconditionally, so the
; binary cannot load below 8.1 whatever the installer claims.
MinVersion=6.3

Compression=lzma2/max
SolidCompression=yes
WizardStyle=modern
SetupIconFile=..\..\Kainote\Bitmaps\KaiLargeIcon.ico
; IDI_KAINOTE_APP is resource id 1, the lowest, so the shell picks it without
; an explicit index.
UninstallDisplayIcon={app}\Kainote.exe
UninstallDisplayName=Kainote
ChangesAssociations=yes

; Inno's Restart Manager does not reliably see a wxWidgets window; the [Code]
; section looks for the class name the application itself uses instead.
CloseApplications=no
RestartApplications=no

; Signing is opt-in: build-installer.ps1 passes /Skainote=<command> and
; /DSignInstaller=1 when a certificate is configured, and nothing otherwise.
#ifdef SignInstaller
SignTool=kainote
SignedUninstaller=yes
#endif

OutputDir={#OutputDir}
OutputBaseFilename=Kainote-{#AppVersion}-x64-setup

[Languages]
Name: "polish"; MessagesFile: "compiler:Languages\Polish.isl"
Name: "english"; MessagesFile: "compiler:Default.isl"

[CustomMessages]
english.AssocSubs=Associate subtitle files (.ass .ssa .srt .sub)
english.AssocVideo=Associate video files
english.AssocTxt=Associate .txt files
english.AppDescription=Subtitle editor
polish.AssocSubs=Skojarz pliki napisów (.ass .ssa .srt .sub)
polish.AssocVideo=Skojarz pliki wideo
polish.AssocTxt=Skojarz pliki .txt
polish.AppDescription=Edytor napisów

[Tasks]
Name: "desktopicon"; Description: "{cm:CreateDesktopIcon}"; Flags: unchecked
Name: "assoc_subs";  Description: "{cm:AssocSubs}"
Name: "assoc_video"; Description: "{cm:AssocVideo}"; Flags: unchecked
; .txt is text/plain in all but name, so taking it machine-wide is its own
; decision rather than part of "subtitles".
Name: "assoc_txt";   Description: "{cm:AssocTxt}"; Flags: unchecked

[Files]
; package.py --flavor installer has already pruned the payload: no PDBs, no
; AVX build, no portable.txt. The excludes are belt and braces.
Source: "{#PayloadDir}\*"; DestDir: "{app}"; Flags: ignoreversion recursesubdirs createallsubdirs; \
    Excludes: "*.pdb,portable.txt,Kainote_AVX.exe"

[Icons]
Name: "{group}\Kainote"; Filename: "{app}\Kainote.exe"
Name: "{group}\{cm:UninstallProgram,Kainote}"; Filename: "{uninstallexe}"
Name: "{autodesktop}\Kainote"; Filename: "{app}\Kainote.exe"; Tasks: desktopicon

#include "associations.iss"

[Registry]
; Without a Capabilities registration the user cannot pick Kainote from
; Settings -> Default apps, only from a file's Open With.
Root: HKLM; Subkey: "Software\Kainote"; ValueType: string; ValueName: "InstallDir"; \
    ValueData: "{app}"; Flags: uninsdeletekey
Root: HKLM; Subkey: "Software\Kainote\Capabilities"; ValueType: string; \
    ValueName: "ApplicationName"; ValueData: "Kainote"
Root: HKLM; Subkey: "Software\Kainote\Capabilities"; ValueType: string; \
    ValueName: "ApplicationDescription"; ValueData: "{cm:AppDescription}"
Root: HKLM; Subkey: "Software\RegisteredApplications"; ValueType: string; \
    ValueName: "Kainote"; ValueData: "Software\Kainote\Capabilities"; Flags: uninsdeletevalue
Root: HKLM; Subkey: "Software\Classes\Applications\Kainote.exe\shell\open\command"; \
    ValueType: string; ValueData: """{app}\Kainote.exe"" ""%1"""; Flags: uninsdeletekey
Root: HKLM; Subkey: "Software\Microsoft\Windows\CurrentVersion\App Paths\Kainote.exe"; \
    ValueType: string; ValueData: "{app}\Kainote.exe"; Flags: uninsdeletekey

[InstallDelete]
; A zip extracted over an installed copy would otherwise make it portable.
Type: files; Name: "{app}\portable.txt"
; Retired when the file type icons moved into the executable.
Type: files; Name: "{app}\Icons.dll"

[UninstallDelete]
Type: files; Name: "{app}\MiniDump.dmp"
Type: dirifempty; Name: "{app}"

[Code]
// The window class kainoteApp.cpp:615 uses for its own second-instance
// handoff. AppMutex would be wrong here: wxSingleInstanceChecker's mutex name
// is internal to wxWidgets and would stop matching on an upgrade.
function KainoteIsRunning: Boolean;
begin
  Result := FindWindowByClassName('Kainote_main_windowNR') <> 0;
end;

function AskToClose(const Context: String): Boolean;
begin
  Result := True;
  while KainoteIsRunning do
  begin
    if MsgBox('Kainote jest uruchomiony. Zamknij go, aby kontynuować ' + Context + '.',
              mbError, MB_RETRYCANCEL) = IDCANCEL then
    begin
      Result := False;
      Exit;
    end;
  end;
end;

function InitializeSetup: Boolean;
begin
  Result := AskToClose('instalację');
end;

function InitializeUninstall: Boolean;
begin
  Result := AskToClose('deinstalację');
end;

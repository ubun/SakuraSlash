#define MyAppName "柯南杀"
#define MyAppVersion "3.0"
#define MyAppPublisher "天子会工作室"
#define MyAppURL "http://weibo.com/conanslash"
#define MyAppExeName "ConanSlash.exe"

[Setup]
; 注: AppId的值为单独标识该应用程序。
; 不要为其他安装程序使用相同的AppId值。
; (生成新的GUID，点击 工具|在IDE中生成GUID。)
AppId={{9EF3FB1B-0915-4C70-ACCC-EC68FCBA2107}
AppName={#MyAppName}
AppVersion={#MyAppVersion}AppVerName={#MyAppName} {#MyAppVersion}
AppPublisher={#MyAppPublisher}
AppPublisherURL={#MyAppURL}
AppSupportURL={#MyAppURL}
;AppUpdatesURL={#MyAppURL}

AppCopyright=?2010-2013 TianziClub QSanguosha Software
VersionInfoVersion=3.0.0.0
VersionInfoCompany=TianziClub Software
VersionInfoDescription=天子会水浒杀
VersionInfoTextVersion=5, 0, 0, 0

DefaultDirName={pf}\ConanSlash
;DisableDirPage=yes
DefaultGroupName={#MyAppName}
AllowNoIcons=yes
LicenseFile=readme1.txt
InfoBeforeFile=readme2.txt
InfoAfterFile=readme3.txt
OutputDir=d:\
OutputBaseFilename=Shuihusha{#MyAppVersion}-Setup
SetupIconFile=package.ico
;WizardImageFile=border.bmp
;WizardSmallImageFile=logo.bmp
Compression=lzma
SolidCompression=yes

[Languages]
Name: "chinesesimp"; MessagesFile: "compiler:Default.isl"

[Tasks]
Name: "desktopicon"; Description: "{cm:CreateDesktopIcon}"; GroupDescription: "{cm:AdditionalIcons}";
; OnlyBelowVersion: 0,6.1
Name: "quicklaunchicon"; Description: "{cm:CreateQuickLaunchIcon}"; GroupDescription: "{cm:AdditionalIcons}"; Flags: unchecked

[Components]
Name: main; Description: "主程序(必选)"; Types: full compact custom; Flags: fixed
Name: doc; Description: "参考文档"; Types: full;

[Files]
Source: "..\ConanSlash.exe"; DestDir: "{app}"; Flags: ignoreversion 

Source: "..\*"; DestDir: "{app}"; Excludes: "\inno\*,\swig\*,\propagate\*,\extension*\*,lua5*.dll,\config.ini"; Flags: ignoreversion recursesubdirs createallsubdirs; Components: main Source: "..\propagate\更新说明.txt"; DestDir: "{app}"; Flags: ignoreversion ; Components: main
;Source: "..\extensions\customcards.*"; DestDir: "{app}\extensions"; Flags: ignoreversion ; Components: main
;Source: "..\extensions\custom-cards.txt"; DestDir: "{app}\extensions"; Flags: ignoreversion ; Components: main
 
Source: "..\extension-doc\*"; DestDir: "{app}\extension-doc"; Flags: ignoreversion recursesubdirs createallsubdirs ; Components: doc
Source: "..\swig\sanguosha.i"; DestDir: "{app}\swig"; Flags: ignoreversion recursesubdirs createallsubdirs ; Components: doc
; 注意: 不要在任何共享系统文件上使用“Flags: ignoreversion”

[Icons]
Name: "{group}\{#MyAppName}"; Filename: "{app}\{#MyAppExeName}"
Name: "{group}\{cm:UninstallProgram,{#MyAppName}}"; Filename: "{uninstallexe}"
Name: "{commondesktop}\{#MyAppName}"; Filename: "{app}\{#MyAppExeName}"; Tasks: desktopicon
Name: "{userappdata}\Microsoft\Internet Explorer\Quick Launch\{#MyAppName}"; Filename: "{app}\{#MyAppExeName}"; Tasks: quicklaunchicon

[Run]
Filename: "{app}\{#MyAppExeName}"; Description: "{cm:LaunchProgram,{#StringChange(MyAppName, '&', '&&')}}"; Flags: nowait postinstall skipifsilent


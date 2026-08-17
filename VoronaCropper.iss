[Setup]

AppName=VoronaCropper
AppVersion=1.0.0
AppPublisher=VoronaCropper

DefaultDirName={autopf}\VoronaCropper


SetupIconFile=logo.ico

DefaultGroupName=VoronaCropper

OutputDir=.
OutputBaseFilename=VoronaCropper_Setup

Compression=lzma
SolidCompression=yes

ArchitecturesInstallIn64BitMode=x64


[Files]

Source: "VoronaCropper.exe"; DestDir: "{app}"

Source: "*.dll"; DestDir: "{app}"

Source: "platforms\*"; DestDir: "{app}\platforms"; Flags: recursesubdirs

Source: "styles\*"; DestDir: "{app}\styles"; Flags: recursesubdirs

Source: "light\*"; DestDir: "{app}\light"; Flags: recursesubdirs

Source: "logo.ico"; DestDir: "{app}"


[Icons]

Name: "{autoprograms}\VoronaCropper"; \
Filename: "{app}\VoronaCropper.exe"; \
IconFilename: "{app}\logo.ico"

Name: "{autodesktop}\VoronaCropper"; \
Filename: "{app}\VoronaCropper.exe"; \
IconFilename: "{app}\logo.ico"
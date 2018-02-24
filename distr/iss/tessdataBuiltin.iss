[Files]
Source: "tessdata\ara.*"; DestDir: "{app}\tessdata"; Components: Languages\Arabic; Flags: ignoreversion;
Source: "tessdata\chi_sim.*"; DestDir: "{app}\tessdata"; Components: Languages\Chineese_simplified; Flags: ignoreversion;
Source: "tessdata\eng.*"; DestDir: "{app}\tessdata"; Components: Languages\English; Flags: ignoreversion;
Source: "tessdata\fra.*"; DestDir: "{app}\tessdata"; Components: Languages\French; Flags: ignoreversion;
Source: "tessdata\deu.*"; DestDir: "{app}\tessdata"; Components: Languages\German; Flags: ignoreversion;
Source: "tessdata\jpn.*"; DestDir: "{app}\tessdata"; Components: Languages\Japanese; Flags: ignoreversion;
Source: "tessdata\rus.*"; DestDir: "{app}\tessdata"; Components: Languages\Russian; Flags: ignoreversion;
Source: "tessdata\spa.*"; DestDir: "{app}\tessdata"; Components: Languages\Spanish; Flags: ignoreversion; 

[Components]
Name: "Languages"; Description: "{cm:Languages}"; Types: custom 

Name: "Languages\Arabic"; Description: "{cm:Arabic}"; Languages: en; Types: custom; ExtraDiskSpaceRequired: 28534834
Name: "Languages\Chineese_simplified"; Description: "{cm:Chineese_simplified}"; Languages: en; Types: custom; ExtraDiskSpaceRequired: 42089623
Name: "Languages\English"; Description: "{cm:English}"; Languages: en; Types: custom; ExtraDiskSpaceRequired: 38371506
Name: "Languages\French"; Description: "{cm:French}"; Languages: en; Types: custom; ExtraDiskSpaceRequired: 37350210
Name: "Languages\German"; Description: "{cm:German}"; Languages: en; Types: custom; ExtraDiskSpaceRequired: 13367187
Name: "Languages\Japanese"; Description: "{cm:Japanese}"; Languages: en; Types: custom; ExtraDiskSpaceRequired: 33072615
Name: "Languages\Russian"; Description: "{cm:Russian}"; Languages: en; Types: custom; ExtraDiskSpaceRequired: 39371802
Name: "Languages\Spanish"; Description: "{cm:Spanish}"; Languages: en; Types: custom; ExtraDiskSpaceRequired: 39171233

Name: "Languages\English"; Description: "{cm:English}"; Languages: ru; Types: custom; ExtraDiskSpaceRequired: 38371506
Name: "Languages\Arabic"; Description: "{cm:Arabic}"; Languages: ru; Types: custom; ExtraDiskSpaceRequired: 28534834
Name: "Languages\Spanish"; Description: "{cm:Spanish}"; Languages: ru; Types: custom; ExtraDiskSpaceRequired: 39171233
Name: "Languages\Chineese_simplified"; Description: "{cm:Chineese_simplified}"; Languages: ru; Types: custom; ExtraDiskSpaceRequired: 42089623
Name: "Languages\German"; Description: "{cm:German}"; Languages: ru; Types: custom; ExtraDiskSpaceRequired: 13367187
Name: "Languages\Russian"; Description: "{cm:Russian}"; Languages: ru; Types: custom; ExtraDiskSpaceRequired: 39371802
Name: "Languages\French"; Description: "{cm:French}"; Languages: ru; Types: custom; ExtraDiskSpaceRequired: 37350210
Name: "Languages\Japanese"; Description: "{cm:Japanese}"; Languages: ru; Types: custom; ExtraDiskSpaceRequired: 33072615

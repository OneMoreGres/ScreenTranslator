// =====================================================================================================================
//    DwinsHs
//
//    Author: HAN-SOFT
//    E-Mail: support@han-soft.com
//    WebURL: http://www.han-soft.com
//    Copyright (C) 2001, 2015 Han-soft Corporation. All rights reserved.
// =====================================================================================================================
//    $Rev: 113 $  $Id: dwinshs.iss 113 2015-07-23 07:30:14Z hanjy $
// =====================================================================================================================

#define Use_DwinsHs

[Code]

type
  TCancelDownload = (cdNone, cdBack, cdExit);
  TProxyMode = (pmDefault, pmDirect, pmProxy);
  TProxyProtocol = (ppHttp, ppSocks);
  TDwinsHs_Proxy = record
    Mode: TProxyMode;
    Protocol: TProxyProtocol;
    Host: AnsiString;
    Port: Integer;
    Username: AnsiString;
    Password: AnsiString;
  end;

var
  DwinsHs_CancelDownload: TCancelDownload;
  DwinsHs_Proxy: TDwinsHs_Proxy;

const
  PM_NOREMOVE = $0000;
  PM_REMOVE = $0001;
  PM_NOYIELD = $0002;
  WM_QUIT = $0012;

type
  TMsg = record
    hWnd: HWnd;
    Msg: LongInt;
    wParam: LongInt;
    lParam: LongInt;
    Time: LongInt;
    Pt: TPoint;
  end;

function PeekMessage(var Msg: TMsg; Wnd: HWnd; MsgFilterMin, MsgFilterMax, wRemoveMsg: LongInt): BOOL;
external 'PeekMessageA@user32.dll stdcall delayload setuponly';

function TranslateMessage(var Msg: TMsg): BOOL;
external 'TranslateMessage@user32.dll stdcall delayload setuponly';

function DispatchMessage(var Msg: TMsg): LongInt;
external 'DispatchMessageA@user32.dll stdcall delayload setuponly';

procedure DwinsHs_ProcessMessages();
var
  Msg: TMsg;
begin
  while PeekMessage(Msg, 0, 0, 0, PM_REMOVE) do
  if Msg.Msg <> WM_QUIT then
  begin
    TranslateMessage(Msg);
    DispatchMessage(Msg);
  end
  else
  begin
    DwinsHs_CancelDownload := cdExit;
  end;
end;

// =====================================================================================================================

type
  Pointer = Cardinal;
  HINTERNET = Pointer;
  PLPSTR = Pointer;
  LPDWORD = DWORD_PTR;

function InternetOpen(lpszAgent: PAnsiChar; dwAccessType: DWORD; lpszProxy, lpszProxyBypass: PAnsiChar;
  dwFlags: DWORD): HINTERNET;
external 'InternetOpenA@wininet.dll stdcall delayload setuponly';

function InternetSetOption(hInet: HINTERNET; dwOption: DWORD; lpBuffer: PAnsiChar; dwBufferLength: DWORD): BOOL;
external 'InternetSetOptionA@wininet.dll stdcall delayload setuponly';

function InternetConnect(hInet: HINTERNET; lpszServerName: PAnsiChar; nServerPort: Word; lpszUsername: PAnsiChar;
  lpszPassword: PAnsiChar; dwService: DWORD; dwFlags: DWORD; dwContext: DWORD_PTR): HINTERNET;
external 'InternetConnectA@wininet.dll stdcall delayload setuponly';

function HttpOpenRequest(hConnect: HINTERNET; lpszVerb: PAnsiChar; lpszObjectName: PAnsiChar; lpszVersion: PAnsiChar;
  lpszReferrer: PAnsiChar; lplpszAcceptTypes: PLPSTR; dwFlags: DWORD; dwContext: DWORD_PTR): HINTERNET;
external 'HttpOpenRequestA@wininet.dll stdcall delayload setuponly';

function HttpSendRequest(hRequest: HINTERNET; lpszHeaders: PAnsiChar; dwHeadersLength: DWORD; lpOptional: PAnsiChar;
  dwOptionalLength: DWORD): BOOL;
external 'HttpSendRequestA@wininet.dll stdcall delayload setuponly';

function HttpQueryInfo(hRequest: HINTERNET; dwInfoLevel: DWORD; lpvBuffer: PAnsiChar; var lpdwBufferLength: DWORD;
  var lpdwReserved: DWORD): BOOL;
external 'HttpQueryInfoA@wininet.dll stdcall delayload setuponly';

function FtpOpenFile(hConnect: HINTERNET; lpszFileName: PAnsiChar; dwAccess: DWORD; dwFlags: DWORD;
  dwContext: DWORD): HINTERNET;
external 'FtpOpenFileA@wininet.dll stdcall delayload setuponly';

function FtpGetFileSize(hFile: HINTERNET; lpdwFileSizeHigh: DWORD): DWORD;
external 'FtpGetFileSize@wininet.dll stdcall delayload setuponly';

function InternetReadFile(hFile: HINTERNET; lpBuffer: PAnsiChar; dwNumberOfBytesToRead: DWORD;
  var lpdwNumberOfBytesRead: DWORD): BOOL;
external 'InternetReadFile@wininet.dll stdcall delayload setuponly';

function InternetCloseHandle(hInet: HINTERNET): BOOL;
external 'InternetCloseHandle@wininet.dll stdcall delayload setuponly';

function InternetGetConnectedState(lpdwFlags: PAnsiChar; dwReserved: DWORD): BOOL;
external 'InternetGetConnectedState@wininet.dll stdcall delayload setuponly';

const
  INTERNET_CONNECTION_OFFLINE = 32;
  INTERNET_OPEN_TYPE_PRECONFIG = 0;
  INTERNET_OPEN_TYPE_DIRECT = 1;
  INTERNET_OPEN_TYPE_PROXY = 3;
  INTERNET_OPTION_PROXY_USERNAME = 43;
  INTERNET_OPTION_PROXY_PASSWORD = 44;
  INTERNET_SERVICE_FTP = 1;
  INTERNET_SERVICE_HTTP = 3;
  INTERNET_FLAG_RELOAD = $80000000;
  INTERNET_FLAG_SECURE = $00800000;
  INTERNET_FLAG_IGNORE_CERT_CN_INVALID = $00001000;
  INTERNET_FLAG_IGNORE_CERT_DATE_INVALID = $00002000;
  INTERNET_FLAG_EXISTING_CONNECT = $20000000;
  INTERNET_FLAG_PASSIVE = $08000000;
  GENERIC_READ = $80000000;
  FTP_TRANSFER_TYPE_BINARY = $00000002;
  HTTP_HEADER = 'Content-Type: application/x-www-form-urlencoded';
  HTTP_QUERY_CONTENT_LENGTH = 5;
  HTTP_QUERY_STATUS_CODE = 19;

  #ifndef DwinsHs_Data_Buffer_Length
    #define DwinsHs_Data_Buffer_Length 4096
  #endif
  READ_BUFFER_LENGTH = {#DwinsHs_Data_Buffer_Length};

  CONNECT_OK = 0;
  CONNECT_ERROR_NETWORK = 1;
  CONNECT_ERROR_OFFLINE = 2;
  CONNECT_ERROR_INITIALIZE = 3;
  CONNECT_ERROR_OPENSESSION = 4;
  CONNECT_ERROR_CREATEREQUEST = 5;
  CONNECT_ERROR_SENDREQUEST = 6;

  READ_OK = 0;
  READ_ERROR_DELETEFILE = 7;
  READ_ERROR_SAVEFILE = 8;
  READ_ERROR_CANCELED = 9;
  READ_ERROR_READDATA = 10;

type
  TReadScheme = (rpHttp, rpHttps, rpFtp);
  TReadMethod = (rmGet, rmPost, rmActive, rmPassive);
  TOnRead = function(URL, Agent: AnsiString; Method: TReadMethod; Index, TotalSize, ReadSize, CurrentSize: LongInt;
    var ReadStr: AnsiString): Boolean;

procedure ParseURL(URL: AnsiString; var Scheme: TReadScheme; var Host, Path, Data, Username, Password: AnsiString;
  var Port: Integer);
var
  i: Integer;
begin
  URL := Trim(URL);
  Scheme := rpHttp;
  Port := 80;
  i := Pos('://', URL);
  if i > 0 then
  begin
    case LowerCase(Trim(Copy(URL, 1, i - 1))) of
      'http':
      begin
        Port := 80;
        Scheme := rpHttp;
      end;
      'https':
      begin
        Port := 443;
        Scheme := rpHttps;
      end;
      'ftp':
      begin
        Port := 21;
        Scheme := rpFtp;
      end;
    end;
    URL := Trim(Copy(URL, i + 3, 65535));
  end;

  i := Pos('/', URL);
  if i > 0 then
  begin
    Host := Trim(Copy(URL, 1, i - 1));
    Path := Trim(Copy(URL, i, 65535));
  end
  else
  begin
    Host := URL;
    Path := '';
  end;

  i := Pos('@', Host);
  if i > 0 then
  begin
    Username := Trim(Copy(Host, 1, i - 1));
    Host := Trim(Copy(Host, i + 1, 65535));
  end
  else
  begin
    Username := '';
  end;

  i := Pos(':', Username);
  if i > 0 then
  begin
    Password := Trim(Copy(Username, i + 1, 65535));
    Username := Trim(Copy(Username, 1, i - 1));
  end
  else
  begin
    Password := '';
  end;

  i := Pos(':', Host);
  if i > 0 then
  begin
    Port := StrToIntDef(Trim(Copy(Host, i + 1, 65535)), Port);
    Host := Trim(Copy(Host, 1, i -1));
  end;

  i := Pos('?', Path);
  if i > 0 then
  begin
    Data := Trim(Copy(Path, i + 1, 65536));
    Path := Trim(Copy(Path, 1, i - 1));
  end
  else
    Data := '';
  Host := LowerCase(Host);
end;

procedure DwinsHs_SetProxy(Mode: TProxyMode; Protocol: TProxyProtocol; Host: AnsiString; Port: Integer;
  Username, Password: AnsiString);
begin
  DwinsHs_Proxy.Mode := Mode;
  DwinsHs_Proxy.Protocol := Protocol;
  DwinsHs_Proxy.Host := Host;
  DwinsHs_Proxy.Port := Port;
  DwinsHs_Proxy.Username := Username;
  DwinsHs_Proxy.Password := Password;
end;

function CreateConnect(URL, Agent: AnsiString; Method: TReadMethod; var hSession, hConnect, hRequest: HINTERNET;
  var Size: LongInt): Integer;
var
  Scheme: TReadScheme;
  ProxyProtocolName, Host, Path, Data, Username, Password: AnsiString;
  Port, i: Integer;
  Buffer: AnsiString;
  Flag, Bytes: DWord;
  Sended: Boolean;
begin
  ParseUrl(URL, Scheme, Host, Path, Data, Username, Password, Port);
  Size := -1;

  Result := CONNECT_ERROR_NETWORK;
  Buffer := '0000';
  if not InternetGetConnectedState(PAnsiChar(Buffer), 0) then Exit;
  Result := CONNECT_ERROR_OFFLINE;
  Flag := Ord(Buffer[1]) + Ord(Buffer[2]) * 256 + Ord(Buffer[3]) * 65536 + Ord(Buffer[4]) * 16777216;
  if (Flag and INTERNET_CONNECTION_OFFLINE) <> 0 then Exit;

  Result := CONNECT_ERROR_INITIALIZE;
  case DwinsHs_Proxy.Mode of
    pmDefault:
      hSession := InternetOpen(Agent, INTERNET_OPEN_TYPE_PRECONFIG, '', '', 0);
    pmDirect:
      hSession := InternetOpen(Agent, INTERNET_OPEN_TYPE_DIRECT, '', '', 0);
    pmProxy:
    begin
      case DwinsHs_Proxy.Protocol of
        ppHttp: ProxyProtocolName := 'HTTP';
        ppSocks: ProxyProtocolName := 'SOCKS';
      end;
      hSession := InternetOpen(Agent, INTERNET_OPEN_TYPE_PROXY, PAnsiChar(ProxyProtocolName + '=' + ProxyProtocolName +
        '://' + DwinsHs_Proxy.Host + ':' + IntToStr(DwinsHs_Proxy.Port)), '', 0);
      if DwinsHs_Proxy.Username <> '' then
        InternetSetOption(hSession, INTERNET_OPTION_PROXY_USERNAME, PAnsiChar(DwinsHs_Proxy.Username),
          Length(DwinsHs_Proxy.Username));
      if DwinsHs_Proxy.Password <> '' then
        InternetSetOption(hSession, INTERNET_OPTION_PROXY_PASSWORD, PAnsiChar(DwinsHs_Proxy.Password),
          Length(DwinsHs_Proxy.Password));
    end;
  end;
  if hSession = 0 then Exit;

  Result := CONNECT_ERROR_OPENSESSION;
  case Scheme of
    rpHttp, rpHttps:
      hConnect := InternetConnect(hSession, PAnsiChar(Host), Port, PAnsiChar(Username), PAnsiChar(Password),
        INTERNET_SERVICE_HTTP, 0, 0);
    rpFtp:
    begin
      if Method = rmPassive then Flag := INTERNET_FLAG_PASSIVE else Flag := 0;
      hConnect := InternetConnect(hSession, PAnsiChar(Host), Port, PAnsiChar(Username), PAnsiChar(Password),
        INTERNET_SERVICE_FTP, INTERNET_FLAG_EXISTING_CONNECT or Flag, 0);
    end;
  end;
  if hConnect = 0 then
  begin
    InternetCloseHandle(hSession);
    Exit;
  end;

  Result := CONNECT_ERROR_CREATEREQUEST;
  case Scheme of
    rpHttp, rpHttps:
    begin
      Flag := INTERNET_FLAG_RELOAD;
      if Scheme = rpHttps then
        Flag := Flag or INTERNET_FLAG_SECURE or INTERNET_FLAG_IGNORE_CERT_CN_INVALID or
          INTERNET_FLAG_IGNORE_CERT_DATE_INVALID;
      case Method of
        rmGet:
        begin
          if Data <> '' then Path := Path + '?' + Data;
          hRequest := HttpOpenRequest(hConnect, 'GET', PAnsiChar(Path),  'HTTP/1.0', '', 0, Flag, 0);
        end;
        rmPost:
          hRequest := HttpOpenRequest(hConnect, 'POST', PAnsiChar(Path), 'HTTP/1.0', '', 0, Flag, 0);
      end;
      if hRequest = 0 then
      begin
        InternetCloseHandle(hConnect);
        InternetCloseHandle(hSession);
        Exit;
      end;
      Result := CONNECT_ERROR_SENDREQUEST;
      case Method of
        rmGet: Sended := HttpSendRequest(hRequest, HTTP_HEADER, Length(HTTP_HEADER), '', 0);
        rmPost: Sended := HttpSendRequest(hRequest, HTTP_HEADER, Length(HTTP_HEADER), PAnsiChar(Data), Length(Data));
      end;
      if Sended then
      begin
        SetLength(Buffer, READ_BUFFER_LENGTH + 1);
        try
          Bytes := READ_BUFFER_LENGTH;
          Flag := 0;
          if HttpQueryInfo(hRequest, HTTP_QUERY_STATUS_CODE, PAnsiChar(Buffer), Bytes, Flag) then
          begin
            Flag := 0;
            for i := 1 to Bytes do Flag := Flag * 10 + Ord(Buffer[i]) - 48;
          end;
          if Flag >= 400 then
          begin
            Result := Flag;
          end
          else
          begin
            Bytes := READ_BUFFER_LENGTH;
            Flag := 0;
            if HttpQueryInfo(hRequest, HTTP_QUERY_CONTENT_LENGTH, PAnsiChar(Buffer), Bytes, Flag) then
            begin
              Size := 0;
              for i := 1 to Bytes do Size := Size * 10 + Ord(Buffer[i]) - 48;
            end;
            Result := CONNECT_OK;
          end;
        finally
          SetLength(Buffer, 0);
        end;
      end;
      if Result <> CONNECT_OK then
      begin
        InternetCloseHandle(hConnect);
        InternetCloseHandle(hSession);
        InternetCloseHandle(hRequest);
        Exit;
      end;
    end;
    rpFtp:
    begin
      hRequest :=
        FtpOpenFile(hConnect, PAnsiChar(Path), GENERIC_READ, FTP_TRANSFER_TYPE_BINARY or INTERNET_FLAG_RELOAD, 0);
      if hRequest = 0 then
      begin
        InternetCloseHandle(hConnect);
        InternetCloseHandle(hSession);
        Exit;
      end;
      Size := FtpGetFileSize(hRequest, 0);
      Result := CONNECT_OK;
    end;
  end;
end;

function DwinsHs_GetRemoteSize(URL, Agent: AnsiString; Method: TReadMethod): LongInt;
var
  hSession, hConnect, hRequest: HINTERNET;
  Status: Integer;
begin
  Status := CreateConnect(URL, Agent, Method, hSession, hConnect, hRequest, Result);
  if Status <> CONNECT_OK then
  begin
    Result := - Status;
    Exit;
  end;
  InternetCloseHandle(hConnect);
  InternetCloseHandle(hSession);
  InternetCloseHandle(hRequest);
end;

function DwinsHs_ReadRemoteURL(URL, Agent: AnsiString; Method: TReadMethod; var Response: AnsiString; var Size: LongInt;
  SaveFilename: string; OnRead: TOnRead): Integer;
var
  hSession, hConnect, hRequest: HINTERNET;
  Buffer, ReadStr: AnsiString;
  ToContinue: Boolean;
  Bytes: DWORD;
  Readed, Index: LongInt;
begin
  Result := CreateConnect(URL, Agent, Method, hSession, hConnect, hRequest, Size);
  if Result <> CONNECT_OK then Exit;
  SetLength(Buffer, READ_BUFFER_LENGTH + 1);
  try
    Index := 0;
    Readed := 0;
    ToContinue := True;
    while ToContinue do
    begin
      if InternetReadFile(hRequest, PAnsiChar(Buffer), READ_BUFFER_LENGTH, Bytes) then
      begin
        if Bytes > 0 then
        begin
          Readed := Readed + Bytes;
          ReadStr := Copy(Buffer, 1, Bytes);
          if SaveFilename <> '' then
          begin
            if (Index = 0) and FileExists(SaveFilename) then
            begin
              Result := READ_ERROR_DELETEFILE;
              if not DeleteFile(SaveFilename) then Break;
            end;
            Result := READ_ERROR_SAVEFILE;
            if not SaveStringToFile(SaveFilename, ReadStr, Index <> 0) then Break;
          end;
          if OnRead <> nil then ToContinue := OnRead(URL, Agent, Method, Index, Size, Readed, Bytes, ReadStr);
          Response := Response + ReadStr;
          Result := READ_OK;
        end
        else
        begin
          ReadStr := '';
          if OnRead <> nil then ToContinue := OnRead(URL, Agent, Method, Index, Size, Readed, 0, ReadStr);
          Response := Response + ReadStr;
          Break;
        end;
        if (not ToContinue) or (DwinsHs_CancelDownload <> cdNone) then
        begin
          ToContinue := False;
          Result := READ_ERROR_CANCELED;
        end;
        DwinsHs_ProcessMessages();
      end
      else
      begin
        ReadStr := '';
        Response := '';
        if SaveFilename <> '' then DeleteFile(SaveFilename);
        if OnRead <> nil then ToContinue := OnRead(URL, Agent, Method, -1, Size, Readed, 0, ReadStr);
        Result := READ_ERROR_READDATA;
        Break;
      end;
      Index := Index + 1;
    end;
  finally
    SetLength(Buffer, 0);
    InternetCloseHandle(hConnect);
    InternetCloseHandle(hSession);
    InternetCloseHandle(hRequest);
  end;
end;

// =====================================================================================================================

#ifdef DwinsHs_Use_Predefined_Downloading_WizardPage

[CustomMessages]

#ifndef DwinsHs_Disable_Default_CustomMessages

DwinsHs_PageCaption =Downloading additional files
DwinsHs_PageDescription =Please wait while setup downloads additional files...
DwinsHs_TotalProgress =Total progress:
DwinsHs_CurrentFile =Current file:
DwinsHs_File =File:
DwinsHs_Speed =Speed:
DwinsHs_Status =Status:
DwinsHs_ElapsedTime =Elapsed time:
DwinsHs_RemainingTime =Remaining time:
DwinsHs_Status_ButtonRetry =&Retry
DwinsHs_Status_ButtonNext =&Next >

DwinsHs_SizeInBytes =%d Bytes
DwinsHs_SizeInKB =%.2f KB
DwinsHs_SizeInMB =%.2f MB
DwinsHs_ProgressValue = %s of %s (%d%%%)
DwinsHs_SpeedInBytes =%d Bytes/s
DwinsHs_SpeedInKB =%.2f KB/s
DwinsHs_SpeedInMB =%.2f MB/s
DwinsHs_TimeInHour =%d hour(s), %d minute(s), %d second(s)
DwinsHs_TimeInMinute =%d minute(s), %d second(s)
DwinsHs_TimeInSecond =%d second(s)

DwinsHs_Status_GetFileInformation =Fetch file size
DwinsHs_Status_StartingDownload =Starting to download
DwinsHs_Status_Downloading =Downloading
DwinsHs_Status_DownlaodComplete =Download completes

DwinsHs_Error_Network =No active Internet connection
DwinsHs_Error_Offline =The computer is in offline mode
DwinsHs_Error_Initialize =Failed to initialize the setup
DwinsHs_Error_OpenSession =Failed to open the FTP or HTTP session
DwinsHs_Error_CreateRequest =Failed to create an HTTP request handle
DwinsHs_Error_SendRequest =Failed to send request to the HTTP server
DwinsHs_Error_DeleteFile =The old file cannot be deleted
DwinsHs_Error_SaveFile =Failed to save data
DwinsHs_Error_Canceled =Download canceled
DwinsHs_Error_ReadData =Failed to read data

DwinsHs_Status_HTTPError =HTTP error %d: %s
DwinsHs_Status_HTTP400 =Bad request
DwinsHs_Status_HTTP401 =Unauthorized
DwinsHs_Status_HTTP404 =Not found
DwinsHs_Status_HTTP407 =Proxy authentication required
DwinsHs_Status_HTTP500 =Internal error
DwinsHs_Status_HTTP502 =Bad gateway
DwinsHs_Status_HTTP503 =Service unavailable
DwinsHs_Status_HTTPxxx =Other error

#endif

[Code]

function FormatURL(URL: AnsiString): AnsiString;
var
  Scheme: TReadScheme;
  SchemeTxt, Host, Path, Username, Password, Data: AnsiString;
  Port: Integer;
begin
  ParseURL(URL, Scheme, Host, Path, Data, Username, Password, Port);
  case Scheme of
    rpHttp: SchemeTxt := 'http';
    rpHttps: SchemeTxt := 'https';
    rpFtp: SchemeTxt := 'ftp';
  end;
  Result := SchemeTxt + '://';
  if (Username <> '') or (Password <> '') then
  begin
    Host := '@' + Host;
    if Username <> '' then Result := Result + Username;
    if Password <> '' then Result := Result + ':' + Password;
  end;
  Result := Result + Host + ':' + IntToStr(Port) + Path;
  if (Data <> '') and ((Scheme = rpHttp) or (Scheme = rpHttps)) then Result := Result + '?' + Data;
end;

function FormatSize(Size: LongInt): string;
begin
  if Size < 1024 then
    Result := Format(ExpandConstant('{cm:DwinsHs_SizeInBytes}'), [Size])
  else if Size < 1048576 then
    Result := Format(ExpandConstant('{cm:DwinsHs_SizeInKB}'), [Double(Size)/1024])
  else
    Result := Format(ExpandConstant('{cm:DwinsHs_SizeInMB}'), [Double(Size)/1048576])
end;

function FormatSpeed(Speed: LongInt): string;
begin
  if Speed < 1024 then
    Result := Format(ExpandConstant('{cm:DwinsHs_SpeedInBytes}'), [Speed])
  else if Speed < 1048576 then
    Result := Format(ExpandConstant('{cm:DwinsHs_SpeedInKB}'), [Double(Speed)/1024])
  else
    Result := Format(ExpandConstant('{cm:DwinsHs_SpeedInMB}'), [Double(Speed)/1048576])
end;

function FormatTime(Seconds: LongInt): string;
begin
  if Seconds >= 3600 then
    Result := Format(ExpandConstant('{cm:DwinsHs_TimeInHour}'), [Seconds div 3600, (Seconds mod 3600) div 60,
      Seconds mod 60])
  else if Seconds >= 60 then
    Result := Format(ExpandConstant('{cm:DwinsHs_TimeInMinute}'), [Seconds div 60, Seconds mod 60])
  else
    Result := Format(ExpandConstant('{cm:DwinsHs_TimeInSecond}'), [Seconds]);
end;

function DecodeDateTimeString(DateTime: string): LongInt;
var
  Year, Month, Day, Hour, Minute, Second: Integer;
  a, y, m: Integer;
begin
  Year := StrToIntDef(Copy(DateTime, 1, 4), 0);
  Month := StrToIntDef(Copy(DateTime, 6, 2), 0);
  Day := StrToIntDef(Copy(DateTime, 9, 2), 0);
  Hour := StrToIntDef(Copy(DateTime, 12, 2), 0);
  Minute := StrToIntDef(Copy(DateTime, 15, 2), 0);
  Second := StrToIntDef(Copy(DateTime, 18, 2), 0);
  a := (14 - Month) div 12;
  y := Year + 4800 - a;
  m := Month + 12 * a - 3;
  Result := (Day + (153 * m + 2) div 5 + 365 * y + y div 4 - y div 100 + y div 400 - 32045 - 2400000) * 86400 - 43200 +
    Hour * 3600 + Minute * 60 + Second;
end;

#ifndef Hs_CreateLabel
#define Hs_CreateLabel
function CreateLabel(OwnerControl: TComponent; Caption: string; Left, Top, Width: Integer; Wrap,
  RightAlignment: Boolean): TLabel;
begin
  Result := TLabel.Create(OwnerControl);
  if OwnerControl = WizardForm then
    Result.Parent := WizardForm
  else
    Result.Parent := TWizardPage(OwnerControl).Surface;
  Result.Caption := Caption;
  Result.Left := Left;
  Result.Top := Top;
  if Wrap or RightAlignment then Result.Width := Width;
  if RightAlignment then Result.Alignment := taRightJustify;
  Result.WordWrap := Wrap;
  Result.AutoSize := True;
end;
#endif

function CreateProgressBar(OwnerControl: TComponent; Left, Top, Width, Height, MinValue, MaxValue, CurValue: Integer):
  TNewProgressBar;
begin
  Result := TNewProgressBar.Create(OwnerControl);
  if OwnerControl = WizardForm then
    Result.Parent := WizardForm
  else
    Result.Parent := TWizardPage(OwnerControl).Surface;
  Result.Left := Left;
  Result.Top := Top;
  Result.Width := Width;
  Result.Height := Height;
  Result.Min := MinValue;
  Result.Max := MaxValue;
  Result.Position := CurValue;
end;

#ifndef Hs_ClickButton
#define Hs_ClickButton

const
  WM_LBUTTONDOWN = 513;
  WM_LBUTTONUP = 514;

procedure ClickNext();
begin
  PostMessage(WizardForm.NextButton.Handle, WM_LBUTTONDOWN,0,0);
  PostMessage(WizardForm.NextButton.Handle, WM_LBUTTONUP,0,0);
end;

procedure ClickCancel();
begin
  PostMessage(WizardForm.CancelButton.Handle, WM_LBUTTONDOWN,0,0);
  PostMessage(WizardForm.CancelButton.Handle, WM_LBUTTONUP,0,0);
end;

#endif

Const
  FILESIZE_QUERY_SERVER = 0;
  FILESIZE_UNKNOWN = -1;
  FILESIZE_KEEP_FORMER = -2;

type
  TDownloadWizardDefine = record
    DownloadingPage: TWizardPage;
    TotalProgress, CurrentProgress: TLabel;
    DownloadFile, DownloadSpeed, DownloadStatus, DownloadElapsedTime, DownloadRemainingTime: TLabel;
    TotalProgressBar, CurrentProgressBar: TNewProgressBar;
    PreviousPageId: Integer;
    BeginTime, AllFilesSize, DownloadedSize: LongInt;
    DownloadError: Boolean;
  end;

  TDownloadItem = record
    Filename: string;
    URL: AnsiString;
    Agent: AnsiString;
    Method: TReadMethod;
    FileSize: LongInt;
    RealSize: LongInt;
    Required: Boolean;
    Downloaded: Boolean;
  end;

  TMirrorItem = record
    Filename: string;
    URL: AnsiString;
    Agent: AnsiString;
    Method: TReadMethod;
    Required: boolean;
  end;

var
  DwinsHs_DownloadWizardDefine: TDownloadWizardDefine;
  DwinsHs_DownloadsList: array of TDownloadItem;
  DwinsHs_MirrorsList: array of TMirrorItem;

function UpdateDownloadingPage(URL, Agent: AnsiString; Method: TReadMethod; Index, TotalSize, ReadSize,
  CurrentSize: LongInt; var ReadStr: AnsiString): Boolean;
var
  ElapsedTime: LongInt;
begin
  if Index >= 0 then
  begin
    DwinsHs_DownloadWizardDefine.DownloadStatus.Caption := ExpandConstant('{cm:DwinsHs_Status_Downloading}');
    DwinsHs_DownloadWizardDefine.DownloadStatus.Repaint;
  end;
  DwinsHs_DownloadWizardDefine.CurrentProgressBar.Position := ReadSize;
  DwinsHs_DownloadWizardDefine.CurrentProgressBar.Repaint;
  DwinsHs_DownloadWizardDefine.DownloadedSize := DwinsHs_DownloadWizardDefine.DownloadedSize + CurrentSize;
  DwinsHs_DownloadWizardDefine.TotalProgressBar.Position := DwinsHs_DownloadWizardDefine.DownloadedSize;
  DwinsHs_DownloadWizardDefine.TotalProgressBar.Repaint;
  if TotalSize > 0 then
  begin
    DwinsHs_DownloadWizardDefine.CurrentProgress.Caption :=
      Format(ExpandConstant('{cm:DwinsHs_ProgressValue}'), [FormatSize(ReadSize), FormatSize(TotalSize),
      ReadSize * 100 / TotalSize]);
    DwinsHs_DownloadWizardDefine.CurrentProgress.Repaint;
  end;
  if DwinsHs_DownloadWizardDefine.AllFilesSize > 0 then
  begin
    DwinsHs_DownloadWizardDefine.TotalProgress.Caption :=
      Format(ExpandConstant('{cm:DwinsHs_ProgressValue}'), [FormatSize(DwinsHs_DownloadWizardDefine.DownloadedSize),
      FormatSize(DwinsHs_DownloadWizardDefine.AllFilesSize), DwinsHs_DownloadWizardDefine.DownloadedSize * 100 /
      DwinsHs_DownloadWizardDefine.AllFilesSize]);
    DwinsHs_DownloadWizardDefine.TotalProgress.Repaint;
  end;
  ElapsedTime :=
    DecodeDateTimeString(GetDateTimeString('yyyy-mm-dd hh:nn:ss', '-', ':')) - DwinsHs_DownloadWizardDefine.BeginTime;
  DwinsHs_DownloadWizardDefine.DownloadElapsedTime.Caption := FormatTime(ElapsedTime);
  DwinsHs_DownloadWizardDefine.DownloadElapsedTime.Repaint;
  if ElapsedTime > 0 then
    DwinsHs_DownloadWizardDefine.DownloadSpeed.Caption :=
      FormatSpeed(DwinsHs_DownloadWizardDefine.DownloadedSize div ElapsedTime)
  else
    DwinsHs_DownloadWizardDefine.DownloadSpeed.Caption := '';
  DwinsHs_DownloadWizardDefine.DownloadSpeed.Repaint;
  if DwinsHs_DownloadWizardDefine.DownloadedSize > 0 then
    DwinsHs_DownloadWizardDefine.DownloadRemainingTime.Caption :=
      FormatTime((DwinsHs_DownloadWizardDefine.AllFilesSize - DwinsHs_DownloadWizardDefine.DownloadedSize) *
      ElapsedTime / DwinsHs_DownloadWizardDefine.DownloadedSize)
  else
    DwinsHs_DownloadWizardDefine.DownloadRemainingTime.Caption := '';
  DwinsHs_DownloadWizardDefine.DownloadRemainingTime.Repaint;
  Result := DwinsHs_CancelDownload = cdNone;
  ReadStr := '';
end;

function GetDownloadStateText(State: Integer): string;
var
  Text: string;
begin
  if State >= 400 then
  begin
    Text := ExpandConstant('{cm:DwinsHs_Status_HTTPxxx}');
    case State of
      400: Text := ExpandConstant('{cm:DwinsHs_Status_HTTP400}');
      401: Text := ExpandConstant('{cm:DwinsHs_Status_HTTP401}');
      404: Text := ExpandConstant('{cm:DwinsHs_Status_HTTP404}');
      407: Text := ExpandConstant('{cm:DwinsHs_Status_HTTP407}');
      500: Text := ExpandConstant('{cm:DwinsHs_Status_HTTP500}');
      502: Text := ExpandConstant('{cm:DwinsHs_Status_HTTP502}');
      503: Text := ExpandConstant('{cm:DwinsHs_Status_HTTP503}');
    end;
    Result := Format(ExpandConstant('{cm:DwinsHs_Status_HTTPError}'), [State, Text]);
  end
  else
  case State of
    READ_OK: Result := ExpandConstant('{cm:DwinsHs_Status_DownlaodComplete}');
    CONNECT_ERROR_NETWORK: Result := ExpandConstant('{cm:DwinsHs_Error_Network}');
    CONNECT_ERROR_OFFLINE: Result := ExpandConstant('{cm:DwinsHs_Error_Offline}');
    CONNECT_ERROR_INITIALIZE: Result := ExpandConstant('{cm:DwinsHs_Error_Initialize}');
    CONNECT_ERROR_OPENSESSION: Result := ExpandConstant('{cm:DwinsHs_Error_OpenSession}');
    CONNECT_ERROR_CREATEREQUEST: Result := ExpandConstant('{cm:DwinsHs_Error_CreateRequest}');
    CONNECT_ERROR_SENDREQUEST: Result := ExpandConstant('{cm:DwinsHs_Error_SendRequest}');
    READ_ERROR_DELETEFILE: Result := ExpandConstant('{cm:DwinsHs_Error_DeleteFile}');
    READ_ERROR_SAVEFILE: Result := ExpandConstant('{cm:DwinsHs_Error_SaveFile}');
    READ_ERROR_CANCELED: Result := ExpandConstant('{cm:DwinsHs_Error_Canceled}');
    READ_ERROR_READDATA: Result := ExpandConstant('{cm:DwinsHs_Error_ReadData}');
  end;
end;

// =====================================================================================================================

procedure DwinsHs_AppendRemoteFile(Filename: string; URL, Agent: AnsiString; Method: TReadMethod; FileSize: LongInt);
var
  i, c: Integer;
  Finded: Boolean;
begin
  URL := FormatURL(URL);
  Filename := Trim(Filename);
  Agent := Trim(Agent);
  Finded := False;
  c := GetArrayLength(DwinsHs_DownloadsList);
  for i := 0 to c - 1 do
  begin
    if CompareText(DwinsHs_DownloadsList[i].Filename, Filename) = 0 then
    begin
      Finded := True;
      DwinsHs_DownloadsList[i].URL := URL;
      DwinsHs_DownloadsList[i].Agent := Agent;
      DwinsHs_DownloadsList[i].Method := Method;
      DwinsHs_DownloadsList[i].Required := True;
      DwinsHs_DownloadsList[i].FileSize := FileSize;
      Break;
    end;
  end;
  if not Finded then
  begin
    SetArrayLength(DwinsHs_DownloadsList, c + 1);
    DwinsHs_DownloadsList[c].Filename := Filename;
    DwinsHs_DownloadsList[c].URL := URL;
    DwinsHs_DownloadsList[c].Agent := Agent;
    DwinsHs_DownloadsList[c].Method := Method;
    DwinsHs_DownloadsList[c].FileSize := FileSize;
    DwinsHs_DownloadsList[c].RealSize := FILESIZE_UNKNOWN;
    DwinsHs_DownloadsList[c].Downloaded := False;
    DwinsHs_DownloadsList[c].Required := True;
  end;
end;

procedure DwinsHs_AppendMirrorFile(Filename: string; URL, Agent: AnsiString; Method: TReadMethod);
var
  i, c: Integer;
  Finded: Boolean;
begin
  URL := FormatURL(URL);
  Filename := Trim(Filename);
  Agent := Trim(Agent);
  Finded := False;
  c := GetArrayLength(DwinsHs_MirrorsList);
  for i := 0 to c - 1 do
  begin
    if (CompareText(DwinsHs_MirrorsList[i].Filename, Filename) = 0) and (DwinsHs_MirrorsList[i].URL = URL) and
       (DwinsHs_MirrorsList[i].Agent = Agent) and (DwinsHs_MirrorsList[i].Method = Method) then
    begin
      Finded := True;
      DwinsHs_MirrorsList[i].Required := True;
      Break;
    end;
  end;
  if not Finded then
  begin
    SetArrayLength(DwinsHs_MirrorsList, c + 1);
    DwinsHs_MirrorsList[c].Filename := Filename;
    DwinsHs_MirrorsList[c].URL := URL;
    DwinsHs_MirrorsList[c].Agent := Agent;
    DwinsHs_MirrorsList[c].Method := Method;
    DwinsHs_MirrorsList[c].Required := True;
  end;
end;

function FindMirror(Filename: string; var URL, Agent: AnsiString; var Method: TReadMethod;
  var MirrorID: Integer): Boolean;
var
  i, c: Integer;
begin
  Result := False;
  c := GetArrayLength(DwinsHs_MirrorsList);
  for i := MirrorID + 1 to c - 1 do
  begin
    if (CompareText(DwinsHs_MirrorsList[i].Filename, Filename) = 0) and DwinsHs_MirrorsList[i].Required then
    begin
      URL := DwinsHs_MirrorsList[i].URL;
      Agent := DwinsHs_MirrorsList[i].Agent;
      Method := DwinsHs_MirrorsList[i].Method;
      MirrorID := i;
      Result := True;
      Break;
    end;
  end;
end;

#ifdef DwinsHs_Use_RemoveRemoteFile
function DwinsHs_RemoveRemoteFile(Filename: string; Delete: Boolean): Boolean;
var
  i: Integer;
begin
  Filename := Trim(Filename);
  Result := False;
  for i := 0 to GetArrayLength(DwinsHs_DownloadsList) - 1 do
  begin
    if (CompareText(DwinsHs_DownloadsList[i].Filename, Filename) = 0) and DwinsHs_DownloadsList[i].Required then
    begin
      Result := True;
      DwinsHs_DownloadsList[i].Required := False;
      if Delete then
      begin
        DwinsHs_DownloadsList[i].RealSize := FILESIZE_UNKNOWN;
        DwinsHs_DownloadsList[i].Downloaded := False;
        if FileExists(Filename) then DeleteFile(Filename);
      end;
      Break;
    end;
  end;
  for i := 0 to GetArrayLength(DwinsHs_MirrorsList) - 1 do
    if CompareText(DwinsHs_MirrorsList[i].Filename, Filename) = 0 then DwinsHs_MirrorsList[i].Required := False;
end;
#endif

#ifdef DwinsHs_Use_ResetRemoteFile
function DwinsHs_ResetRemoteFile(Filename: string; FileSize: Longint): Boolean;
var
  i: Integer;
begin
  Filename := Trim(Filename);
  Result := False;
  for i := 0 to GetArrayLength(DwinsHs_DownloadsList) - 1 do
  begin
    if (CompareText(DwinsHs_DownloadsList[i].Filename, Filename) = 0) and DwinsHs_DownloadsList[i].Required then
    begin
      Result := True;
      if FileSize <> FILESIZE_KEEP_FORMER then DwinsHs_DownloadsList[i].FileSize := FileSize;
      DwinsHs_DownloadsList[i].RealSize := FILESIZE_UNKNOWN;
      DwinsHs_DownloadsList[i].Downloaded := False;
      if FileExists(Filename) then DeleteFile(Filename);
      Break;
    end;
  end;
end;
#endif

#ifdef DwinsHs_Use_RemoteFilesCount
function DwinsHs_RemoteFilesCount(WaitToDownload: Boolean): Integer;
var
  i: Integer;
begin
  Result := 0;
  for i := 0 to GetArrayLength(DwinsHs_DownloadsList) - 1 do
  begin
    if DwinsHs_DownloadsList[i].Required then
    begin
      Result := Result + 1;
      if WaitToDownload and DwinsHs_DownloadsList[i].Downloaded then Result := Result - 1;
    end;
  end;
end;
#endif

#ifdef DwinsHs_Use_RemoveAllRemoteFiles
procedure DwinsHs_RemoveAllRemoteFiles(Delete: Boolean);
var
  i: Integer;
begin
  for i := 0 to GetArrayLength(DwinsHs_DownloadsList) - 1 do
  begin
    DwinsHs_DownloadsList[i].Required := False;
    if Delete and DwinsHs_DownloadsList[i].Required then
    begin
      DwinsHs_DownloadsList[i].RealSize := FILESIZE_UNKNOWN;
      DwinsHs_DownloadsList[i].Downloaded := False;
      if FileExists(DwinsHs_DownloadsList[i].Filename) then DeleteFile(DwinsHs_DownloadsList[i].Filename);
    end;
  end;
  for i := 0 to GetArrayLength(DwinsHs_MirrorsList) - 1 do DwinsHs_MirrorsList[i].Required := False;
end;
#endif

#ifdef DwinsHs_Use_ResetAllRemoteFiles
procedure DwinsHs_ResetAllRemoteFiles();
var
  i: Integer;
begin
  for i := 0 to GetArrayLength(DwinsHs_DownloadsList) - 1 do
  begin
    DwinsHs_DownloadsList[i].RealSize := FILESIZE_UNKNOWN;
    DwinsHs_DownloadsList[i].Downloaded := False;
    if FileExists(DwinsHs_DownloadsList[i].Filename) then DeleteFile(DwinsHs_DownloadsList[i].Filename);
  end;
end;
#endif

#if (Find(FindSection('Files') + 1, 'DwinsHs_Check(', FIND_CONTAINS||FIND_TRIM) > 0)
function DwinsHs_Check(Filename, URL, Agent, Method: string; FileSize: LongInt): Boolean;
var
  MethodValue: TReadMethod;
begin
  Result := True;
  case LowerCase(Trim(Method)) of
    'get': MethodValue := rmGet;
    'post': MethodValue := rmPost;
    'active': MethodValue := rmActive;
    'passive': MethodValue := rmPassive;
  end;
  DwinsHs_AppendRemoteFile(Filename, URL, Agent, MethodValue, FileSize);
end;
#endif

procedure DwinsHs_InitializeWizard(AfterId: Integer);
begin
  DwinsHs_DownloadWizardDefine.PreviousPageId := AfterId;
  DwinsHs_DownloadWizardDefine.DownloadingPage := CreateCustomPage(AfterId,
    ExpandConstant('{cm:DwinsHs_PageCaption}'), ExpandConstant('{cm:DwinsHs_PageDescription}'));
  DwinsHs_DownloadWizardDefine.TotalProgress := CreateLabel(DwinsHs_DownloadWizardDefine.DownloadingPage, '',
    ScaleX(8), ScaleY(16), DwinsHs_DownloadWizardDefine.DownloadingPage.SurfaceWidth - ScaleX(16), False, True);
  CreateLabel(DwinsHs_DownloadWizardDefine.DownloadingPage, ExpandConstant('{cm:DwinsHs_TotalProgress}'),
    ScaleX(8), ScaleY(16), DwinsHs_DownloadWizardDefine.DownloadingPage.SurfaceWidth - ScaleX(16), False, False);
  DwinsHs_DownloadWizardDefine.TotalProgressBar := CreateProgressBar(DwinsHs_DownloadWizardDefine.DownloadingPage,
    ScaleX(8), ScaleY(34), DwinsHs_DownloadWizardDefine.DownloadingPage.SurfaceWidth - ScaleX(16), ScaleY(16), 0, 0,
    0);
  DwinsHs_DownloadWizardDefine.CurrentProgress := CreateLabel(DwinsHs_DownloadWizardDefine.DownloadingPage, '',
    ScaleX(8), ScaleY(60), DwinsHs_DownloadWizardDefine.DownloadingPage.SurfaceWidth - ScaleX(16), False, True);
  CreateLabel(DwinsHs_DownloadWizardDefine.DownloadingPage, ExpandConstant('{cm:DwinsHs_CurrentFile}'), ScaleX(8),
    ScaleY(60), DwinsHs_DownloadWizardDefine.DownloadingPage.SurfaceWidth - ScaleX(16), False, False);
  DwinsHs_DownloadWizardDefine.CurrentProgressBar := CreateProgressBar(DwinsHs_DownloadWizardDefine.DownloadingPage,
    ScaleX(8), ScaleY(78), DwinsHs_DownloadWizardDefine.DownloadingPage.SurfaceWidth - ScaleX(16), ScaleY(16), 0, 0,
    0);
  DwinsHs_DownloadWizardDefine.DownloadFile := CreateLabel(DwinsHs_DownloadWizardDefine.DownloadingPage, '',
    ScaleX(8), ScaleY(114), DwinsHs_DownloadWizardDefine.DownloadingPage.SurfaceWidth - ScaleX(16), False, True);
  CreateLabel(DwinsHs_DownloadWizardDefine.DownloadingPage, ExpandConstant('{cm:DwinsHs_File}'),
    ScaleX(8), ScaleY(114), DwinsHs_DownloadWizardDefine.DownloadingPage.SurfaceWidth - ScaleX(16), False, False);
  DwinsHs_DownloadWizardDefine.DownloadSpeed := CreateLabel(DwinsHs_DownloadWizardDefine.DownloadingPage, '',
    ScaleX(8), ScaleY(134), DwinsHs_DownloadWizardDefine.DownloadingPage.SurfaceWidth - ScaleX(16), False, True);
  CreateLabel(DwinsHs_DownloadWizardDefine.DownloadingPage, ExpandConstant('{cm:DwinsHs_Speed}'), ScaleX(8),
    ScaleY(134), DwinsHs_DownloadWizardDefine.DownloadingPage.SurfaceWidth - ScaleX(16), False, False);
  DwinsHs_DownloadWizardDefine.DownloadStatus := CreateLabel(DwinsHs_DownloadWizardDefine.DownloadingPage, '',
    ScaleX(8), ScaleY(154), DwinsHs_DownloadWizardDefine.DownloadingPage.SurfaceWidth - ScaleX(16), False, True);
  CreateLabel(DwinsHs_DownloadWizardDefine.DownloadingPage, ExpandConstant('{cm:DwinsHs_Status}'),
    ScaleX(8), ScaleY(154), DwinsHs_DownloadWizardDefine.DownloadingPage.SurfaceWidth - ScaleX(16), False, False);
  DwinsHs_DownloadWizardDefine.DownloadElapsedTime := CreateLabel(DwinsHs_DownloadWizardDefine.DownloadingPage, '',
    ScaleX(8), ScaleY(174), DwinsHs_DownloadWizardDefine.DownloadingPage.SurfaceWidth - ScaleX(16), False, True);
  CreateLabel(DwinsHs_DownloadWizardDefine.DownloadingPage, ExpandConstant('{cm:DwinsHs_ElapsedTime}'),
    ScaleX(8), ScaleY(174), DwinsHs_DownloadWizardDefine.DownloadingPage.SurfaceWidth - ScaleX(16), False, False);
  DwinsHs_DownloadWizardDefine.DownloadRemainingTime := CreateLabel(DwinsHs_DownloadWizardDefine.DownloadingPage, '',
    ScaleX(8), ScaleY(194), DwinsHs_DownloadWizardDefine.DownloadingPage.SurfaceWidth - ScaleX(16), False, True);
  CreateLabel(DwinsHs_DownloadWizardDefine.DownloadingPage, ExpandConstant('{cm:DwinsHs_RemainingTime}'),
    ScaleX(8), ScaleY(194), DwinsHs_DownloadWizardDefine.DownloadingPage.SurfaceWidth - ScaleX(16), False, False);
  SetArrayLength(DwinsHs_DownloadsList, 0);
  DwinsHs_Proxy.Mode := pmDefault;
end;

type
  TBeforeDownload = function (): Boolean;
  TAfterDownload = procedure (State: Integer);

var
  DwinsHs_BeforeDownload: TBeforeDownload;
  DwinsHs_AfterDownload: TAfterDownload;

procedure DwinsHs_CurPageChanged(CurPageID: Integer; BeforeDownload: TBeforeDownload; AfterDownload: TAfterDownload);
var
  CurURL, CurAgent: AnsiString;
  CurMethod: TReadMethod;
  CurDownloadedSize: LongInt;
  Response: AnsiString;
  CurrentSize, Size: LongInt;
  i, State, MirrorID: Integer;
begin
  DwinsHs_BeforeDownload := BeforeDownload;
  DwinsHs_AfterDownload := AfterDownload;
  if (CurPageId = DwinsHs_DownloadWizardDefine.PreviousPageId) and (CurPageId = wpPreparing) then
  begin
    for i := 0 to GetArrayLength(DwinsHs_DownloadsList) - 1 do DwinsHs_DownloadsList[i].Required := False;
  end
  else if CurPageId = DwinsHs_DownloadWizardDefine.DownloadingPage.Id then
  begin
    WizardForm.NextButton.Enabled := False;
    WizardForm.NextButton.Caption := ExpandConstant('{cm:DwinsHs_Status_ButtonNext}');
    WizardForm.NextButton.Repaint;
    if BeforeDownload <> nil then
    begin
      if not BeforeDownload() then Exit;
    end;
    DwinsHs_DownloadWizardDefine.TotalProgress.Caption := '';
    DwinsHs_DownloadWizardDefine.TotalProgress.Repaint;
    DwinsHs_DownloadWizardDefine.TotalProgressBar.Position := 0;
    DwinsHs_DownloadWizardDefine.TotalProgressBar.Style := npbstMarquee;
    DwinsHs_DownloadWizardDefine.TotalProgressBar.Repaint;
    DwinsHs_DownloadWizardDefine.CurrentProgress.Caption := '';
    DwinsHs_DownloadWizardDefine.CurrentProgress.Repaint;
    DwinsHs_DownloadWizardDefine.CurrentProgressBar.Position := 0;
    DwinsHs_DownloadWizardDefine.CurrentProgressBar.Style := npbstMarquee;
    DwinsHs_DownloadWizardDefine.CurrentProgressBar.Repaint;
    DwinsHs_DownloadWizardDefine.DownloadSpeed.Caption := '';
    DwinsHs_DownloadWizardDefine.DownloadSpeed.Repaint;
    DwinsHs_DownloadWizardDefine.DownloadStatus.Caption := ExpandConstant('{cm:DwinsHs_Status_GetFileInformation}');
    DwinsHs_DownloadWizardDefine.DownloadStatus.Font.Color := DwinsHs_DownloadWizardDefine.DownloadFile.Font.Color;
    DwinsHs_DownloadWizardDefine.DownloadStatus.Repaint;
    DwinsHs_DownloadWizardDefine.DownloadElapsedTime.Caption := FormatTime(0);
    DwinsHs_DownloadWizardDefine.DownloadElapsedTime.Repaint;
    DwinsHs_DownloadWizardDefine.DownloadRemainingTime.Caption := '';
    DwinsHs_DownloadWizardDefine.DownloadRemainingTime.Repaint;
    DwinsHs_ProcessMessages();
    DwinsHs_DownloadWizardDefine.DownloadError := False;
    DwinsHs_DownloadWizardDefine.AllFilesSize := 0;
    DwinsHs_DownloadWizardDefine.DownloadedSize := 0;
    DwinsHs_CancelDownload := cdNone;
    State := 0;
    DwinsHs_DownloadWizardDefine.BeginTime := DecodeDateTimeString(GetDateTimeString('yyyy-mm-dd hh:nn:ss', '-', ':'));
    for i := 0 to GetArrayLength(DwinsHs_DownloadsList) - 1 do
    begin
      if DwinsHs_DownloadsList[i].Required then
      begin
        DwinsHs_DownloadWizardDefine.DownloadFile.Caption := ExtractFileName(DwinsHs_DownloadsList[i].Filename);
        DwinsHs_DownloadWizardDefine.DownloadFile.Repaint;
        DwinsHs_DownloadsList[i].RealSize := DwinsHs_DownloadsList[i].FileSize;
        if DwinsHs_DownloadsList[i].FileSize <= 0 then
        begin
          CurURL := DwinsHs_DownloadsList[i].URL;
          CurAgent := DwinsHs_DownloadsList[i].Agent;
          CurMethod := DwinsHs_DownloadsList[i].Method;
          MirrorID := -1;
          repeat
            CurrentSize := DwinsHs_GetRemoteSize(CurURL, CurAgent, CurMethod);
            if CurrentSize >= 0 then Break;
            if not FindMirror(DwinsHs_DownloadsList[i].Filename, CurURL, CurAgent, CurMethod, MirrorId) then Break;
          until false;
          if CurrentSize >= 0 then DwinsHs_DownloadsList[i].RealSize := CurrentSize;
        end;
        DwinsHs_ProcessMessages();
        if DwinsHs_DownloadsList[i].RealSize > 0 then
          DwinsHs_DownloadWizardDefine.AllFilesSize :=
            DwinsHs_DownloadWizardDefine.AllFilesSize + DwinsHs_DownloadsList[i].RealSize;
      end;
    end;
    DwinsHs_DownloadWizardDefine.TotalProgressBar.Max := DwinsHs_DownloadWizardDefine.AllFilesSize;
    DwinsHs_DownloadWizardDefine.TotalProgressBar.Style := npbstNormal;
    DwinsHs_DownloadWizardDefine.CurrentProgressBar.Style := npbstNormal;
    DwinsHs_DownloadWizardDefine.DownloadStatus.Font.Color := DwinsHs_DownloadWizardDefine.DownloadFile.Font.Color;
    DwinsHs_DownloadWizardDefine.TotalProgressBar.State := npbsNormal;
    DwinsHs_DownloadWizardDefine.CurrentProgressBar.State := npbsNormal;
    DwinsHs_DownloadWizardDefine.TotalProgressBar.Repaint;
    DwinsHs_DownloadWizardDefine.CurrentProgressBar.Repaint;
    DwinsHs_DownloadWizardDefine.DownloadStatus.Repaint;
    DwinsHs_ProcessMessages();
    for i := 0 to GetArrayLength(DwinsHs_DownloadsList) - 1 do
    begin
      DwinsHs_ProcessMessages();
      if DwinsHs_DownloadsList[i].Downloaded then
      begin
        DwinsHs_DownloadWizardDefine.DownloadedSize :=
          DwinsHs_DownloadWizardDefine.DownloadedSize + DwinsHs_DownloadsList[i].RealSize;
        DwinsHs_DownloadWizardDefine.DownloadFile.Caption := ExtractFileName(DwinsHs_DownloadsList[i].Filename);
        DwinsHs_DownloadWizardDefine.DownloadFile.Repaint;
        DwinsHs_DownloadWizardDefine.DownloadStatus.Caption := ExpandConstant('{cm:DwinsHs_Status_DownlaodComplete}');
        DwinsHs_DownloadWizardDefine.DownloadStatus.Repaint;
        UpdateDownloadingPage(DwinsHs_DownloadsList[i].URL, DwinsHs_DownloadsList[i].Agent,
          DwinsHs_DownloadsList[i].Method, -1, DwinsHs_DownloadsList[i].RealSize, DwinsHs_DownloadsList[i].RealSize, 0,
          Response);
        if DwinsHs_CancelDownload <> cdNone then Break;
      end
      else
      begin
        DwinsHs_DownloadWizardDefine.DownloadFile.Caption := ExtractFileName(DwinsHs_DownloadsList[i].Filename);
        DwinsHs_DownloadWizardDefine.DownloadFile.Repaint;
        CurDownloadedSize := DwinsHs_DownloadWizardDefine.DownloadedSize;
        CurURL := DwinsHs_DownloadsList[i].URL;
        CurAgent := DwinsHs_DownloadsList[i].Agent;
        CurMethod := DwinsHs_DownloadsList[i].Method;
        MirrorID := -1;
        repeat
          DwinsHs_DownloadWizardDefine.DownloadedSize := CurDownloadedSize;
          DwinsHs_DownloadWizardDefine.DownloadStatus.Caption := ExpandConstant('{cm:DwinsHs_Status_StartingDownload}');
          DwinsHs_DownloadWizardDefine.DownloadStatus.Repaint;
          UpdateDownloadingPage(CurURL, CurAgent, CurMethod, -1, DwinsHs_DownloadsList[i].RealSize, 0, 0, Response);
          DwinsHs_DownloadWizardDefine.CurrentProgressBar.Max := DwinsHs_DownloadsList[i].RealSize;
          State := DwinsHs_ReadRemoteURL(CurURL, CurAgent, CurMethod, Response, Size, DwinsHs_DownloadsList[i].Filename,
            @UpdateDownloadingPage);
          DwinsHs_DownloadWizardDefine.DownloadStatus.Caption := GetDownloadStateText(State);
          DwinsHs_DownloadWizardDefine.DownloadStatus.Repaint;
          if DwinsHs_CancelDownload <> cdNone then Break;
          if State = READ_OK then Break;
          if not FindMirror(DwinsHs_DownloadsList[i].Filename, CurURL, CurAgent, CurMethod, MirrorID) then Break;
        until false;
        if DwinsHs_CancelDownload <> cdNone then Break;
        if State <> READ_OK then Break;
        DwinsHs_DownloadsList[i].Downloaded := True;
      end;
      WizardForm.Repaint;
    end;
    DwinsHs_DownloadWizardDefine.TotalProgressBar.Repaint;
    DwinsHs_DownloadWizardDefine.CurrentProgressBar.Repaint;
    case DwinsHs_CancelDownload of
      cdNone:
      begin
        WizardForm.NextButton.Enabled := True;
        if State <> READ_OK then
        begin
          DwinsHs_DownloadWizardDefine.TotalProgressBar.State := npbsError;
          DwinsHs_DownloadWizardDefine.DownloadStatus.Font.Color := clRed;
          DwinsHs_DownloadWizardDefine.CurrentProgressBar.State := npbsError;
          WizardForm.NextButton.Caption := ExpandConstant('{cm:DwinsHs_Status_ButtonRetry}');
          WizardForm.NextButton.Repaint;
          DwinsHs_DownloadWizardDefine.DownloadError := True;
          if AfterDownload <> nil then AfterDownload(State);
        end
        else
        begin
          DwinsHs_DownloadWizardDefine.DownloadStatus.Font.Color := clGreen;
          if AfterDownload <> nil then AfterDownload(State);
          #ifdef DwinsHs_Auto_Continue
          ClickNext();
          #endif
        end;
      end;
      cdBack: ;
      cdExit: ClickCancel();
    end;
  end;
end;

procedure DwinsHs_ShouldSkipPage(CurPageID: Integer; var ChangeResult: Boolean);
var
  i: Integer;
begin
  if CurPageID = DwinsHs_DownloadWizardDefine.DownloadingPage.Id then
  begin
    ChangeResult := True;
    for i := 0 to GetArrayLength(DwinsHs_DownloadsList) - 1 do
    begin
      if DwinsHs_DownloadsList[i].Required and (not DwinsHs_DownloadsList[i].Downloaded) then
      begin
        ChangeResult := False;
        Break;
      end;
    end;
  end;
end;

procedure DwinsHs_BackButtonClick(CurPageID: Integer);
begin
  if CurPageID = DwinsHs_DownloadWizardDefine.DownloadingPage.Id then DwinsHs_CancelDownload := cdBack;
end;

procedure DwinsHs_NextButtonClick(CurPageID: Integer; var ChangeResult: Boolean);
begin
  if (CurPageID = DwinsHs_DownloadWizardDefine.DownloadingPage.Id) and DwinsHs_DownloadWizardDefine.DownloadError then
  begin
    ChangeResult := False;
    DwinsHs_CurPageChanged(CurPageID, DwinsHs_BeforeDownload, DwinsHs_AfterDownload);
  end;
end;

procedure DwinsHs_CancelButtonClick(CurPageID: Integer; var Cancel, Confirm: Boolean);
begin
  if (CurPageId = DwinsHs_DownloadWizardDefine.DownloadingPage.Id) and (DwinsHs_CancelDownload = cdExit) then
    Confirm := False;
end;

#endif

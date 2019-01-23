/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/
#include "externs.h"
#include "devFlarm.h"
#include "dlgTools.h"
#include "Dialogs.h"
#include "WindowControls.h"
#include "resource.h"
#include "dlgIGCProgress.h"
#include "Util/Clamp.hpp"
#include "OS/Sleep.h"
#include "dlgFlarmIGCDownload.h"






static WndForm *wf = NULL;




PDeviceDescriptor_t CDevFlarm::m_pDevice=NULL;
BOOL CDevFlarm::m_bCompassCalOn=FALSE;
Mutex* CDevFlarm::m_pCritSec_DeviceData=NULL;
double CDevFlarm::m_abs_press=0.0;
double CDevFlarm::m_delta_press=0.0;

TCHAR CDevFlarm::m_szVersion[15]={0};




bool CDevFlarm::Register(){
	return devRegister(GetName(), cap_baro_alt|cap_vario, &Install);
}

BOOL CDevFlarm::Install( PDeviceDescriptor_t d ) {
  StartupStore(_T("Flarm Drvier Install %s"), NEWLINE);
	_tcscpy(d->Name, GetName());
	d->ParseNMEA = NULL ; // ParseNMEA;
	d->PutMacCready = NULL;
	d->PutBugs = NULL;
	d->PutBallast = NULL;
	d->Open = Open;
	d->Close = Close;
	d->Init = NULL;
	d->LinkTimeout = NULL;
	d->Declare = NULL;
	d->IsGPSSource = GetFalse;
	d->IsBaroSource = GetTrue;
	d->Config = Config;
	d->ParseStream  = FlarmParseString;

	return(TRUE);
}


#define REC_BUFFERSIZE 512
uint8_t RingBuff[REC_BUFFERSIZE+1];
volatile  uint16_t InCnt=0;
volatile  uint16_t OutCnt=0;
static bool recEnable = true;

BOOL CDevFlarm::FlarmParseString(DeviceDescriptor_t *d, char *String, int len, NMEA_INFO *GPS_INFO)
{
if(d == NULL) return 0;
if(String == NULL) return 0;
if(len == 0) return 0;

int cnt=0;

if(recEnable)
  while (cnt < len)
  {
    RingBuff[InCnt++] = (TCHAR) String[cnt++];
    InCnt %= REC_BUFFERSIZE;
  } //  (cnt < len)

return  true;
}


uint8_t RecChar( DeviceDescriptor_t *d, uint8_t *inchar, uint16_t Timeout)
{

  uint16_t TimeCnt =0;
  uint8_t Tmp;
  while(OutCnt == InCnt)
  {
    Poco::Thread::sleep(1);
    Poco::Thread::Thread::yield();
    if(TimeCnt++ > Timeout)
    {
      {StartupStore(TEXT("REC_TIMEOUT_ERROR" ));}
      return REC_TIMEOUT_ERROR;
    }
  }
  Tmp = RingBuff[OutCnt++];
  OutCnt %= REC_BUFFERSIZE;
  if(inchar)
    *inchar = Tmp;

  return REC_NO_ERROR;
}



BOOL CDevFlarm::Open( PDeviceDescriptor_t d) {
	m_pDevice = d;

	m_pCritSec_DeviceData = new Mutex();

	return TRUE;
}

BOOL CDevFlarm::Close (PDeviceDescriptor_t d) {
	m_pDevice = NULL;

	delete m_pCritSec_DeviceData;
	m_pCritSec_DeviceData = NULL;

	return TRUE;
}


inline double int16toDouble(int v) {
	return (double)(int16_t)v;
};

inline double int24toDouble(int v) {
	if(v > (1<<23)){
		v = -(v - (1<<24)+1);
	}
	return v;
};

void CDevFlarm::LockDeviceData(){
	if(m_pCritSec_DeviceData) {
		m_pCritSec_DeviceData->Lock();
	}
}


void CDevFlarm::UnlockDeviceData(){
	if(m_pCritSec_DeviceData) {
		m_pCritSec_DeviceData->Unlock();
	}
}

BOOL CDevFlarm::ParseNMEA( DeviceDescriptor_t *d, TCHAR *String, NMEA_INFO *pINFO ) {

	return FALSE;
}










BOOL CDevFlarm::GetDeviceName( PDeviceDescriptor_t d ){
  /*  if (d && d->Com) {
    	d->Com->WriteString(TEXT("$PCPILOT,C,GETNAME\r\n"));
    }*/
	return TRUE;
}



BOOL CDevFlarm::GetFirmwareVersion(PDeviceDescriptor_t d) {
/*    if (d && d->Com) {
        d->Com->WriteString(TEXT("$PCPILOT,C,GETFW\r\n"));
    }*/
	return TRUE;
}






CallBackTableEntry_t CDevFlarm::CallBackTable[]={
  EndCallBackEntry()
};

BOOL CDevFlarm::Config(PDeviceDescriptor_t d){
        if(m_pDevice != d) {
                StartupStore(_T("Flarm Config : Invalide device descriptor%s"), NEWLINE);
                return FALSE;
        }

        WndForm* wf = dlgLoadFromXML(CallBackTable, IDR_XML_DEVFLARM);
        if(wf) {

        WndButton *wBt = NULL;

        wBt = (WndButton *)wf->FindByName(TEXT("cmdClose"));
        if(wBt){
                wBt->SetOnClickNotify(OnCloseClicked);
        }


        wBt = (WndButton *)wf->FindByName(TEXT("cmdIGCDownload"));
        if(wBt){
                wBt->SetOnClickNotify(OnIGCDownloadClicked);
        }

        wBt = (WndButton *)wf->FindByName(TEXT("cmdFlarmReboot"));
        if(wBt){
                wBt->SetOnClickNotify(OnRebootClicked);
        }
/*
        wBt = (WndButton *)wf->FindByName(TEXT("prpFlarmId"));
        if(wBt){
                wBt->SetOnClickNotify(OnFlarmIdClicked);
        }*/

        WndProperty* wp;
        wp = (WndProperty*)wf->FindByName(TEXT("prpFlarmId"));
        if (wp) {
        	wp->GetDataField()->SetAsString(_T("DD222"));

          wp->RefreshDisplay();
        }




        GetFirmwareVersion(m_pDevice);

//              wf->SetTimerNotify(1000, OnTimer);
                wf->ShowModal();

                delete wf;
                wf=NULL;
        }
        return TRUE;
}


bool CDevFlarm::OnTimer(WndForm* pWnd){
  Update(pWnd);
  return true;
}

void CDevFlarm::OnCloseClicked(WndButton* pWnd){
  if(pWnd) {
    WndForm * pForm = pWnd->GetParentWndForm();
    if(pForm) {
      pForm->SetModalResult(mrOK);
    }
  }
}



void CDevFlarm::OnIGCDownloadClicked(WndButton* pWnd) {
	(void)pWnd;
	if(m_pDevice) {
	dlgIGCSelectListShowModal(m_pDevice);
	}
}


void CDevFlarm::OnRebootClicked(WndButton* pWnd) {
        (void)pWnd;
        StartupStore(TEXT("OnRebootClicked"));
        if(m_pDevice) {
            FlarmReboot(m_pDevice);
        }
}

BOOL CDevFlarm::FlarmReboot(PDeviceDescriptor_t d) {
    if (d && d->Com) {
        d->Com->WriteString(TEXT("$PFLAR,0*55\r\n"));
        StartupStore(TEXT("$PFLAR,0*55\r\n"));
    }
    return TRUE;
}

void CDevFlarm::Update(WndForm* pWnd) {
	TCHAR Temp[50] = {0};

	LockFlightData();

	UnlockFlightData();

	LockDeviceData();
	_stprintf(Temp, TEXT("Flarm - Version: %s"), m_szVersion);
	UnlockDeviceData();

	pWnd->SetCaption(Temp);

	TCHAR Tmp[255];
	        WndProperty* wp;
	        wp = (WndProperty*)wf->FindByName(TEXT("prpFlarmId"));
	        if (wp) {

	            StartupStore(TEXT("DD1234\r\n"));
	            _tcsncpy(Tmp, wp->GetDataField()->GetAsString(),190);
	            StartupStore(Tmp);
	        }

}

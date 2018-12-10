/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/

#ifndef devFlarm_h__
#define devFlarm_h__
#include "devBase.h"
#include "Util/tstring.hpp"
#include "nmeaistream.h"
#include "dlgTools.h"

class WindowControl;
class WndButton;


typedef union{
  uint16_t val;
  uint8_t byte[2];
} ConvUnion;

#define STARTFRAME    0x73
#define ESCAPE        0x78
#define ESC_ESC       0x55
#define ESC_START     0x31

#define EOF_          0x1A
#define ACK           0xA0
#define NACK          0xB7
#define PING          0x01
#define SETBAUDRATE   0x02
#define FLASHUPLAOD   0x10
#define FLASHDOWNLOAD 0x11
#define EXIT          0x12
#define SELECTRECORD  0x20
#define GETRECORDINFO 0x21
#define GETIGCDATA    0x22

class CDevFlarm : public DevBase
{
private:
	// no ctor
	CDevFlarm() {}

	// no copy
	CDevFlarm(const CDevFlarm&) {}
	void operator=(const CDevFlarm&) {}

//Init
public:
	static bool Register();
	static const TCHAR* GetName() { return TEXT("Flarm"); }
	static BOOL Open(PDeviceDescriptor_t d);
	static BOOL Close (PDeviceDescriptor_t d);


private:
	static BOOL Install(PDeviceDescriptor_t d);

// Receive data
private:
	static BOOL ParseNMEA(DeviceDescriptor_t *d, TCHAR *String, NMEA_INFO *pINFO);



// Send Command
	static BOOL GetDeviceName( PDeviceDescriptor_t d );
	static BOOL SetDeviceName( PDeviceDescriptor_t d, const tstring& strName );
	static BOOL GetFirmwareVersion( PDeviceDescriptor_t d );

	static BOOL SetZeroDeltaPressure( PDeviceDescriptor_t d );


	static BOOL FlarmParseString(DeviceDescriptor_t *d, char *String, int len, NMEA_INFO *GPS_INFO);
	static BOOL m_bCompassCalOn;

	static double m_abs_press;
	static double m_delta_press;

	static TCHAR m_szVersion[15];

	static Mutex* m_pCritSec_DeviceData;

	static void LockDeviceData();
	static void UnlockDeviceData();

// Config
	static BOOL Config(PDeviceDescriptor_t d);
	static void OnCloseClicked(WndButton* pWnd);


	static void OnIGCDownloadClicked(WndButton* pWnd);

	static bool OnTimer(WndForm* pWnd);

	static void Update(WndForm* pWnd);

	static CallBackTableEntry_t CallBackTable[];
	static PDeviceDescriptor_t m_pDevice;
};

typedef struct
{
  ConvUnion blocksize;
  uint8_t   Version;
  ConvUnion Sequence;
  uint8_t   Command;
  ConvUnion CRC_in;
  /**************************/
  uint8_t   Error;
  volatile BOOL      RecReady;
  uint16_t  PL_Idx;
  uint16_t  BlkIndex;
  /**************************/
  uint8_t   Payload[2000];

} BinBlock;




#define NO_BLK_ERROR  0
#define CRC_BLKERROR 1
#endif // devCProbe_h__

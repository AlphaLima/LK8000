/*
 * LK8000 Tactical Flight Computer -  WWW.LK8000.IT
 * Released under GNU/GPL License v.2
 * See CREDITS.TXT file for authors and copyrights
 * 
 * File:   FilePort.cpp
 * Author: Bruno de Lacheisserie
 * 
 * Created on February 22, 2016, 8:29 PM
 */
#include "externs.h"
#include "FilePort.h"
#include <functional>
#include "utils/zzip_stream.h"

class FilePort
{

#ifdef WIN32
    WSADATA wsd;
    WSAStartup(MAKEWORD(1, 1), &wsd);
#endif
}

FilePort::~FilePort() {

#ifdef WIN32
    WSACleanup();
#endif
}

bool FilePort::Initialize() {

    if (!Connect()) {
        goto failed;
    }
    FileStream = _tfopen(Replay_FileName[GetPortIndex()], TEXT("rt"));

    if(FileStream)
    	goto failed;
    
    if(!ComPort::Initialize()) {
        // no need to log failed of StartRxThread it's already made in ComPort::Initialize();
        goto failed;
    }

    StartupStore(_T(". ComPort %u Init <%s> end OK%s"), (unsigned)GetPortIndex() + 1, GetPortName(), NEWLINE);
    return true;
failed:
    StatusMessage(mbOk, NULL, TEXT("%s %s"), MsgToken(762), GetPortName());


    return false;
}

int FilePort::SetRxTimeout(int TimeOut) {

    unsigned dwTimeout = mTimeout;
    mTimeout = TimeOut;
    return dwTimeout;
}

size_t FilePort::Read(void *szString, size_t size) {

    if(!FileStream) {
        return false; // socket not connect,that can happen with TCPServer Port.
    }


    if (fgets((char*) szString, 200, FileStream)==NULL) {
   // 	szString[0] = (char*)'\0';
      return false;
    }

    size = strlen((char*)szString);

    

    return 0U;
}

bool FilePort::Close() {
    ComPort::Close();
    fclose(FileStream);
    FileStream = NULL;
    StartupStore(_T(". ComPort %u closed Ok.%s"), (unsigned)GetPortIndex() + 1, NEWLINE); // 100210 BUGFIX missing
    return true;
}

bool FilePort::Write(const void *data, size_t length) {


    return true;
}

unsigned FilePort::RxThread() {
    unsigned dwWaitTime = 0;
    _Buff_t szString;
    Purge();
    
        


    while (!StopEvt.tryWait(dwWaitTime)) {

        UpdateStatus();

        int nRecv = ReadData(szString);
        if (nRecv > 0) {
            std::for_each(std::begin(szString), std::begin(szString) + nRecv, std::bind(&FilePort::ProcessChar, this, _1));
            dwWaitTime = 5; // avoid cpu overhead;
        } else {
            dwWaitTime = 50; // if no more data wait 50ms ( max data rate 20Hz )
        }
    }

    return 0UL;
}

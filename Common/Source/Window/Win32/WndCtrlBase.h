/*
 * LK8000 Tactical Flight Computer -  WWW.LK8000.IT
 * Released under GNU/GPL License v.2
 * See CREDITS.TXT file for authors and copyrights
 *
 * File:   WndCtrlBase.h
 * Author: Bruno de Lacheisserie
 *
 * Created on 2 décembre 2014
 */

#ifndef WNDCTRLBASE_H
#define	WNDCTRLBASE_H
#include "WndPaint.h"

class WndCtrlBase :public WndPaint {
public:
    WndCtrlBase(const TCHAR* szName);
    virtual ~WndCtrlBase();
    
    const TCHAR* GetWndName() const {
        return _szWindowName.c_str();
    }
    
protected:    
    std::tstring _szWindowName;
};

#endif	/* WNDCTRLBASE_H */


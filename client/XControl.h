#pragma once
#include <X11/Xlib.h>

class XControl
{
public:
    XControl(Display * _pkDisplay);
    void MoveCursor(int _nX, int _nY);
    void TriggerMousePress(int _nButton);
    void TriggerMouseRelease(int _nButton);
    void TriggerClick(int _nButton);
    void TriggerKeyPress(int _nKey);
    void TriggerKeyRelease(int _nKey);
protected:
    void SendFakeMouseEvent(unsigned int _nButton, bool _bPress, unsigned long _nDelay);
    void SendFakeKeyEvent(unsigned long _nKeySym, bool _bPress, unsigned long _nDelay);
    Display *m_pkDisplay;
};

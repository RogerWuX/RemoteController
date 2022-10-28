#include "XControl.h"

#include <X11/extensions/XTest.h>

#include "Logger.h"

XControl::XControl(Display * _pkDisplay)
    :m_pkDisplay(_pkDisplay)
{
    int nEventBase, nErrorBase;
    int nMajorVersion;
    int nMinorVersion;
    if(!XTestQueryExtension(m_pkDisplay, &nEventBase, &nErrorBase, &nMajorVersion, &nMinorVersion)){
        MAIN_LOGGER->warn("XTest extension is not supported.");
    }
}

void XControl::MoveCursor(int _nX, int _nY)
{
    Window nRootWindow = DefaultRootWindow(m_pkDisplay);
    XWarpPointer(m_pkDisplay, None, nRootWindow, 0, 0, 0, 0, _nX, _nY);
    XFlush(m_pkDisplay);
}

void XControl::TriggerMousePress(int _nButton)
{
    SendFakeMouseEvent(_nButton, true, CurrentTime);
}

void XControl::TriggerMouseRelease(int _nButton)
{
    SendFakeMouseEvent(_nButton, false, CurrentTime);
}

void XControl::TriggerClick(int _nButton)
{
    SendFakeMouseEvent(_nButton, true, CurrentTime);
    SendFakeMouseEvent(_nButton, false, 1);
}

void XControl::TriggerKeyPress(int _nKey)
{
    SendFakeKeyEvent(_nKey, true, CurrentTime);
}

void XControl::TriggerKeyRelease(int _nKey)
{
    SendFakeKeyEvent(_nKey, false, CurrentTime);
}

void XControl::SendFakeMouseEvent(unsigned int _nButton, bool _bPress, unsigned long _nDelay)
{
    XTestFakeButtonEvent(m_pkDisplay, _nButton, _bPress, _nDelay);
    XFlush(m_pkDisplay);
}

void XControl::SendFakeKeyEvent(unsigned long _nKeySym, bool _bPress, unsigned long _nDelay)
{
    XTestFakeKeyEvent(m_pkDisplay, XKeysymToKeycode(m_pkDisplay, _nKeySym), _bPress, _nDelay);
    XFlush(m_pkDisplay);
}

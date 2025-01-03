#ifndef BASEUTILS_H
#define BASEUTILS_H

#ifdef _WIN32
# include <Windows.h>
#else

#endif


namespace Utils
{
#ifdef _WIN32
    enum WinVer : BYTE {
        Undef, WinXP, WinVista, Win7, Win8, Win8_1, Win10, Win11
    };
    WinVer getWinVersion();
    // bool isColorDark(COLORREF color);
    COLORREF getColorizationColor(bool isActive = true, COLORREF topColor = 0x00ffffff);
#else

#endif
};

#endif // BASEUTILS_H

#include "stdafx.h"

#include <iostream>


#include <stdlib.h>
#include <stdio.h>
#include <windows.h>
#include <Wingdi.h>


using namespace std;

void MyDrawThread()
{
	HDC hdc;
	SIZE s;
	s.cx = ::GetSystemMetrics(SM_CXSCREEN);
	s.cy = ::GetSystemMetrics(SM_CYSCREEN);
	int x, y, z, r, g, b;
	int horizontal = 0;
	int vertical = 0;
	RECT desktop;
	// Get a handle to the desktop window
	const HWND hDesktop = GetDesktopWindow();
	// Get the size of screen to the variable desktop
	GetWindowRect(hDesktop, &desktop);
	// The top left corner will have coordinates (0,0)
	// and the bottom right corner will have coordinates
	// (horizontal, vertical)
	horizontal = desktop.right;
	vertical = desktop.bottom;
	HBRUSH hbr, hbrOld;
	x = horizontal / 4; //s.cx * rand() / RAND_MAX;  // position x
	y = vertical / 4; //s.cy * rand() / RAND_MAX;  // position y
	z = 30; // radius
	r = 255; // red color componennt
	g = 200;// green color component
	b = 0;// blue color component
	hbr = ::CreateSolidBrush(RGB(r,g,b));
	hdc = ::GetDCEx(NULL, 0, 0);
	hbrOld = (HBRUSH) ::SelectObject(hdc, hbr);
	::Ellipse(hdc, x - z, y - z, x + z, y + z);
	::SelectObject(hdc, hbrOld);
	::DeleteObject(hbr);
	::ReleaseDC(NULL, hdc);
	::Sleep(20);
}
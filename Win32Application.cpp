//*********************************************************
//
// Copyright (c) Microsoft. All rights reserved.
// This code is licensed under the MIT License (MIT).
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
//
//*********************************************************

#include "stdafx.h"
#include "Win32Application.h"

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#define DBOUT( s )            

HWND Win32Application::m_hwnd = nullptr;
bool isRecordingFrameTimes = false;
std::vector<frameData> frameTimes;

char frameTimesFilePath[256] = { 'f', 'r' , 'a' , 'm' , 'e' , 'T', 'i', 'm', 'e', 's', '.', 'c', 's', 'v' };
unsigned long long frameCount;

//prototyping
void writePerformanceLog(std::string filepath, std::vector<frameData>* frameTimes);
void clearFrameTimes(std::vector<frameData>& frameTimes);


int Win32Application::Run(DXSample* pSample, HINSTANCE hInstance, int nCmdShow)
{
	// Parse the command line parameters
	int argc;
	LPWSTR* argv = CommandLineToArgvW(GetCommandLineW(), &argc);
	pSample->ParseCommandLineArgs(argv, argc);
	LocalFree(argv);

	// Initialize the window class.
	WNDCLASSEX windowClass = { 0 };
	windowClass.cbSize = sizeof(WNDCLASSEX);
	windowClass.style = CS_HREDRAW | CS_VREDRAW;
	windowClass.lpfnWndProc = WindowProc;
	windowClass.hInstance = hInstance;
	windowClass.hCursor = LoadCursor(NULL, IDC_ARROW);
	windowClass.lpszClassName = L"DXSampleClass";
	RegisterClassEx(&windowClass);

	RECT windowRect = { 0, 0, static_cast<LONG>(pSample->GetWidth()), static_cast<LONG>(pSample->GetHeight()) };
	AdjustWindowRect(&windowRect, WS_OVERLAPPEDWINDOW, FALSE);

	// Create the window and store a handle to it.
	m_hwnd = CreateWindow(
		windowClass.lpszClassName,
		pSample->GetTitle(),
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		windowRect.right - windowRect.left,
		windowRect.bottom - windowRect.top,
		nullptr,		// We have no parent window.
		nullptr,		// We aren't using menus.
		hInstance,
		pSample);

	// Initialize the sample. OnInit is defined in each child-implementation of DXSample.
	pSample->OnInit();

	ShowWindow(m_hwnd, nCmdShow);

	// Main sample loop.
	MSG msg = {};
	while (msg.message != WM_QUIT)
	{
		// Process any messages in the queue.
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	pSample->OnDestroy();

	// Return this part of the WM_QUIT message to Windows.
	return static_cast<char>(msg.wParam);
}

// Main message handler for the sample.
LRESULT CALLBACK Win32Application::WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	DXSample* pSample = reinterpret_cast<DXSample*>(GetWindowLongPtr(hWnd, GWLP_USERDATA));

	switch (message)
	{
	case WM_CREATE: {
		// Save the DXSample* passed in to CreateWindow.
		LPCREATESTRUCT pCreateStruct = reinterpret_cast<LPCREATESTRUCT>(lParam);
		SetWindowLongPtr(hWnd, GWLP_USERDATA,
			reinterpret_cast<LONG_PTR>(pCreateStruct->lpCreateParams));
	}
				  return 0;

	case WM_KEYDOWN:
		if (pSample) {
			pSample->OnKeyDown(static_cast<UINT8>(wParam));
		}
		if (static_cast<UINT8>(wParam) == VK_ESCAPE)
			PostQuitMessage(0);
		if (static_cast<UINT8>(wParam) == VK_RETURN)
		{
			DBOUT("Not recording ");
			isRecordingFrameTimes = !isRecordingFrameTimes;
		}
		if (static_cast<UINT8>(wParam) == 0x57)
			writePerformanceLog(frameTimesFilePath, &frameTimes);
		if (static_cast<UINT8>(wParam) == 0x43)
			clearFrameTimes(frameTimes);

		return 0;

	case WM_KEYUP:
		if (pSample) {
			pSample->OnKeyUp(static_cast<UINT8>(wParam));
		}
		return 0;

	case WM_PAINT:
		if (pSample) {
			std::chrono::steady_clock::time_point start(std::chrono::steady_clock::now());
			pSample->OnUpdate();
			pSample->OnRender();
			std::chrono::steady_clock::time_point end(std::chrono::steady_clock::now());
			float frameTime = std::chrono::duration_cast<std::chrono::duration<double>>(end - start).count();

			frameData frame;
			frame.frameTime = frameTime * 1000;
			frame.frameNumber = frameCount;

			if (isRecordingFrameTimes)
				frameTimes.push_back(frame);
		}
		return 0;

	case WM_DESTROY:
	{
		std::string str1 = "C : \\Users\Hakaru\Desktop\GPURaytracer\frametimes.csv";
		writePerformanceLog(str1, &frameTimes);
		PostQuitMessage(0);
	}
		return 0;
	case WM_LBUTTONDOWN:
		break;
	case WM_RBUTTONDOWN:
		break;
	case WM_MBUTTONDOWN:
		if (pSample) {
			pSample->OnButtonDown(static_cast<UINT32>(lParam));
		}

	case WM_MOUSEMOVE:
		if (pSample) {
			pSample->OnMouseMove(static_cast<UINT8>(wParam),
				static_cast<UINT32>(lParam));
		}
		return 0;

	}

	// Handle any messages the switch statement didn't.
	return DefWindowProc(hWnd, message, wParam, lParam);
}

void writePerformanceLog(std::string filepath, std::vector<frameData>* frameTimes)
{
	std::ofstream frameTimeLog(filepath);
	frameTimeLog << "FrameID " << ';' << "FrameTime(ms)" << "," << std::endl;

	for (unsigned short i = 0; i < frameTimes->size(); i++)
		frameTimeLog << frameTimes->at(i).frameNumber << ';' << frameTimes->at(i).frameTime << "," << std::endl;

	frameTimeLog.close();

}

void clearFrameTimes(std::vector<frameData>& frameTimes)
{

	for (unsigned long i = frameTimes.size(); i >= 0; i--)
	{
		frameTimes.pop_back();
		//frameTimes.erase(frameTimes.begin() + i);
	}
}
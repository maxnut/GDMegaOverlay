#include "speedhack.h"

namespace Speedhack
{
	double speed = 1.0;
	bool initialised = false;

	_tGetTickCount _GTC = nullptr;
	DWORD _GTC_BaseTime = 0, _GTC_OffsetTime = 0;

	_tGetTickCount64 _GTC64 = nullptr;
	ULONGLONG _GTC64_BaseTime = 0, _GTC64_OffsetTime = 0;

	_tQueryPerformanceCounter _QPC = nullptr;
	LARGE_INTEGER _QPC_BaseTime = LARGE_INTEGER(), _QPC_OffsetTime = LARGE_INTEGER();

	DWORD WINAPI _hGetTickCount()
	{
		return _GTC_OffsetTime + ((_GTC() - _GTC_BaseTime) * speed);
	}

	ULONGLONG WINAPI _hGetTickCount64()
	{
		return _GTC64_OffsetTime + ((_GTC64() - _GTC64_BaseTime) * speed);
	}

	BOOL WINAPI _hQueryPerformanceCounter(LARGE_INTEGER *lpPerformanceCount)
	{
		LARGE_INTEGER x;
		_QPC(&x);
		lpPerformanceCount->QuadPart = _QPC_OffsetTime.QuadPart + ((x.QuadPart - _QPC_BaseTime.QuadPart) * speed);
		return TRUE;
	}

	void Setup()
	{
		_GTC = &GetTickCount;
		_GTC_BaseTime = _GTC();
		_GTC_OffsetTime = _GTC_BaseTime;

		_GTC64 = &GetTickCount64;
		_GTC64_BaseTime = _GTC64();
		_GTC64_OffsetTime = _GTC64_BaseTime;

		_QPC = &QueryPerformanceCounter;
		_QPC(&_QPC_BaseTime);
		_QPC_OffsetTime.QuadPart = _QPC_BaseTime.QuadPart;

		DetourTransactionBegin();
		DetourUpdateThread(GetCurrentThread());

		DetourAttach(&(PVOID &)_GTC, _hGetTickCount);
		DetourAttach(&(PVOID &)_GTC64, _hGetTickCount64);
		DetourAttach(&(PVOID &)_QPC, _hQueryPerformanceCounter);

		

		DetourTransactionCommit();

		initialised = true;
	}

	void Detach()
	{
		DetourTransactionBegin();
		DetourUpdateThread(GetCurrentThread());

		DetourDetach(&(PVOID &)_GTC, _hGetTickCount);
		DetourDetach(&(PVOID &)_GTC64, _hGetTickCount64);
		DetourDetach(&(PVOID &)_QPC, _hQueryPerformanceCounter);

		DetourTransactionCommit();

		initialised = false;
	}

	void SetSpeed(double relSpeed)
	{
		if (initialised)
		{
			_GTC_OffsetTime = _hGetTickCount();
			_GTC_BaseTime = _GTC();

			_GTC64_OffsetTime = _hGetTickCount64();
			_GTC64_BaseTime = _GTC64();

			_hQueryPerformanceCounter(&_QPC_OffsetTime);
			_QPC(&_QPC_BaseTime);
		}

		speed = relSpeed;
	}
}

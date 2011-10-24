#include <Windows.h>

#pragma once
ref class DepthEngine
{
private:

public:
	DepthEngine(void);
	void DepthEngine::Depth2Color(BYTE* depth, RGBQUAD* buf);
};


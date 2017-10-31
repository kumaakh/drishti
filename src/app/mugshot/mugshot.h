#pragma once
extern "C" __declspec(dllexport)
int __stdcall
takeMugShot(int camIdx, int width, int height, unsigned char *outBuff, int maxFrames);
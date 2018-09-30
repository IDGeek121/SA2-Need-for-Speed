// IDGeek121
// September 29, 2018
// Feel free to tweak this if you really care, but if you make something cooler, please make a PR.

#include <Windows.h>
#include <stdio.h>
#include <cmath>

float speedToFade(float hSpeed, float charSpeed, float *currFade);

int main()
{
	// Hook to process
	DWORD pid;
	HWND hWnd = FindWindowA(NULL, "SONIC ADVENTURE 2");
	GetWindowThreadProcessId(hWnd, &pid);
	HANDLE pHandle = OpenProcess(PROCESS_VM_READ | PROCESS_VM_WRITE | PROCESS_VM_OPERATION, FALSE, pid);

	while (pHandle)
	{
		// Character Speed Constants
		float speedVals[] = { 16.0f, 4.18f, 4.65f };
		float passedSpeed;

		// Value to be written to the fade byte this iteration
		float fadeValue;

		// Values to be read in
		byte timerEnd, stage, character, menuMode;
		float kartX, kartZ, hSpeed;
		DWORD ptr1, ptr2;

		// Read in necessary bytes
		ReadProcessMemory(pHandle, (LPVOID)0x0174AFDA, &timerEnd, sizeof(timerEnd), NULL);
		ReadProcessMemory(pHandle, (LPVOID)0x01934B70, &stage, sizeof(stage), NULL);
		ReadProcessMemory(pHandle, (LPVOID)0x01934B80, &character, sizeof(character), NULL);
		ReadProcessMemory(pHandle, (LPVOID)0x01934BE0, &menuMode, sizeof(menuMode), NULL);
		ReadProcessMemory(pHandle, (LPVOID)0x01A35DF0, &kartX, sizeof(kartX), NULL);
		ReadProcessMemory(pHandle, (LPVOID)0x01A35DF8, &kartZ, sizeof(kartZ), NULL);

		// Get HSpeed
		ReadProcessMemory(pHandle, (LPVOID)0x01DEA6E0, &ptr1, sizeof(ptr1), NULL);
		ReadProcessMemory(pHandle, (LPVOID)(ptr1 + 0x40), &ptr2, sizeof(ptr2), NULL);
		ReadProcessMemory(pHandle, (LPVOID)(ptr2 + 0x64), &hSpeed, sizeof(hSpeed), NULL);

		// Determine what speed to pass in based on character.
		if (character == 0 || character == 1)
		{
			passedSpeed = speedVals[0];
		}
		else if (character == 4 || character == 5)
		{
			passedSpeed = speedVals[1];
		}
		else if (character == 6 || character == 7)
		{
			passedSpeed = speedVals[2];
		}

		// If in-game and not in results screen...
		if (menuMode == 16 && !timerEnd)
		{
			// Kart speed check
			if (stage == 70)
			{
				float kartVel = std::sqrtf(std::pow(kartX, 2) + std::pow(kartZ, 2));
				fadeValue = speedToFade(kartVel, 15, &fadeValue);
			}
			// Normal speed otherwise
			else
			{
				fadeValue = speedToFade(hSpeed, passedSpeed, &fadeValue);
			}
			
			byte fadeValueByte = (byte)fadeValue;
			int result = WriteProcessMemory(pHandle, (LPVOID)0x0171CDA3, &fadeValueByte, sizeof(fadeValueByte), NULL);
		}
		// For smooth transition when fading into stage.
		else if (menuMode == 7)
		{
			fadeValue = 0;
		}

		Sleep(10);
	}
	return 0;
}

// Calculate the fade value to be written based on the given hSpeed.
float speedToFade(float hSpeed, float charSpeed, float *currFade)
{
	// Bounds check
	if (hSpeed > charSpeed)
	{
		hSpeed = charSpeed;
	}
	else if (hSpeed < 0)
	{
		hSpeed = 0;
	}

	// Linear scale
	float targetFade = (-(254 / charSpeed) * hSpeed + 254);

	// Smooth fading (epilepsy)
	*currFade += (float)(.1 * (targetFade - *currFade));

	return *currFade;
}
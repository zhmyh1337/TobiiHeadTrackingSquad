#include "tobii_gameintegration.h"
#include <algorithm>
#include <iostream>
#include <iomanip>
#include "windows.h"

using namespace TobiiGameIntegration;

#define ENABLE_PITCH 0

// settings tuned for Squad game with 3200x2000 res and 1000 DPI mouse
// change only the numbers, unless you know what you're doing
static constexpr float k_sens = 25.0f;
#if ENABLE_PITCH
static constexpr float k_ySensMult = 0.25f;
#else
static constexpr float k_ySensMult = 0.0f;
#endif
static constexpr float k_deadYawIRL = 2.5f;
static constexpr float k_deadPitchIRL = 2.5f;
static constexpr float k_maxYawIRL = 650.0f / k_sens + k_deadYawIRL;
#if ENABLE_PITCH
static constexpr float k_maxPitchIRL = 100.0f / (k_sens * k_ySensMult) + k_deadPitchIRL;
#else
static constexpr float k_maxPitchIRL = 0.0f;
#endif

HWND GetConsoleHwnd(void); // See SampleHelpFunctions.cpp

bool IsCursorVisible()
{
	CURSORINFO ci = { sizeof(CURSORINFO) };
	if (GetCursorInfo(&ci))
		return ci.flags & CURSOR_SHOWING;
	return false;
}

int main() {
	ITobiiGameIntegrationApi* api = GetApi("Extended View Sample");
	IExtendedView* extendedView = api->GetFeatures()->GetExtendedView();

	// Turn on head tracking position
	ExtendedViewSettings extendedViewSettings;
	extendedView->GetSettings(extendedViewSettings);
	extendedViewSettings.HeadTracking.PositionEnabled = true;
	extendedView->UpdateSettings(extendedViewSettings);

	api->GetTrackerController()->TrackWindow(GetConsoleHwnd());

	std::cout << "F8 to exit" << std::endl << std::endl;

	while (!GetAsyncKeyState(VK_F8))
	{
		Sleep(1);
		api->Update();

		if (IsCursorVisible())
		{
			continue;
		}

		const Transformation trans = extendedView->GetTransformation();

		std::cout << std::fixed << std::setprecision(3);
		std::cout << "Extended View Rot(deg) [Y: " << trans.Rotation.YawDegrees << ",P: " << trans.Rotation.PitchDegrees << ",R: " << trans.Rotation.RollDegrees << "] " <<
			"Pos(mm) [X: " << trans.Position.X << ",Y: " << trans.Position.Y << ",Z: " << trans.Position.Z << "]          \r";

		static float actualYaw = 0.0f;
		static float actualPitch = 0.0f;

		auto desiredYaw = trans.Rotation.YawDegrees;
		bool yawDead = std::abs(desiredYaw) < k_deadYawIRL;
		desiredYaw = std::clamp(desiredYaw, -k_maxYawIRL, k_maxYawIRL);
		if (!yawDead)
		{
			desiredYaw -= k_deadYawIRL * (desiredYaw >= 0.0f ? 1.0f : -1.0f);
		}

		auto desiredPitch = trans.Rotation.PitchDegrees;
		bool pitchDead = std::abs(desiredPitch) < k_deadPitchIRL;
		desiredPitch = std::clamp(desiredPitch, -k_maxPitchIRL, k_maxPitchIRL);
		if (!pitchDead)
		{
			desiredPitch -= k_deadPitchIRL * (desiredPitch >= 0.0f ? 1.0f : -1.0f);
		}

		desiredYaw *= k_sens;
		desiredPitch *= k_sens * k_ySensMult;

		if (yawDead)
		{
			desiredYaw = 0.0f;
		}
		if (pitchDead)
		{
			desiredPitch = 0.0f;
		}

		auto desiredDeltaYaw = desiredYaw - actualYaw;
		auto desiredDeltaPitch = desiredPitch - actualPitch;

		long dx = static_cast<long>(std::round(desiredDeltaYaw));
		long minusDy = static_cast<long>(std::round(desiredDeltaPitch));

		actualYaw += static_cast<float>(dx);
		actualPitch += static_cast<float>(minusDy);

		if (dx || minusDy)
		{
			mouse_event(MOUSEEVENTF_MOVE, static_cast<DWORD>(dx), static_cast<DWORD>(-minusDy), 0, 0);
		}
	}

	api->Shutdown();
}

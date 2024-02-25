#include "tobii_gameintegration.h"
#include <iostream>
#include <iomanip>
#include "windows.h"
#include <thread>
#include <chrono>

using namespace TobiiGameIntegration;

static constexpr float sens = 15.0f;
static constexpr float YSensMult = 0.25f;

HWND GetConsoleHwnd(void); // See SampleHelpFunctions.cpp

int main() {
	ITobiiGameIntegrationApi* api = GetApi("Extended View Sample");
	IExtendedView* extendedView = api->GetFeatures()->GetExtendedView();

	// Turn on head tracking position
	ExtendedViewSettings extendedViewSettings;
	extendedView->GetSettings(extendedViewSettings);
	extendedViewSettings.HeadTracking.PositionEnabled = true;
	extendedView->UpdateSettings(extendedViewSettings);

	api->GetTrackerController()->TrackWindow(GetConsoleHwnd());

	//std::cout << "Right Control: reset head pose, Left Control: hold to pause extended view." << std::endl << std::endl;

	while (!GetAsyncKeyState(VK_F8))
	{
		api->Update();

		const SHORT onKeyDownCode = (SHORT)0x8001;
		const SHORT isKeyDownCode = (SHORT)0x8000;
		//bool resetDefaultHeadPose = GetAsyncKeyState(VK_RCONTROL) == onKeyDownCode;
		//bool pause = (GetAsyncKeyState(VK_RCONTROL) & isKeyDownCode) != 0;

		//if (resetDefaultHeadPose)
		//{
		//	extendedView->ResetDefaultHeadPose(); // this resets the default head pose, such that the user's current head pose is considered neutral
		//}

		//pause ? extendedView->Pause(false) : extendedView->UnPause(); // pausing and unpausing extended view via this function smoothly transitions to the new values

		static float consumedYaw = 0;
		static float consumedPitch = 0;

		//static bool wasKeyDown = true;
		//bool isKeyDown = GetAsyncKeyState(VK_LMENU) & isKeyDownCode;
		//if (wasKeyDown && !isKeyDown)
		//{
		//	extendedView->ResetDefaultHeadPose();
		//	consumedYaw = 0;
		//	consumedPitch = 0;
		//	api->Update();
		//}

		const Transformation trans = extendedView->GetTransformation();

		std::cout << std::fixed << std::setprecision(3);
		std::cout << "Extended View Rot(deg) [Y: " << trans.Rotation.YawDegrees << ",P: " << trans.Rotation.PitchDegrees << ",R: " << trans.Rotation.RollDegrees << "] " <<
			"Pos(mm) [X: " << trans.Position.X << ",Y: " << trans.Position.Y << ",Z: " << trans.Position.Z << "]          \r";

		//if (!isKeyDown)
		//{
		auto deltaYaw = trans.Rotation.YawDegrees - consumedYaw;
		auto deltaPitch = trans.Rotation.PitchDegrees - consumedPitch;
		long dx = deltaYaw * sens;
		long dy = deltaPitch * sens * YSensMult;
		consumedYaw += dx / sens;
		consumedPitch += dy / sens / YSensMult;
		consumedYaw = trans.Rotation.YawDegrees;
		consumedPitch = trans.Rotation.PitchDegrees;
		if (std::abs(dx) < 10) {
			dx = 0;
		}
		if (std::abs(dy) < 10) {
			dy = 0;
		}
		mouse_event(MOUSEEVENTF_MOVE, (DWORD)dx, (DWORD)-dy, 0, 0);
		//}

		//wasKeyDown = isKeyDown;
		Sleep(1);
	}

	api->Shutdown();
}

#include "tobii_gameintegration.h"
#include <iostream>
#include <iomanip>
#include "windows.h"
#include <thread>
#include <chrono>

using namespace TobiiGameIntegration;

static constexpr float speedX = 200.0f;
static constexpr float speedY = 50.0f;
static constexpr float deadRadius = 20.0f;
static constexpr float bias = -deadRadius;
static constexpr float clamp = 10000.0f;

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

	std::cout << "Right Control: reset head pose, Left Control: hold to pause extended view." << std::endl << std::endl;

	while (!GetAsyncKeyState(VK_F8))
	{
		api->Update();

		const SHORT onKeyDownCode = (SHORT)0x8001;
		const SHORT isKeyDownCode = (SHORT)0x8000;
		bool resetDefaultHeadPose = GetAsyncKeyState(VK_RCONTROL) == onKeyDownCode;
		//bool pause = (GetAsyncKeyState(VK_RCONTROL) & isKeyDownCode) != 0;

		if (resetDefaultHeadPose)
		{
			extendedView->ResetDefaultHeadPose(); // this resets the default head pose, such that the user's current head pose is considered neutral
			api->Update();
		}

		//pause ? extendedView->Pause(false) : extendedView->UnPause(); // pausing and unpausing extended view via this function smoothly transitions to the new values
		static auto lastTime = std::chrono::high_resolution_clock::now();
		auto nowTime = std::chrono::high_resolution_clock::now();
		auto deltaSeconds = std::chrono::duration_cast<std::chrono::duration<float>>(nowTime - lastTime).count();

		const Transformation trans = extendedView->GetTransformation();

		//std::cout << std::fixed << std::setprecision(3);
		//std::cout << "Extended View Rot(deg) [Y: " << trans.Rotation.YawDegrees << ",P: " << trans.Rotation.PitchDegrees << ",R: " << trans.Rotation.RollDegrees << "] " <<
		//	"Pos(mm) [X: " << trans.Position.X << ",Y: " << trans.Position.Y << ",Z: " << trans.Position.Z << "]          ";

		auto dist = std::sqrt(trans.Rotation.YawDegrees * trans.Rotation.YawDegrees + trans.Rotation.PitchDegrees * trans.Rotation.PitchDegrees);
		//std::cout << std::endl << std::endl;
		if (dist > deadRadius)
		{
			//auto dx = (std::signbit(trans.Rotation.YawDegrees) ? -1 : 1) * (std::min)(clamp, std::pow(base, (std::abs(trans.Rotation.YawDegrees) + bias) * speed)) * deltaSeconds;
			//auto dy = (std::signbit(trans.Rotation.PitchDegrees) ? -1 : 1) * (std::min)(clamp, std::pow(base, (std::abs(trans.Rotation.PitchDegrees) + bias) * speed)) * deltaSeconds;
			auto dx = (std::signbit(trans.Rotation.YawDegrees) ? -1 : 1) * (std::min)(clamp, (std::abs(trans.Rotation.YawDegrees) + bias) * speedX) * deltaSeconds;
			auto dy = (std::signbit(trans.Rotation.PitchDegrees) ? -1 : 1) * (std::min)(clamp, (std::abs(trans.Rotation.PitchDegrees) + bias) * speedY) * deltaSeconds;
			mouse_event(MOUSEEVENTF_MOVE, (DWORD)dx, (DWORD)-dy, 0, 0);
			std::cout << dx << " " << dy;
		}
		else
		{
			std::cout << "DEAD";
		}

		std::cout << "                                                    \r";

		//if (!isKeyDown)
		//{
		//	auto deltaYaw = trans.Rotation.YawDegrees - consumedYaw;
		//	auto deltaPitch = trans.Rotation.PitchDegrees - consumedPitch;
		//	long dx = deltaYaw * sens;
		//	long dy = deltaPitch * sens;
		//	consumedYaw += dx / sens;
		//	consumedPitch += dy / sens;
		//	mouse_event(MOUSEEVENTF_MOVE, (DWORD)dx, (DWORD)-dy, 0, 0);
		//}
		//
		//wasKeyDown = isKeyDown;

		lastTime = nowTime;
		Sleep(1);
	}

	api->Shutdown();
}

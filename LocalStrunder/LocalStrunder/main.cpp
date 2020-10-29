#include "main.h"


bool active = true; 
int BulletCount = 1, OriginalAmmo = 32767, OriginalAmmoInMgz = 32767;
BOOL bNoRecoil, bPenetration, bUnlimitedAmmo;
SOCKET g_Sock;
DWORD g_PID;
uint64_t g_Base;
PlayerData myData = PlayerData(), curPawn = PlayerData();
TArray < DWORD > CurrentPRA;

static const char* ConHdr =
"=================================================\n"
"|                OdinSF - Client                |\n"
"|This software is intended for private use only.|\n"
"|                 Version : 1.4.5               |\n"
"=================================================\n\n";

void DoActions() {
	while (active)
	{
		Sleep(1);
		if (GetAsyncKeyState(VK_F5) & 1)
		{
			bNoRecoil = !bNoRecoil;
		}
		if (GetAsyncKeyState(VK_F6) & 1)
		{
			bPenetration = !bPenetration;
		}
		if (GetAsyncKeyState(VK_F7) & 1)
		{
			if (BulletCount != 1)
			{
				BulletCount--;
				printf("Bullet : %i\n", BulletCount);
			}
		}
		if (GetAsyncKeyState(VK_F8) & 1)
		{
			if (BulletCount != 25)
			{
				BulletCount++;
				printf("Bullet : %i\n", BulletCount);
			}
		}
		if (GetAsyncKeyState(VK_F9) & 1)
		{
			bUnlimitedAmmo = !bUnlimitedAmmo;
			if (!bUnlimitedAmmo)
			{
				OriginalAmmoInMgz = 32767;
				OriginalAmmo = 32767;
				printf("Unlimited Ammo : Not Active\n");
			}
		}
	}
}

void render(void* ovv) {
	if (g_PID == 0 || g_Base == 0) {
		return;
	}
	Overlay* ov = (Overlay*)ovv;
	Direct dx = ov->CurrentDirectX;
	if (bNoRecoil)
	{
		dx.DrawString(10, 50, 255, 46, 135, 190, L"No Recoil : Active");
	}
	else
	{
		dx.DrawString(10, 50, 255, 46, 135, 190, L"No Recoil : Not Active");
	}
	if (bPenetration)
	{
		dx.DrawString(10, 60, 255, 46, 135, 190, L"Penetration Exploit : Active");
	}
	else
	{
		dx.DrawString(10, 60, 255, 46, 135, 190, L"Penetration Exploit : Not Active");
	}
	if (bUnlimitedAmmo)
	{
		dx.DrawString(10, 70, 255, 46, 135, 190, L"Unlimited Ammo : Active");
	}
	else
	{
		dx.DrawString(10, 70, 255, 46, 135, 190, L"Unlimited Ammo : Not Active");
	}
	myData.PlayerController = driver::read<DWORD>(g_Sock, g_PID, g_Base + SFPlayerControllerPtr);
	if (myData.PlayerController == 0) return;
	DWORD WorldInfo = driver::read<DWORD>(g_Sock, g_PID, (uintptr_t)myData.PlayerController + OFFSET_WorldInfo);
	DWORD PawnList = driver::read<DWORD>(g_Sock, g_PID, (uintptr_t)WorldInfo + OFFSET_PawnList);
	while (PawnList)
	{
		myData.Pointer = driver::read<DWORD>(g_Sock, g_PID, (uintptr_t)myData.PlayerController + OFFSET_myPawn);
		DWORD PlayerCamera = driver::read<DWORD>(g_Sock, g_PID, (uintptr_t)myData.PlayerController + OFFSET_PlayerCam);
		DWORD PlayerReplication = driver::read<DWORD>(g_Sock, g_PID, (uintptr_t)myData.PlayerController + OFFSET_ViewTarget); if (PlayerReplication == 0)continue;
		PlayerReplication = driver::read<DWORD>(g_Sock, g_PID, (uintptr_t)PlayerReplication + OFFSET_PRI);
		myData.TeamIndex = driver::read<DWORD>(g_Sock, g_PID, (uintptr_t)driver::read<DWORD>(g_Sock, g_PID, (uintptr_t)PlayerReplication + OFFSET_TeamInfo) + OFFSET_TeamIndex);
		curPawn.Pointer = PawnList;
		if (myData.Pointer == curPawn.Pointer) { PawnList = driver::read<DWORD>(g_Sock, g_PID, (uintptr_t)curPawn.Pointer + OFFSET_NextPawn); continue; }
		PlayerReplication = driver::read<DWORD>(g_Sock, g_PID, (uintptr_t)curPawn.Pointer + OFFSET_PRI);
		curPawn.TeamIndex = driver::read<DWORD>(g_Sock, g_PID, (uintptr_t)driver::read<DWORD>(g_Sock, g_PID, (uintptr_t)PlayerReplication + OFFSET_TeamInfo) + OFFSET_TeamIndex);
		PawnList = driver::read<DWORD>(g_Sock, g_PID, (uintptr_t)curPawn.Pointer + OFFSET_NextPawn);
		if (curPawn.TeamIndex == myData.TeamIndex) { curPawn.Pointer = 0; continue; }
		myData.FOVAngle = driver::read<float>(g_Sock, g_PID, (uintptr_t)myData.PlayerController + OFFSET_ViewAngle);
		myData.CameraRotation.Pitch = driver::read<int>(g_Sock, g_PID, (uintptr_t)PlayerCamera + 0x60); myData.CameraRotation.Yaw = driver::read<int>(g_Sock, g_PID, (uintptr_t)PlayerCamera + 0x64); myData.CameraRotation.Roll = driver::read<int>(g_Sock, g_PID, (uintptr_t)PlayerCamera + 0x68);
		myData.CameraLocation.X = driver::read<float>(g_Sock, g_PID, (uintptr_t)PlayerCamera + 0x54); myData.CameraLocation.Y = driver::read<float>(g_Sock, g_PID, (uintptr_t)PlayerCamera + 0x58); myData.CameraLocation.Z = driver::read<float>(g_Sock, g_PID, (uintptr_t)PlayerCamera + 0x5C);
		myData.Location3D.X = driver::read<float>(g_Sock, g_PID, (uintptr_t)myData.Pointer + 0x54); myData.Location3D.Y = driver::read<float>(g_Sock, g_PID, (uintptr_t)myData.Pointer + 0x58); myData.Location3D.Z = driver::read<float>(g_Sock, g_PID, (uintptr_t)myData.Pointer + 0x5C);
		myData.Health = driver::read<int>(g_Sock, g_PID, (uintptr_t)myData.Pointer + OFFSET_Health);
		curPawn.PlayerName.Count = driver::read<DWORD>(g_Sock, g_PID, (uintptr_t)PlayerReplication + OFFSET_NameCount);
		wchar_t Name[32]; 
		wchar_t SomeThing = driver::read_SP<wchar_t>(g_Sock, g_PID, (uintptr_t)driver::read<DWORD>(g_Sock, g_PID, (uintptr_t)PlayerReplication + OFFSET_Name), ((uintptr_t)curPawn.PlayerName.Count * 2));
		wcscpy(Name, L"");wcscat(Name, &SomeThing);
		curPawn.PlayerName.Data = (wchar_t*)&Name;
		curPawn.Health = driver::read<int>(g_Sock, g_PID, (uintptr_t)curPawn.Pointer + OFFSET_Health);	if (curPawn.Health <= 0)continue;
		curPawn.EyeHeight = driver::read<float>(g_Sock, g_PID, (uintptr_t)curPawn.Pointer + OFFSET_EyeHeight);
		curPawn.Location3D.X = driver::read<float>(g_Sock, g_PID, (uintptr_t)curPawn.Pointer + 0x54); curPawn.Location3D.Y = driver::read<float>(g_Sock, g_PID, (uintptr_t)curPawn.Pointer + 0x58); curPawn.Location3D.Z = driver::read<float>(g_Sock, g_PID, (uintptr_t)curPawn.Pointer + 0x5C);
		Vector3 HeadLocation = curPawn.Location3D; HeadLocation.Z += curPawn.EyeHeight;
		curPawn.W2S = WorldToScreen(curPawn.Location3D, myData.FOVAngle, myData, curPawn, ov->getWidth(), ov->getHeight());
		HeadLocation = WorldToScreen(HeadLocation, myData.FOVAngle, myData, curPawn, ov->getWidth(), ov->getHeight());
		std::wstring Distance = std::to_wstring((myData.DistTo(curPawn.Location3D) / 70)); Distance = Distance.substr(0, Distance.find('.')) + L"m";
		std::wstring Health = std::to_wstring(curPawn.Health); Health = L"HP: " + Health.substr(0, Health.find('.'));

		if (curPawn.W2S.X > 0 && curPawn.W2S.Y > 0)
		{
			dx.DrawString(curPawn.W2S.X - 1, curPawn.W2S.Y - 14, 255, 0, 0, 0, Distance.c_str()); dx.DrawString(curPawn.W2S.X, curPawn.W2S.Y - 15, 255, 255, 0, 0, Distance.c_str());
			dx.DrawString(curPawn.W2S.X - 1, curPawn.W2S.Y - 29, 255, 0, 0, 0, Health.c_str()); dx.DrawString(curPawn.W2S.X, curPawn.W2S.Y - 30, 255, 255, 0, 0, Health.c_str());
			dx.DrawStringWCH(curPawn.W2S.X - 1, curPawn.W2S.Y - 44, 255, 0, 0, 0, curPawn.PlayerName.Data); 
			dx.DrawString(curPawn.W2S.X, curPawn.W2S.Y - 45, 255, 255, 0, 0, curPawn.PlayerName.Data);
			dx.DrawLine(ov->getWidth() / 2, ov->getHeight(), curPawn.W2S.X, curPawn.W2S.Y, 255, 255, 0, 0);
		}
		
		if (myData.Health <= 0) { OriginalAmmo = 32767; OriginalAmmoInMgz == 32767; driver::write<DWORD>(g_Sock, g_PID, (uintptr_t)myData.PlayerController + OFFSET_bRecoilCam, 2116); continue; }
		DWORD Weapon = driver::read<DWORD>(g_Sock, g_PID, (uintptr_t)myData.Pointer + OFFSET_Weapon);
		DWORD OATStageOne = driver::read<DWORD>(g_Sock, g_PID, (uintptr_t)Weapon + ObjectArchetype);
		DWORD OATStageTwo = driver::read<DWORD>(g_Sock, g_PID, (uintptr_t)OATStageOne + ObjectArchetype);
		DWORD OATStageThree = driver::read<DWORD>(g_Sock, g_PID, (uintptr_t)OATStageTwo + ObjectArchetype);
		if (Weapon && OATStageOne && OATStageTwo && OATStageThree && myData.PlayerController)
		{
			/*
				Desc : Have to check if the weapon is OK to mod
				Melee Hold :	7423 	Default__SFWeap_MeleeSwing
				Melee Knife:	7421	Default__SFWeap_Melee
				AR Pistol : 	7413 	Default__SFWeap_Bullet
				Nade :			39809 	Default__SFWeap_HandGrenade
				Sniper : 		39815 	Default__SFWeap_SniperBase
			*/
			int curWeaponType = driver::read<int>(g_Sock, g_PID, (uintptr_t)OATStageThree + NameIndex);
			if (curWeaponType == 39809 || curWeaponType == 7423 || curWeaponType == 7421)continue;
			if (bNoRecoil && curWeaponType == 7413)
			{
				driver::write<float>(g_Sock, g_PID, (uintptr_t)Weapon + OFFSET_mulValueWalk, 0.0f);
				driver::write<float>(g_Sock, g_PID, (uintptr_t)Weapon + OFFSET_mulValueRun, 0.0f);
				driver::write<float>(g_Sock, g_PID, (uintptr_t)Weapon + OFFSET_mulValueJump, 0.0f);
				//driver::write<float>(g_Sock, g_PID, (uintptr_t)myData.Pointer + 0xFA8, 0.0f); //currentCrossVert
				driver::write<float>(g_Sock, g_PID, (uintptr_t)myData.Pointer + 0xFAC, 0.8f); //expectCrossVert
				driver::write<float>(g_Sock, g_PID, (uintptr_t)myData.Pointer + 0xFB0, 1.0f); //crossDecreasValue
				driver::write<float>(g_Sock, g_PID, (uintptr_t)myData.Pointer + 0xFD4, 0.0f); //currentDomValue
				driver::write<float>(g_Sock, g_PID, (uintptr_t)myData.Pointer + 0xFE0, 0.0f); //spiny
				driver::write<DWORD>(g_Sock, g_PID, (uintptr_t)myData.PlayerController + OFFSET_bRecoilCam, 2116);
			}
			else if (bNoRecoil && curWeaponType == 39815)
			{
				driver::write<float>(g_Sock, g_PID, (uintptr_t)Weapon + OFFSET_mulValueWalk, 0.0f);
				driver::write<float>(g_Sock, g_PID, (uintptr_t)Weapon + OFFSET_mulValueRun, 0.0f);
				driver::write<float>(g_Sock, g_PID, (uintptr_t)Weapon + OFFSET_mulValueJump, 0.0f);
				driver::write<float>(g_Sock, g_PID, (uintptr_t)myData.Pointer + 0xFA8, 0.00f); //currentCrossVert
				driver::write<float>(g_Sock, g_PID, (uintptr_t)myData.Pointer + 0xFAC, 0.00f); //expectCrossVert
				driver::write<float>(g_Sock, g_PID, (uintptr_t)myData.Pointer + 0xFB0, 1.0f); //crossDecreasValue
				driver::write<float>(g_Sock, g_PID, (uintptr_t)myData.Pointer + 0xFD4, 0.00f); //currentDomValue
				driver::write<float>(g_Sock, g_PID, (uintptr_t)myData.Pointer + 0xFE0, 0.00f); //spiny
				driver::write<DWORD>(g_Sock, g_PID, (uintptr_t)myData.PlayerController + OFFSET_bRecoilCam, 1792);
			}
			else
			{
				driver::write<DWORD>(g_Sock, g_PID, (uintptr_t)myData.PlayerController + OFFSET_bRecoilCam, 2116);
			}
			driver::write<int>(g_Sock, g_PID, (uintptr_t)Weapon + OFFSET_SGPelletCount, BulletCount);
			if (bUnlimitedAmmo)
			{
				if (OriginalAmmo == 32767 && OriginalAmmoInMgz == 32767)
				{
					OriginalAmmo = driver::read<int>(g_Sock, g_PID, (uintptr_t)Weapon + OFFSET_AmmoCount);
					OriginalAmmoInMgz = driver::read<int>(g_Sock, g_PID, (uintptr_t)Weapon + OFFSET_AmmoInMgz);
				}
				driver::write<int>(g_Sock, g_PID, (uintptr_t)Weapon + OFFSET_AmmoCount, OriginalAmmo);
				driver::write<int>(g_Sock, g_PID, (uintptr_t)Weapon + OFFSET_AmmoInMgz, OriginalAmmoInMgz);
			}
			if (bPenetration)
			{
				driver::write<float>(g_Sock, g_PID, (uintptr_t)Weapon + OFFSET_ThicknessForPenetrate, 3.40282e+38);
				driver::write<float>(g_Sock, g_PID, (uintptr_t)Weapon + OFFSET_fPenetrationMax, 3.40282e+38);
				driver::write<DWORD>(g_Sock, g_PID, (uintptr_t)Weapon + OFFSET_MaxPenetrationCount, 2147483647);
			}
		}
	}
}

void getPID()
{
	if (g_PID != 0)return;
	printf("Searching for Special Force 2...\n");
	while (g_PID == 0)
	{
		Sleep(1000);
		std::wstring yw = (std::wstring(L"s") + std::wstring(L"f2") + std::wstring(L".exe"));
		const wchar_t* ptr = yw.c_str();
		g_PID = driver::get_process_pid(g_Sock, (wchar_t*)ptr, yw.length());

		memset((void*)&ptr[0], 0x00, yw.size() * 2);
		yw.clear();
	}
	printf("Special Force 2 found! PID : %X\n", g_PID);
	Sleep(3000);
}

bool getBase() {
	if (g_Base != 0)return true;
	g_Base = driver::get_process_base_address(g_Sock, g_PID);
	if (g_Base == 0)
	{
		log("Failed to get base address!");
		g_PID = 0;
		Sleep(500);
		return false;
	}
	printf("Process Base Address : %X\n", (DWORD)g_Base);
	SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 12);
	printf("System Ready.\n");
	SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 0xE);
	printf("F5 - No Recoil\nF6 - Penetration Exploit\nF7,F8 - Damage Exploit\nF9 - Unlimited Ammo\nF12 - AZ-5 Shut Down\n");
	return true;
}

std::string a = "";
std::string b = "9af7c125";
char c;

int main(int argc, char** argv)
{
	SetConsoleTitle(L"OdinSF - Client");
	SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 0xF);
	printf(ConHdr);
	SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 0x8);
	CreateThread(NULL, NULL, (LPTHREAD_START_ROUTINE)DoActions, NULL, NULL, NULL);
	if (argc > 1) {
		driver::initialize();
		g_Sock = driver::connect();
		driver::deinitialize();
		exit(0);
		return 0;
	}
	J();
	std::cout << "\nConnecting to OdinSF - Driver...\n";
	driver::initialize();
	J();
	g_Sock = driver::connect();
	if (g_Sock == INVALID_SOCKET)
	{
		std::cout << "Connection Failure!\n";
		Sleep(500);
		exit(0);
		return 0;
	}
	std::cout << "Connected! Initialization completed!\n";
	Overlay ov1 = Overlay();
	ov1.SetRender(render);
	ov1.Start();
	DWORD tid = 0;
	while (active) {
		getPID();
		if (!getBase()) continue;
		ov1.SetTargetHWND(Utils::FindMainWindow(g_PID));
		if (!ov1.isRunning()) {
			ov1.Start();
		}
		while (active)
		{
			Sleep(1);
			if (GetAsyncKeyState(VK_F12) & 32768) { active = false; break; }
		}
	}
	ov1.Clear();
	active = false;
	driver::deinitialize();
	J();
	return 0;
}

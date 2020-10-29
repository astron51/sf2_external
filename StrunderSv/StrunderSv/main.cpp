#include "bypass.h"
#include "server.h"

VOID ActiveLoop(_In_ PVOID StartContext) {
	UNICODE_STRING uniName;
	RtlInitUnicodeString(&uniName, L"\\DosDevices\\C:\\c.txt");
	while (GetGlobalActive()) {
		Sleep(2000); //Every 2 seconds check for close driver request
		if (fileExists(uniName)) {
			log("Bye\n");
			SetGlobalActive(FALSE);
			return;
		}
	}
}

void InitThreads() {
	StartThread(ActiveLoop, nullptr);
	StartThread(server_thread, nullptr);
}

VOID RealM(void* args) {
	log("Driver Loading\n");
	Sleep(2000); //make sure kdmapper closed
	log("Driver Loaded\n");
	J();

	//PREPARE BUFFERED AREA
	log("To Allocate %ld bytes\n", sizeof(BasicDataArea));

	UNICODE_STRING intelDrv;
	DataArea.IVDName[0] = L'i';
	DataArea.IVDName[2] = L'q';
	DataArea.IVDName[4] = L'v';
	DataArea.IVDName[6] = L'w';
	DataArea.IVDName[8] = L'6';
	DataArea.IVDName[10] = L'4';
	DataArea.IVDName[12] = L'e';
	DataArea.IVDName[14] = L'.';
	DataArea.IVDName[16] = L's';
	DataArea.IVDName[18] = L'y';
	DataArea.IVDName[20] = L's';
	RtlInitUnicodeString(&intelDrv, (PCWSTR)DataArea.IVDName);// L"iqvw64e.sys");
	J();

	const UINT64 result = ClearCacheEntry(intelDrv);

	RtlSecureZeroMemory(intelDrv.Buffer, intelDrv.Length);
	intelDrv.Length = 0;
	intelDrv.MaximumLength = 0;
	RtlSecureZeroMemory(DataArea.IVDName, 100);
	if (result == 0) {
		log("Load cancelled, can't clean the PiDDBCache\n");
		return;
	}
	J();

	InitThreads();
	J();
}

NTSTATUS MainEntry(PDRIVER_OBJECT driver, PUNICODE_STRING path) {
	StartThread(RealM, nullptr);
	return STATUS_SUCCESS;
}
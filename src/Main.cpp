#include <Windows.h>
#include <string>
#include <iostream>
#include <new>
#include <ddraw.h>
#include <wingdi.h>
#include <stdint.h>
#include "Hooks.h"



extern "C"
{
	__declspec(dllexport) void Start();
}

#define Naked __declspec( naked )  

using namespace std;

/*
MIXMASTER BRASIL - DLL MIXMASTER PATCH

-> Controlador de pacotes
-> Alteração da resolução do jogo
-> Leitura de arquivos personalizados
-> Anti-Hack
-> Patchs - Ajustes no jogo

*/

void StartInjection();
void Start();
BOOL APIENTRY DllMain(HINSTANCE hInst, DWORD reason, LPVOID reserved)
{
	switch (reason)
	{
	case DLL_PROCESS_ATTACH:
		Start();
		break;
	case DLL_THREAD_ATTACH:
		break;
	case DLL_THREAD_DETACH:
		break;
	case DLL_PROCESS_DETACH:
		break;
	}

	return TRUE;
}



void Start()
{
	CreateThread(NULL, NULL, LPTHREAD_START_ROUTINE(StartInjection), NULL, 0, 0);
}

void StartInjection()
{
	AllocConsole();
	freopen("conin$", "r", stdin);
	freopen("conout$", "w", stdout);
	freopen("conout$", "w", stderr);
	SetConsoleTitle(L"MixMaster Hook - DLL Output");
	cout << "Console de testes ... " << endl;
	cout << "DLL Injetada!" << endl;



	chat::ClientSendMessageChat("[+] MixMaster API Initialized!");



	// hook client receive packets
	ReceiveHook::InitializePacketReceiveHook();


	//Patch::FixSelectCharacters();
	//Draw::InitializeTextDrawHook();

}




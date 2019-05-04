#pragma once
#include <Windows.h>
#include <commapi.h>
#include <iostream>
#include "MMAPI.h"

#define Naked __declspec( naked )  

using namespace std;
namespace chat
{

	void *Send_ChatMessage_Address = (void*)0x432880; // endereço da função; SendChat(mensagem, ColorA, ColorB)
	bool ClientSendMessageChat(char *Mensagem)
	{
		__asm
		{
			PUSHAD
			push 0xFF000000
			push 0xFFFFFF20
			push Mensagem
			mov ecx, 0x5D40D0 // endereço do tipo de chat; normal, guild, whispper etc ... 
			CALL Send_ChatMessage_Address
			POPAD
		}
		return true;
	}

}

namespace ReceiveHook
{
	void * ReceivePacketJMPHookAddress = (void*)0x414655; // endereço que irá alocar o JMP
	DWORD * ReceivePacketJMPHookAddresCallback = (DWORD*)0x41465A; // endereço de retorno
	int PacketType;
	int BufferPointer;
	WORD PacketLen = 0;
	char *Buffer;


	void ZSHandlePacket()
	{
		switch (PacketType)
		{
		case 2:
			cout << "[Received] Personagens ..." << endl;
			break;
		case 16:
			cout << "[Received] Confirmação de autenticação do usuário ..." << endl;
			break;
		case 102:
			cout << "[Received] informações do usuário: Dados do personagem." << endl;
			break;
		case 67:
			cout << "[Received] informações do usuário: Dados do inventário." << endl;
			break;
		case 202:
			cout << "[Received] Mensagem: Megaphone" << endl;
			break;
		case 125:
			cout << "[Received] Mensagem: Chat" << endl;
			break;
		case 126:
			cout << "[Received] Mensagem: Whisper chat" << endl;
			break;
		case 120:
			cout << "[Received] Entidade: Caminhou!" << endl;
			break;
		case 107:
			cout << "[Received] Entidade: Spawnou!" << endl;
			break;
		case 109:
			cout << "[Received] Entidade: Despawn" << endl;
			break;
		case 138:
			cout << "[Received] Entidade: Mudou de sprite" << endl;
			break;
		case 154:
			cout << "[Received] Entidade: Usou skill" << endl;
			break;
		case 122:
			cout << "[Received] Drop: Item" << endl;
			break;
		case 123:
			cout << "[Received] Item despawn" << endl;
			break;
		case 108:
			cout << "[Received] Mob spawn in window" << endl;
			break;
		case 250:
			cout << "[Received] Jogador usou tapete!" << endl;
			break;
		case 110:
			cout << "[Received] Jogador: Sentou ou levantou!" << endl;
			break;
		case 189:
			cout << "[Received] Atualização de status!" << endl;
			break;
		case 170:
			cout << "[Received] Trade request!" << endl;
			break;
		case 246:
			cout << "[Received] Friend request!" << endl;
			break;
		case 201:
			cout << "[Received] Party Request!" << endl;
			break;
		case 180:
			cout << "[Received] Forced disconnect!" << endl;
			break;
		case 117:
			cout << "[Received] Update char position!" << endl;
			break;
		default:
			//cout << "Pacote desconhecido: " << PacketType << endl;
			break;
		}
	}

	void ProcessPacketReceived()
	{
		//PacketLen = MMAPI::readMemory<WORD>((LPVOID)BufferPointer); // tamanho do pacote
		//Buffer = new (nothrow) char[PacketLen]; // cria um buffer com o tamanho do pacote
		//memcpy((void*)Buffer, (void*)(BufferPointer + 6), PacketLen); // copia o pacote para o buffer criado
		ZSHandlePacket(); // processa o buffer
	}

	Naked void MyReceivePacketFunction()
	{
		__asm
		{
			PUSHAD
			PUSHFD
			movzx eax, BYTE PTR DS : [0x535DAC] // 0x535DAC endereço do tipo do pacote
			mov PacketType, eax
			mov BufferPointer, 0x535DB0
		}
		ProcessPacketReceived();
		__asm
		{
			POPFD
			POPAD
			MOV BYTE PTR DS:[0x535DAC], AL
			//mov ecx, offset BufferPointer
			JMP ReceivePacketJMPHookAddresCallback
		}

	}

	bool InitializePacketReceiveHook()
	{
		// func address 0x415020 packet handle
		// create hook at 0x4150C7
		// return hook to 0x4150CC
		cout << "[+] Initializing intercepting Packet Receive hook function!" << endl;
		MMAPI::PlaceJMP((BYTE*)ReceivePacketJMPHookAddress, (DWORD)MyReceivePacketFunction, 5);
		cout << "[+]  Packet Receive Hook Initialized!" << endl;
		return true;
	}
}


namespace Patch
{
	void * FixCharHookAddr = (void*)0x04081CF;
	DWORD * FixCharFuncCallbak = (DWORD*)0x04081D4;
	DWORD * FixCharFuncReadByte = (DWORD*)0x451130;


	Naked void MyFixCharFunc()
	{
		__asm
		{
			PUSHAD
			PUSHFD
			//CALL FixCharFuncReadByte
		}
		__asm
		{
			POPFD
			POPAD

			//MOV BYTE PTR DS : [0x545A10], AL
			
			JMP FixCharFuncCallbak;
		}
	}

	bool FixSelectCharacters()
	{
		// address to hook 004081D6
		// address to callback hook 004081DB
		cout << "[+] Initializing Fix Character Selector!" << endl;
		MMAPI::PlaceJMP((BYTE*)FixCharHookAddr, (DWORD)MyFixCharFunc, 5);
		cout << "[+]  Fixed Character View!" << endl;

		return true;
	}

}

namespace Draw
{
	void *DrawBaseFunc = (void*)0x448BB0;
	DWORD * DrawBaseAddrClass = (DWORD*)0x0AC5204;
	DWORD Color = 0x0FFFFFF;

	DWORD Address1 = 0x063FE7A;
	DWORD Address2 = 0x052A678;


	void * TextDrawHook = (void*)0x0467F14;
	DWORD * TextDrawHookCallback = (DWORD*)0x0467F19;
	DWORD * RandomVariable = (DWORD*)0x09CE7C8;
	// Hook at 00467F14
	// Callback hook 00467F19


	int MyPosY[] = { 126, 38, 32, 19, 32, 40, 50, 50, 60, 31 };

	void TextDraw(int PosX, int PosY, COLORREF TextColor, COLORREF SecondColor, char method, char padding, char absolute, char* Mensagem)
	{
		__asm
		{
			push MyPosY
			push Mensagem
			push absolute
			push padding
			push method
			push SecondColor
			push TextColor
			push PosY
			push PosX
			push offset DrawBaseAddrClass
			call DrawBaseFunc
		}
	}


	Naked void MyDrawFunc()
	{
		__asm
		{
			PUSHAD
			PUSHFD
		}
		
		TextDraw(50, 60, 0xFFFFFF, 0, 0, 0, 0, "Message: %03d");

		cout << "Draw Func is called!" << endl;
		__asm
		{
			POPFD
			POPAD
			MOV ECX, OFFSET MyPosY
			JMP TextDrawHookCallback
		}
	}

	bool InitializeTextDrawHook()
	{
		cout << "[+] Initializing TextDraw Hook" << endl;
		MMAPI::PlaceJMP((BYTE*)TextDrawHook, (DWORD)MyDrawFunc, 5);
		cout << "[+] TextDraw Hook Started !" << endl;
		return true;
	}
}


#include <LWPlatform/LWApplication.h>
#include <LWCore/LWAllocators/LWAllocator_Default.h>
#include <LWCore/LWAllocators/LWAllocator_LocalCircular.h>
#include <LWPlatform/LWThread.h>
#include <LWCore/LWTimer.h>
#include "ServerApp.h"
#include <chrono>

void NetworkSendThread(LWThread *T) {
	ServerApp *A = (ServerApp*)T->GetUserData();
	while (!(A->GetFlag()&ServerApp::Terminate)) {
		A->NetworkSend(LWTimer::GetCurrent());
		std::this_thread::sleep_for(std::chrono::milliseconds(1));
	}
	return;
}

void NetworkRecvThread(LWThread *T) {
	ServerApp *A = (ServerApp*)T->GetUserData();
	while (!(A->GetFlag()&ServerApp::Terminate)) {
		A->NetworkRecv(LWTimer::GetCurrent());
		std::this_thread::sleep_for(std::chrono::milliseconds(1));
	}
	return;
}

int LWMain(int argc, char **argv) {
	LWAllocator_Default Allocator;
	LWAllocator_LocalCircular InPackets(64 * 1024 * 1024); //64mb.
	LWAllocator_LocalCircular OutPackets(64 * 1024 * 1024);

	ServerApp *A = Allocator.Allocate<ServerApp>(Allocator, InPackets, OutPackets);
	LWThread SThread(NetworkSendThread, A);
	LWThread RThread(NetworkRecvThread, A);
	while (!(A->GetFlag()&ServerApp::Terminate)) {
		A->Update(LWTimer::GetCurrent());
		std::this_thread::sleep_for(std::chrono::milliseconds(1));
	}
	SThread.Join();
	RThread.Join();
	LWAllocator::Destroy(A);
	return 0;
}
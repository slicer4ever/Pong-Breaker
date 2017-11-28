#include <LWCore/LWAllocators/LWAllocator_Default.h>
#include <LWCore/LWAllocators/LWAllocator_LocalCircular.h>
#include <LWCore/LWTimer.h>
#include <LWPlatform/LWThread.h>
#include <LWPlatform/LWApplication.h>
#include "App.h"

LWAllocator *Allocator;
LWAllocator *InAllocator;
LWAllocator *OutAllocator;
App *A;
LWThread *UThread;
LWThread *NThread;

void NetworkThread(LWThread *T){
	LWRunLoop([](void *UserData)->bool {
		A->NetworkThread(LWTimer::GetCurrent());
		return (A->GetFlag()&App::Terminate) == 0;
	}, LWTimer::GetResolution() / 60, T->GetUserData());
}

void UpdateThread(LWThread *T) {
	LWRunLoop([](void *UserData)->bool {
		A->UpdateThread(LWTimer::GetCurrent());
		return (A->GetFlag()&App::Terminate) == 0;
	}, LWTimer::GetResolution() / 60, T->GetUserData());
	return;
}

int LWMain(int argc, char **argv) {
	Allocator = new LWAllocator_Default();
	InAllocator = new LWAllocator_LocalCircular(1024 * 64);
	OutAllocator = new LWAllocator_LocalCircular(1024 * 64);
	A = Allocator->Allocate<App>(*Allocator, *InAllocator, *OutAllocator);

	UThread = Allocator->Allocate<LWThread>(UpdateThread, nullptr);
	NThread = Allocator->Allocate<LWThread>(NetworkThread, nullptr);
	LWRunLoop([](void *UserData)->bool {
		uint64_t Current = LWTimer::GetCurrent();
		A->InputThread(Current).RenderThread(Current);
		bool Running = (A->GetFlag()&App::Terminate) == 0;
		if (Running) return Running;
		UThread->Join();
		NThread->Join();
		LWAllocator::Destroy(UThread);
		LWAllocator::Destroy(NThread);
		LWAllocator::Destroy(A);
		delete Allocator;
		delete InAllocator;
		delete OutAllocator;
		return Running;
	}, LWTimer::GetResolution() / 60, A);

	return 0;
}
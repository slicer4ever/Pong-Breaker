#include <LWCore/LWAllocators/LWAllocator_Default.h>
#include <LWCore/LWTimer.h>
#include <LWPlatform/LWThread.h>
#include <LWPlatform/LWApplication.h>
#include "App.h"

LWAllocator *Allocator;
App *A;
LWThread *UThread;

void UpdateThread(LWThread *T) {

	LWRunLoop([](void *UserData)->bool {
		A->UpdateThread(LWTimer::GetCurrent());
		return (A->GetFlag()&App::Terminate) == 0;
	}, LWTimer::GetResolution() / 60, T->GetUserData());
	return;
}

int LWMain(int argc, char **argv) {
	Allocator = new LWAllocator_Default();
	A = Allocator->Allocate<App>(*Allocator);

	UThread = Allocator->Allocate<LWThread>(UpdateThread, nullptr);
	LWRunLoop([](void *UserData)->bool {
		uint64_t Current = LWTimer::GetCurrent();
		A->InputThread(Current).RenderThread(Current);
		bool Running = (A->GetFlag()&App::Terminate) == 0;
		if (Running) return Running;
		UThread->Join();
		LWAllocator::Destroy(UThread);
		LWAllocator::Destroy(A);
		delete Allocator;
		return Running;
	}, LWTimer::GetResolution() / 60, A);

	return 0;
}
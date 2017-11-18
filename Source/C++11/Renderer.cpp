#include "Renderer.h"
#include <LWPlatform/LWWindow.h>
#include <LWEAsset.h>
#include <LWCore/LWMatrix.h>

Frame *Renderer::BeginFrame(void) {
	if (m_WriteFrame - m_ReadFrame >= MaxFrames) return nullptr;
	Frame *F = &m_Frames[m_WriteFrame%MaxFrames];
	F->m_UIFrame.m_Mesh->ClearUploadable();
	F->m_UIFrame.m_TextureCount = 0;
	F->m_GameMesh->ClearUploadable();
	return F;
}

void Renderer::FrameFinished(void) {
	m_WriteFrame++;
	return;
}

bool Renderer::WriteLine(Frame &F, const LWVector2f &Start, const LWVector2f &End, float Thickness, const LWVector4f &Color) {
	if (!F.m_GameMesh->CanWriteVertices(6)) return false;
	LWVertexUI *V = F.m_GameMesh->GetVertexAt(F.m_GameMesh->WriteVertices(6));
	LWVector2f Dir = End - Start;
	LWVector2f Perp = Dir.Normalize().Perpindicular()*Thickness;

	LWVector2f TopLeft = Start + Perp;
	LWVector2f BtmLeft = Start - Perp;
	LWVector2f TopRight = End + Perp;
	LWVector2f BtmRight = End - Perp;
	*(V + 0) = { LWVector4f(TopLeft, 0.0f, 1.0f), Color, LWVector4f(0.0f) };
	*(V + 1) = { LWVector4f(BtmLeft, 0.0f, 1.0f), Color, LWVector4f(0.0f) };
	*(V + 2) = { LWVector4f(BtmRight, 0.0f, 1.0f), Color, LWVector4f(0.0f) };
	*(V + 3) = { LWVector4f(TopLeft, 0.0f, 1.0f), Color, LWVector4f(0.0f) };
	*(V + 4) = { LWVector4f(BtmRight, 0.0f, 1.0f), Color, LWVector4f(0.0f) };
	*(V + 5) = { LWVector4f(TopRight, 0.0f, 1.0f), Color, LWVector4f(0.0f) };

	return true;
}

bool Renderer::WriteCircle(Frame &F, const LWVector2f &Position, float Radius, uint32_t Steps, const LWVector4f &Color) {
	if (!F.m_GameMesh->CanWriteVertices(Steps * 3)) return false;
	LWVertexUI *V = F.m_GameMesh->GetVertexAt(F.m_GameMesh->WriteVertices(Steps * 3));
	uint32_t o = 0;
	float ThetaInc = LW_2PI / Steps;
	LWVector2f Prev = LWVector2f::MakeTheta(0.0f)*Radius + Position;
	for (uint32_t i = 1; i <= Steps; i++) {
		LWVector2f Curr = LWVector2f::MakeTheta(ThetaInc*(float)i)*Radius + Position;
		*(V + o++) = { LWVector4f(Position, 0.0f, 1.0f), Color, LWVector4f(0.0f) };
		*(V + o++) = { LWVector4f(Prev, 0.0f, 1.0f), Color, LWVector4f(0.0f) };
		*(V + o++) = { LWVector4f(Curr, 0.0f, 1.0f), Color, LWVector4f(0.0f) };
		Prev = Curr;
	}
	return true;
}

bool Renderer::WriteOutlinedCircle(Frame &F, const LWVector2f &Position, float Radius, float Thickness, uint32_t Steps, const LWVector4f &Color) {
	if (!F.m_GameMesh->CanWriteVertices(Steps * 6)) return false;
	float ThetaInc = LW_2PI / Steps;
	LWVector2f Prev = LWVector2f::MakeTheta(0.0f)*Radius + Position;
	for (uint32_t i = 1; i <= Steps; i++) {
		LWVector2f Curr = LWVector2f::MakeTheta(ThetaInc*(float)i)*Radius + Position;
		WriteLine(F, Prev, Curr, Thickness, Color);
		Prev = Curr;
	}
	return true;
}

Renderer &Renderer::UpdateFrame(Frame &F) {
	F.m_UIFrame.m_Mesh->MarkUploadable();
	F.m_GameMesh->MarkUploadable();
	return *this;
}

Renderer &Renderer::DrawFrame(Frame &F) {
	m_Driver->DrawMesh(m_UIColorShader, LWVideoDriver::Triangle, F.m_GameMesh);
	DrawUIFrame(F.m_UIFrame);
	return *this;
}

Renderer &Renderer::DrawUIFrame(LWEUIFrame &F) {
	uint32_t o = 0;
	for (uint32_t i = 0; i < F.m_TextureCount; i++) {
		LWShader *Sdr = (F.m_FontTexture[i] ? m_FontShader : (F.m_Textures[i] ? m_UITexShader : m_UIColorShader));
		Sdr->SetTexture(0, F.m_Textures[i]);
		m_Driver->DrawMesh(Sdr, LWVideoDriver::Triangle, F.m_Mesh, F.m_VertexCount[i], o);
		o += F.m_VertexCount[i];
	}
	return *this;
}

Renderer &Renderer::SizeChanged(LWWindow *Window) {
	LWVector2f WndSize = Window->GetSizef();
	LWMatrix4f Ortho = LWMatrix4f::Ortho(0.0f, WndSize.x, 0.0f, WndSize.y, 0.0f, 1.0f);
	memcpy(m_UIData->GetLocalBuffer(), &Ortho, sizeof(LWMatrix4f));
	m_UIData->SetEditLength(sizeof(LWMatrix4f)).MarkUpdated();
	m_Driver->ViewPort();
	m_SizeChanged = false;
	return *this;
}

Renderer &Renderer::Render(LWWindow *Window) {
	//Size changed!
	if (Window->GetFlag()&LWWindow::SizeChanged) m_SizeChanged = true;
	if (!m_Driver->Update()) return *this;
	if (m_SizeChanged) SizeChanged(Window);

	if (!m_WriteFrame) return *this;
	if (m_WriteFrame != m_ReadFrame) {
		UpdateFrame(m_Frames[m_ReadFrame%MaxFrames]);
		m_ReadFrame++;
	}

	m_Driver->Clear(LWVideoDriver::Color, 0xFF, 0.0f, 0);
	DrawFrame(m_Frames[(m_ReadFrame - 1) % MaxFrames]);
	m_Driver->Present(nullptr);
	return *this;
}

Renderer::Renderer(LWVideoDriver *Driver, LWEAssetManager *Manager, LWAllocator &Allocator) : m_Driver(Driver), m_ReadFrame(0), m_WriteFrame(0), m_SizeChanged(true) {

	for (uint32_t i = 0; i < MaxFrames; i++) {
		m_Frames[i].m_UIBuffer = m_Driver->CreateVideoBuffer(LWVideoBuffer::Vertex, sizeof(LWVertexUI)*MaxUIRects * 6, Allocator, LWVideoBuffer::WriteDiscardable | LWVideoBuffer::LocalCopy, nullptr);
		m_Frames[i].m_UIFrame.m_Mesh = LWVertexUI::MakeMesh(Allocator, m_Frames[i].m_UIBuffer, 0);
		m_Frames[i].m_GameBuffer = m_Driver->CreateVideoBuffer(LWVideoBuffer::Vertex, sizeof(LWVertexUI)*MaxGameRects * 6, Allocator, LWVideoBuffer::WriteDiscardable | LWVideoBuffer::LocalCopy, nullptr);
		m_Frames[i].m_GameMesh = LWVertexUI::MakeMesh(Allocator, m_Frames[i].m_GameBuffer, 0);
	}

	m_UIData = m_Driver->CreateVideoBuffer(LWVideoBuffer::Uniform, sizeof(LWMatrix4f), Allocator, LWVideoBuffer::WriteDiscardable | LWVideoBuffer::LocalCopy, nullptr);

	m_VideoState = m_Driver->CreateVideoState(LWVideoState::CULL_CW | LWVideoState::BLENDING, LWVideoState::BLEND_SRC_ALPHA, LWVideoState::BLEND_ONE_MINUS_SRC_ALPHA, LWVideoState::LESS_EQL, Allocator);

	m_UITexShader = Manager->GetAsset<LWShader>("UITextureShader");
	m_UIColorShader = Manager->GetAsset<LWShader>("UIColorShader");
	m_FontShader = Manager->GetAsset<LWShader>("FontShader");

	m_UITexShader->SetUniformBlock(LWSHADER_BLOCKUNIFORM, m_UIData);
	m_UIColorShader->SetUniformBlock(LWSHADER_BLOCKUNIFORM, m_UIData);
	m_FontShader->SetUniformBlock(LWSHADER_BLOCKUNIFORM, m_UIData);

	m_Driver->SetVideoState(m_VideoState);
}

Renderer::~Renderer() {
	for (uint32_t i = 0; i < MaxFrames; i++) {
		LWAllocator::Destroy(m_Frames[i].m_UIFrame.m_Mesh);
		m_Driver->DestroyVideoBuffer(m_Frames[i].m_UIBuffer);
		LWAllocator::Destroy(m_Frames[i].m_GameMesh);
		m_Driver->DestroyVideoBuffer(m_Frames[i].m_GameBuffer);
	}
	LWVideoDriver::DestroyVideoDriver(m_Driver);
}
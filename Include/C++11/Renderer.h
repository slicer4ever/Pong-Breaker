#ifndef RENDERER_H
#define RENDERER_H
#include <LWVideo/LWVideoDriver.h>
#include <LWEUIManager.h>
struct Frame {
	LWEUIFrame m_UIFrame;
	LWVideoBuffer *m_UIBuffer;
	LWMesh<LWVertexUI> *m_GameMesh;
	LWVideoBuffer *m_GameBuffer;

};

class Renderer {
public:
	enum {
		MaxFrames = 3,
		MaxUIRects = 2048,
		MaxGameRects = 2048
	};

	Frame *BeginFrame(void);

	void FrameFinished(void);

	bool WriteLine(Frame &F, const LWVector2f &Start, const LWVector2f &End, float Thickness, const LWVector4f &Color);

	bool WriteCircle(Frame &F, const LWVector2f &Position, float Radius, uint32_t Steps, const LWVector4f &Color);

	bool WriteOutlinedCircle(Frame &F, const LWVector2f &Position, float Radius, float Thickness, uint32_t Steps,  const LWVector4f &Color);

	Renderer &UpdateFrame(Frame &F);

	Renderer &DrawFrame(Frame &F);

	Renderer &DrawUIFrame(LWEUIFrame &F);

	Renderer &SizeChanged(LWWindow *Window);

	Renderer &Render(LWWindow *Window);

	Renderer(LWVideoDriver *Driver, LWEAssetManager *AssetMan, LWAllocator &Allocator);

	~Renderer();
private:
	LWVideoDriver *m_Driver;
	Frame m_Frames[MaxFrames];
	LWVideoBuffer *m_UIData;
	LWVideoState *m_VideoState;
	LWShader *m_UIColorShader;
	LWShader *m_UITexShader;
	LWShader *m_FontShader;
	uint32_t m_ReadFrame;
	uint32_t m_WriteFrame;
	bool m_SizeChanged;

};

#endif
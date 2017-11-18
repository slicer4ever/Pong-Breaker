#ifndef GAME_H
#define GAME_H
#include <LWCore/LWVector.h>
#include "Types.h"

class GameObject {
public:
	enum {
		Circle,
		Rect,
		ShapeBits=0x1,

		Ball=0,
		GoalA,
		GoalB,
		BlockerA,
		BlockerB
	};
	static const uint32_t DefaultHealth = 2;

	static bool CheckLine(GameObject &A, const LWVector2f &LineStart, const LWVector2f &LineEnd, LWVector2f *OutPnt, LWVector2f *OutNormal);

	static bool CheckLineCircle(const LWVector2f &LineStart, const LWVector2f &LineEnd, const LWVector2f &Ctrpnt, float Radius, LWVector2f *OutPnt, LWVector2f *OutNormal);

	static bool CheckLineRect(const LWVector2f &LineStart, const LWVector2f &LineEnd, const LWVector2f &CtrPnt, const LWVector2f &Size, float Theta, LWVector2f *OutPnt, LWVector2f *OutNormal);

	static bool CheckRectRect(GameObject &RectA, GameObject &RectB, LWVector2f *OutVec);

	static bool CheckRectCircle(GameObject &RectA, GameObject &CircleB, LWVector2f *OutVec);

	static bool CheckCircleCicle(GameObject &CircleA, GameObject &CircleB, LWVector2f *OutVec);

	static bool CheckCollision(GameObject &A, GameObject &B, LWVector2f *OutVec);

	GameObject &SetIndex(uint32_t Index);

	GameObject &SetPosition(const LWVector2f &Position);

	GameObject &SetVelocity(const LWVector2f &Velocity);

	GameObject &Update(const LWVector2f &FieldSize);

	GameObject &Draw(const LWVector2f &ScreenCoord, float Scale, Frame *F, Renderer *R, const LWVector4f &Color);

	GameObject &AddHealth(int32_t Health);

	LWVector2f GetPosition(void) const;

	LWVector2f GetSize(void) const;

	LWVector2f GetVelocity(void) const;

	float GetTheta(void) const;

	uint32_t GetFlag(void) const;

	uint32_t GetIndex(void) const;

	uint32_t GetID(void) const;

	int32_t GetHealth(void) const;

	GameObject(const LWVector2f &Position, const LWVector2f &Velocity, const LWVector2f &Size, float Theta, uint32_t ID, uint32_t Flag);

	GameObject();

private:
	LWVector2f m_Position;
	LWVector2f m_Velocity;
	LWVector2f m_Size;
	float m_Theta;
	uint32_t m_ID;
	uint32_t m_Flag;
	uint32_t m_Index;
	int32_t m_Health;

};

class Game {
public:
	enum {
		MaxObjects = 64,
		BaseBlockers=2,
	};	
	static const float BallSpeed;
	static const LWVector2f BlockSize;

	bool Update(App *A);

	void DrawFrame(Frame *F, Renderer *R, const LWVector2f &WndSize);

	float GetViewScale(const LWVector2f &WndSize);

	LWVector2f MapScreen(const LWVector2f &ScreenCoord, const LWVector2f &WndSize);

	LWVector2f MapGame(const LWVector2f &GameCoord, const LWVector2f &WndSize);

	Game &ResetRound(void);

	Game &Initiate(const LWVector2f &FieldSize);

	Game &SpawnBlocker(const LWVector2f &Position, float Theta, uint32_t BlockerID);

	GameObject *GetNextGameObject(GameObject &Obj);

	Game &ReleaseGameObject(GameObject *Obj);

	GameObject *GetBall(void);

	GameObject *TestRay(const LWVector2f &Start, const LWVector2f &End, LWVector2f *OutPnt, LWVector2f *OutNormal);

	GameObject *TestRay(const LWVector2f &Start, const LWVector2f &End, GameObject *SimulatedObjs, uint32_t SimulatedCnt, LWVector2f *OutPnt, LWVector2f *OutNormal);

	uint32_t GetScoreA(void) const;

	uint32_t GetScoreB(void) const;

	uint32_t GetBlocksA(void) const;

	uint32_t GetBlocksB(void) const;

	LWVector2f GetFieldSize(void) const;

	bool Finished(void) const;

	Game();
private:

	GameObject m_ObjectPool[MaxObjects];
	GameObject *m_ObjectList[MaxObjects];
	uint32_t m_ObjectCount;
	uint32_t m_ScoreA;
	uint32_t m_ScoreB;
	uint32_t m_ABlocks;
	uint32_t m_BBlocks;
	LWVector2f m_FieldSize;

};

#endif
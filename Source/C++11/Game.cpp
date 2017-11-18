#include "Game.h"
#include "App.h"
#include "Renderer.h"
#include <algorithm>

const float Game::BallSpeed = 7.5f;
const LWVector2f Game::BlockSize = LWVector2f(15.0f, 50.0f);

bool GameObject::CheckLine(GameObject &A, const LWVector2f &LineStart, const LWVector2f &LineEnd, LWVector2f *OutPnt, LWVector2f *OutNormal) {
	if ((A.GetFlag()&GameObject::ShapeBits) == GameObject::Circle) return CheckLineCircle(LineStart, LineEnd, A.GetPosition(), A.GetSize().x, OutPnt, OutNormal);
	return CheckLineRect(LineStart, LineEnd, A.GetPosition(), A.GetSize(), A.GetTheta(), OutPnt, OutNormal);
}

bool GameObject::CheckLineCircle(const LWVector2f &LineStart, const LWVector2f &LineEnd, const LWVector2f &Ctrpnt, float Radius, LWVector2f *OutPnt, LWVector2f *OutNormal) {
	LWVector2f LineDir = LineEnd - LineStart;
	LWVector2f CircNrm = (LineDir.Perpindicular().Normalize());

	LWVector2f Dir = Ctrpnt - LineStart;
	float D = Dir.Dot(CircNrm);
	if (D > Radius || D < -Radius) return false;
	LWVector2f LinePnt = Ctrpnt - CircNrm*D;
	float N = sqrtf(Radius*Radius-D*D);
	LWVector2f LinePntA = LinePnt - CircNrm.Perpindicular()*N;
	LWVector2f LinePntB = LinePnt + CircNrm.Perpindicular()*N;

	LWVector2f LinePosA = LinePntA - LineStart;
	LWVector2f LinePosB = LinePntB - LineStart;
	float DotA = LinePosA.Dot(LineDir);
	float DotB = LinePosB.Dot(LineDir);
	if (OutPnt) *OutPnt = DotA < DotB ? LinePntA : LinePntB;
	if (OutNormal) *OutNormal = DotA < DotB ? (LinePntA- Ctrpnt).Normalize() : (LinePntB- Ctrpnt).Normalize();
	return (DotA >= 0.0f && DotA <= LineDir.LengthSquared()) || (DotB >= 0.0f && DotB <= LineDir.LengthSquared());
}

bool GameObject::CheckLineRect(const LWVector2f &LineStart, const LWVector2f &LineEnd, const LWVector2f &CtrPnt, const LWVector2f &Size, float Theta, LWVector2f *OutPnt, LWVector2f *OutNormal) {
	LWVector2f LineDir = LineEnd - LineStart;
	LWVector2f Rot = LWVector2f::MakeTheta(Theta);
	LWVector3f Planes[] = { LWVector3f(-1.0f, 0.0f, Size.x), LWVector3f(1.0f, 0.0f, Size.x), LWVector3f(0.0f, 1.0f, Size.y), LWVector3f(0.0f, -1.0f, Size.y) };

	auto TestPnt = [](LWVector2f &Pnt, const LWVector3f *Planes, uint32_t PlaneCnt)->bool {
		for (uint32_t i = 0; i < PlaneCnt; i++) {
			float d = Planes[i].x*Pnt.x + Planes[i].y*Pnt.y - Planes[i].z;
			if (d > 0.01f) return false;
		}
		return true;
	};

	LineDir = LWVector2f(LineDir.x*Rot.x - LineDir.y*Rot.y, LineDir.x*Rot.y + LineDir.y*Rot.x);
	LWVector2f LineDirN = LineDir.Normalize();

	float LineLen = LineDir.Length();
	bool Res = false;
	float LowestD = LineLen +1.0f;
	LWVector2f LowestPnt;
	LWVector2f LineRel = LineStart - CtrPnt;
	LineRel = LWVector2f(LineRel.x*Rot.x - LineRel.y*Rot.y, LineRel.x*Rot.y + LineRel.y*Rot.x);
	uint32_t LowestPlane = 0;
	for (uint32_t i = 0; i < 4; i++) {
		float D = LineRel.x*Planes[i].x + LineRel.y*Planes[i].y - Planes[i].z;
		float DirD = fabs(LineDirN.Dot(LWVector2f(Planes[i].x, Planes[i].y)));
		if(DirD<=std::numeric_limits<float>::epsilon()) continue;
		D = D / DirD;
		LWVector2f LinePnt = LineRel+LineDirN*D;
		//Check point is inside our planes.
		if (!TestPnt(LinePnt, Planes, 4)) continue;
		if(D<0.0f || D>LineLen) continue;
		if (D < LowestD) {
			LowestPnt = LinePnt-LineRel;
			LowestPlane = i;
			LowestD = D;
		}
		Res = true;
	}
		
	if (Res && OutPnt) *OutPnt = LWVector2f(LowestPnt.x*Rot.x + LowestPnt.y*Rot.y, -LowestPnt.x*Rot.y + LowestPnt.y*Rot.x) + LineStart;
	if (Res && OutNormal) {
		*OutNormal = LWVector2f(Planes[LowestPlane].x, Planes[LowestPlane].y);
		*OutNormal = LWVector2f((*OutNormal).x*Rot.x + (*OutNormal).y*Rot.y, -(*OutNormal).x*Rot.y + (*OutNormal).y*Rot.x);
	}
	return Res;
}


bool GameObject::CheckRectRect(GameObject &RectA, GameObject &RectB, LWVector2f *OutVec) {
	return false;
}

bool GameObject::CheckRectCircle(GameObject &RectA, GameObject &CircleB, LWVector2f *OutVec) {

	LWVector2f Rot = LWVector2f::MakeTheta(RectA.GetTheta());
	LWVector2f Size = RectA.GetSize();
	LWVector3f Planes[] = { LWVector3f(-1.0f, 0.0f, Size.x), LWVector3f(1.0f, 0.0f, Size.x), LWVector3f(0.0f, 1.0f, Size.y), LWVector3f(0.0f, -1.0f, Size.y) };

	LWVector2f Dir = CircleB.GetPosition() - RectA.GetPosition();
	Dir = LWVector2f(Dir.x*Rot.x - Dir.y*Rot.y, Dir.x*Rot.y + Dir.y*Rot.x);
	
	float LowestD = -1000.0f;
	LWVector3f LowestPlane = LWVector3f(0.0f);
	for (uint32_t i = 0; i < 4; i++) {
		float D = Planes[i].x*Dir.x+Planes[i].y*Dir.y-Planes[i].z;
		if (D > CircleB.GetSize().x) return false;
		if (D > LowestD) {
			LowestD = D;
			LowestPlane = Planes[i];
		}
	}
	if (OutVec) {
		LWVector2f Plane = LWVector2f(LowestPlane.x*Rot.x + LowestPlane.y*Rot.y, -LowestPlane.x*Rot.y + LowestPlane.y*Rot.x);
		*OutVec = Plane*(LowestD - CircleB.GetSize().x);
		//*OutVec = LWVector2f();
	}

	return true;
}

bool GameObject::CheckCircleCicle(GameObject &CircleA, GameObject &CircleB, LWVector2f *OutVec) {
	LWVector2f Dir = CircleB.GetPosition() - CircleA.GetPosition();
	float Len = Dir.Length();
	LWVector2f DirN = Dir/Len;
	float r = CircleA.GetSize().x + CircleB.GetSize().y;
	if (OutVec) *OutVec = DirN*(Len - r);
	return Len <= r;
}


bool GameObject::CheckCollision(GameObject &A, GameObject &B, LWVector2f *OutVec) {
	uint32_t ShapeA = A.GetFlag()&ShapeBits;
	uint32_t ShapeB = B.GetFlag()&ShapeBits;

	if (ShapeA == Circle) {
		if (ShapeB == Circle) return CheckCircleCicle(A, B, OutVec);
		bool Res = CheckRectCircle(B, A, OutVec);
		if (OutVec) *OutVec = -*OutVec;
		return Res;
	}
	if (ShapeB == Circle) return CheckRectCircle(A, B, OutVec);
	return CheckRectRect(A, B, OutVec);
}

GameObject &GameObject::SetIndex(uint32_t Index) {
	m_Index = Index;
	return *this;
}

GameObject &GameObject::Update(const LWVector2f &FieldSize) {
	m_Position += m_Velocity;
	LWVector2f Size = m_Size;
	if ((m_Flag&ShapeBits) == Circle) Size.y = m_Size.x;
	if (m_Position.x + m_Size.x >= FieldSize.x) m_Velocity.x *= -1.0f;
	if (m_Position.x - m_Size.x < 0.0f) m_Velocity.x *= -1.0f;
	if (m_Position.y + m_Size.y >= FieldSize.y) m_Velocity.y *= -1.0f;
	if (m_Position.y - m_Size.y < 0.0f) m_Velocity.y *= -1.0f;
	return *this;
}

GameObject &GameObject::Draw(const LWVector2f &ScreenCoord, float Scale, Frame *F, Renderer *R, const LWVector4f &Color) {
	if ((m_Flag&ShapeBits) == Circle) R->WriteCircle(*F, ScreenCoord, m_Size.x*Scale, 24, Color);
	else {
		const float Thickness = 1.0f;
		LWVector2f Rot = LWVector2f::MakeTheta(m_Theta);
		LWVector2f TopLeft = LWVector2f(-m_Size.x, m_Size.y)*Scale;
		LWVector2f BtmLeft = LWVector2f(-m_Size.x, -m_Size.y)*Scale;
		LWVector2f TopRight = LWVector2f(m_Size.x, m_Size.y)*Scale;
		LWVector2f BtmRight = LWVector2f(m_Size.x, -m_Size.y)*Scale;

		TopLeft = LWVector2f(TopLeft.x*Rot.x + TopLeft.y*Rot.y, -TopLeft.x*Rot.y + TopLeft.y*Rot.x)+ ScreenCoord;
		TopRight = LWVector2f(TopRight.x*Rot.x + TopRight.y*Rot.y, -TopRight.x*Rot.y + TopRight.y*Rot.x)+ ScreenCoord;
		BtmLeft = LWVector2f(BtmLeft.x*Rot.x + BtmLeft.y*Rot.y, -BtmLeft.x*Rot.y + BtmLeft.y*Rot.x)+ ScreenCoord;
		BtmRight = LWVector2f(BtmRight.x*Rot.x + BtmRight.y*Rot.y, -BtmRight.x*Rot.y + BtmRight.y*Rot.x)+ ScreenCoord;


		R->WriteLine(*F, TopLeft, TopRight, Thickness, Color);
		R->WriteLine(*F, TopRight, BtmRight, Thickness, Color);
		R->WriteLine(*F, BtmLeft, BtmRight, Thickness, Color);
		R->WriteLine(*F, TopLeft, BtmLeft, Thickness, Color);
	}
	return *this;
}


GameObject &GameObject::SetPosition(const LWVector2f &Position) {
	m_Position = Position;
	return *this;
}

GameObject &GameObject::SetVelocity(const LWVector2f &Velocity) {
	m_Velocity = Velocity;
	return *this;
}

GameObject &GameObject::AddHealth(int32_t Health) {
	m_Health+=Health;
	return *this;
}

LWVector2f GameObject::GetPosition(void) const{
	return m_Position;
}

LWVector2f GameObject::GetSize(void) const{
	return m_Size;
}

LWVector2f GameObject::GetVelocity(void) const {
	return m_Velocity;
}

float GameObject::GetTheta(void) const {
	return m_Theta;
}

uint32_t GameObject::GetFlag(void) const {
	return m_Flag;
}

uint32_t GameObject::GetIndex(void) const {
	return m_Index;
}

uint32_t GameObject::GetID(void) const {
	return m_ID;
}

int32_t GameObject::GetHealth(void) const {
	return m_Health;
}

GameObject::GameObject(const LWVector2f &Position, const LWVector2f &Velocity, const LWVector2f &Size, float Theta, uint32_t ID, uint32_t Flag) : m_Position(Position), m_Velocity(Velocity), m_Size(Size), m_Theta(Theta), m_Flag(Flag), m_Health(DefaultHealth), m_ID(ID) {}

GameObject::GameObject() {}


bool Game::Update(App *A) {
	for (uint32_t i = 0; i < m_ObjectCount; i++) m_ObjectList[i]->Update(m_FieldSize);

	for (uint32_t i = 1; i < m_ObjectCount; i++) { //We only care about what our ball hits!
		LWVector2f OutVec;
		if (GameObject::CheckCollision(*m_ObjectList[0], *m_ObjectList[i], &OutVec)) {
			m_ObjectList[0]->SetPosition(m_ObjectList[0]->GetPosition() + OutVec);
			OutVec = OutVec.Normalize();
			float D = m_ObjectList[0]->GetVelocity().Dot(OutVec);
			m_ObjectList[0]->SetVelocity((m_ObjectList[0]->GetVelocity() - OutVec*D*2.0f).Normalize()*BallSpeed);
			if (m_ObjectList[i]->GetID() == GameObject::GoalA) {
				m_ScoreB++;
				return true;
			}
			if (m_ObjectList[i]->GetID() == GameObject::GoalB) {
				m_ScoreA++;
				return true;
			}
			if (m_ObjectList[i]->AddHealth(-1).GetHealth() <= 0) {
				ReleaseGameObject(m_ObjectList[i]);
				i--;
			}
		}
	}
	return false;
}

void Game::DrawFrame(Frame *F, Renderer *R, const LWVector2f &WndSize) {
	LWVector2f WndScale = WndSize / m_FieldSize;
	float Scale = std::min<float>(WndScale.x, WndScale.y);
	LWVector2f Offset = LWVector2f(WndSize.x - (m_FieldSize.x*Scale), WndSize.y - (m_FieldSize.y*Scale))*0.5f;
	LWVector4f IDClrs[] = { LWVector4f(1.0f, 0.647f, 0.0f, 1.0f), LWVector4f(0.8f, 0.0f, 0.0f, 1.0f), LWVector4f(0.0f, 0.0f, 0.8f, 1.0f), LWVector4f(1.0f, 0.0f, 0.0f, 1.0f), LWVector4f(0.0f, 0.0f, 1.0f, 1.0f) };
	for (uint32_t i = 0; i < m_ObjectCount; i++) {
		float Fade = (float)m_ObjectList[i]->GetHealth() / (float)GameObject::DefaultHealth;
		LWVector4f Clr = IDClrs[m_ObjectList[i]->GetID()];
		Clr.w *= Fade;
		m_ObjectList[i]->Draw(m_ObjectList[i]->GetPosition()*Scale+Offset, Scale, F, R, Clr);
	}
}

float Game::GetViewScale(const LWVector2f &WndSize) {
	LWVector2f WndScale = WndSize / m_FieldSize;
	return std::min<float>(WndScale.x, WndScale.y);
}

LWVector2f Game::MapScreen(const LWVector2f &ScreenCoord, const LWVector2f &WndSize) {
	LWVector2f WndScale = WndSize / m_FieldSize;
	float Scale = std::min<float>(WndScale.x, WndScale.y);
	LWVector2f Offset = LWVector2f(WndSize.x - (m_FieldSize.x*Scale), WndSize.y - (m_FieldSize.y*Scale))*0.5f;
	Scale = 1.0f / Scale;
	return (ScreenCoord - Offset)*Scale;
}

LWVector2f Game::MapGame(const LWVector2f &GameCoord, const LWVector2f &WndSize) {
	LWVector2f WndScale = WndSize / m_FieldSize;
	float Scale = std::min<float>(WndScale.x, WndScale.y);
	LWVector2f Offset = LWVector2f(WndSize.x - (m_FieldSize.x*Scale), WndSize.y - (m_FieldSize.y*Scale))*0.5f;
	return GameCoord*Scale + Offset;
}

Game &Game::ResetRound(void) {
	m_ObjectCount = 0;
	m_ABlocks = BaseBlockers;
	m_BBlocks = BaseBlockers;
	float Theta = 0.0f;
	if ((m_ScoreA + m_ScoreB) & 1) Theta = LW_PI;
	GameObject BallDesc = GameObject(LWVector2f(m_FieldSize*0.5f), LWVector2f::MakeTheta(Theta)*BallSpeed, LWVector2f(10.0f), 0.0f, GameObject::Ball, GameObject::Circle);
	GameObject GoalADesc = GameObject(LWVector2f(0.0f, m_FieldSize.y*0.5f), LWVector2f(), LWVector2f(5.0f, m_FieldSize.y*0.5f), 0.0f, GameObject::GoalA, GameObject::Rect);
	GameObject GoalBDesc = GameObject(LWVector2f(m_FieldSize.x, m_FieldSize.y*0.5f), LWVector2f(), LWVector2f(5.0f, m_FieldSize.y*0.5f), 0.0f, GameObject::GoalB, GameObject::Rect);

	GetNextGameObject(BallDesc);
	GetNextGameObject(GoalADesc);
	GetNextGameObject(GoalBDesc);
	return *this;
}

Game &Game::Initiate(const LWVector2f &FieldSize) {
	m_FieldSize = FieldSize;
	m_ScoreA = 0;
	m_ScoreB = 0;
	ResetRound();
	return *this;
}

Game &Game::SpawnBlocker(const LWVector2f &Position, float Theta, uint32_t BlockerID) {
	GameObject Desc = GameObject(Position, LWVector2f(0.0f), Game::BlockSize, Theta, BlockerID, GameObject::Rect);
	if (BlockerID == GameObject::BlockerA && m_ABlocks>0) {
		GetNextGameObject(Desc);
		m_ABlocks--;
		m_BBlocks++;
	} else if (BlockerID == GameObject::BlockerB && m_BBlocks>0) {
		GetNextGameObject(Desc);
		m_BBlocks--;
		m_ABlocks++;
	}
	return *this;
}

GameObject *Game::GetNextGameObject(GameObject &Obj) {
	if (m_ObjectCount >= MaxObjects) return nullptr;
	*m_ObjectList[m_ObjectCount] = Obj;
	m_ObjectList[m_ObjectCount]->SetIndex(m_ObjectCount);
	m_ObjectCount++;
	return &m_ObjectPool[m_ObjectCount-1];
}

Game &Game::ReleaseGameObject(GameObject *Obj) {
	uint32_t TIndex = Obj->GetIndex();
	uint32_t CIndex = m_ObjectList[m_ObjectCount - 1]->GetIndex();

	m_ObjectList[TIndex] = m_ObjectList[m_ObjectCount - 1];
	m_ObjectList[CIndex] = Obj;
	m_ObjectList[TIndex]->SetIndex(TIndex);
	m_ObjectList[CIndex]->SetIndex(CIndex);
	m_ObjectCount--;
	return *this;
}

GameObject *Game::GetBall(void) {
	return m_ObjectList[0];
}

GameObject *Game::TestRay(const LWVector2f &Start, const LWVector2f &End, LWVector2f *OutPnt, LWVector2f *OutNormal) {
	GameObject *ResObj = nullptr;
	LWVector2f Res = End;
	LWVector2f ResNormal;
	for (uint32_t i = 1; i < m_ObjectCount; i++) {
		LWVector2f TRes;
		LWVector2f TResNormal;
		if (GameObject::CheckLine(*m_ObjectList[i], Start, Res, &TRes, &TResNormal)) {
			Res = TRes;
			ResNormal = TResNormal;
			ResObj = m_ObjectList[i];
		}
	}
	if (OutPnt) *OutPnt = Res;
	if (OutNormal) *OutNormal = ResNormal;
	return ResObj;
}

GameObject *Game::TestRay(const LWVector2f &Start, const LWVector2f &End, GameObject *SimulatedObjs, uint32_t SimulatedCnt, LWVector2f *OutPnt, LWVector2f *OutNormal) {
	GameObject *ResObj = nullptr;
	LWVector2f Res = End;
	LWVector2f ResNormal;
	for (uint32_t i = 1; i < m_ObjectCount; i++) {
		LWVector2f TRes;
		LWVector2f TResNormal;
		if (GameObject::CheckLine(*m_ObjectList[i], Start, Res, &TRes, &TResNormal)) {
			Res = TRes;
			ResNormal = TResNormal;
			ResObj = m_ObjectList[i];
		}
	}
	for (uint32_t i = 0; i < SimulatedCnt; i++) {
		LWVector2f TRes;
		LWVector2f TResNormal;
		if (GameObject::CheckLine(SimulatedObjs[i], Start, Res, &TRes, &TResNormal)) {
			Res = TRes;
			ResNormal = TResNormal;
			ResObj = SimulatedObjs + i;
		}
	}
	if (OutPnt) *OutPnt = Res;
	if (OutNormal) *OutNormal = ResNormal;
	return ResObj;
}

uint32_t Game::GetScoreA(void) const {
	return m_ScoreA;
}

uint32_t Game::GetScoreB(void) const {
	return m_ScoreB;
}

uint32_t Game::GetBlocksA(void) const {
	return m_ABlocks;
}

uint32_t Game::GetBlocksB(void) const {
	return m_BBlocks;
}

LWVector2f Game::GetFieldSize(void) const {
	return m_FieldSize;
}

bool Game::Finished(void) const {
	return m_ScoreA == 5 || m_ScoreB == 5;
}

Game::Game() : m_ScoreA(0), m_ScoreB(0) {
	for (uint32_t i = 0; i < MaxObjects; i++) m_ObjectList[i] = &m_ObjectPool[i];
}
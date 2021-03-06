#include "stdafx.h"
#include "SceneBase.h"


SceneBase::SceneBase()
	: pCamera(NULL), pPostEffect(NULL), pDirLight(NULL), pDirLightCamera(NULL), isWireFrame(false),
	nPassNumber(0), fBloomIntensity(0.2f)
{
	pCamera = new Camera(); //메인카메라 생성
	pCamera->ReadyRenderToTexture(4096, 4096); //렌더타겟 맵 세팅

	pPostEffect = NULL; //나중에 세팅!

	pDirLight = new DirectionLight(); //방향성 광원 세팅
	pDirLight->Color = D3DXCOLOR(1, 1, 1, 1);
	pDirLight->Intensity = 1.0f;

	pDirLightCamera = new Camera; //방향성 광원의 방향을 가진 빛 카메라

	this->pDirLightCamera->bOrtho = true; //직교투영으로 바꾼다.
	this->pDirLightCamera->camNear = 0.1f; //카메라 가까운거리
	this->pDirLightCamera->camFar = 300.0f; //카메라 먼거리
	this->pDirLightCamera->aspect = 1.0f; //종횡비
	this->pDirLightCamera->orthoSize = 600.0f; //투영크기는 그림자 크기로

	this->pDirLightCamera->ReadyShadowToTexture(8192); //방향성 카메라의 그림자 렌더타겟 텍스쳐 준비
}


SceneBase::~SceneBase()
{
}

HRESULT SceneBase::Init()
{
	if (FAILED(this->Scene_Init()))
		return E_FAIL;

	this->pCamera->ReadyRenderToTexture(WINSIZE_X, WINSIZE_Y); //메인카메라의 렌더타겟 텍스쳐 준비

	//0----1
	//|   /|
	//|  / |
	//| /  |
	//|/   |
	//3----2

	//투영좌표 포지션
	this->stSceneVertex[0].Position = D3DXVECTOR3(-1.0f, 1.0f, 0.0f);
	this->stSceneVertex[1].Position = D3DXVECTOR3( 1.0f, 1.0f, 0.0f);
	this->stSceneVertex[2].Position = D3DXVECTOR3( 1.0f, -1.0f, 0.0f);
	this->stSceneVertex[3].Position = D3DXVECTOR3(-1.0f, -1.0f, 0.0f);

	//UV 좌표
	this->stSceneVertex[0].UV = D3DXVECTOR2(0.0f,0.0f);
	this->stSceneVertex[1].UV = D3DXVECTOR2(1.0f,0.0f);
	this->stSceneVertex[2].UV = D3DXVECTOR2(1.0f,1.0f);
	this->stSceneVertex[3].UV = D3DXVECTOR2(0.0f,1.0f);

	//인덱스
	this->dwSceneIndex[0] = 0;
	this->dwSceneIndex[1] = 1;
	this->dwSceneIndex[2] = 3;
	this->dwSceneIndex[3] = 3;
	this->dwSceneIndex[4] = 1;
	this->dwSceneIndex[5] = 2;

	this->pPostEffect = SHADER->GetResource("fx/PostEffect.fx");

	this->pDirLight->RotateX(D3DX_PI / 2);
	this->pDirLight->RotateX(-2.0f*TIMEMANAGER->getElapsedTime());

	//TweakBar 준비
	TwBar* bar = TweakBar::Get()->GetBar();
	TwAddVarRW(bar, "PostEffect Pass", TW_TYPE_INT32, &nPassNumber, "min=0 max=5 step=1 group='PostEffect'");
	TwAddVarRW(bar, "Bloom Intensity", TW_TYPE_FLOAT, &fBloomIntensity, "min=0 max=1 step=0.1 group='PostEffect'");
}

void SceneBase::Release()
{
	this->Scene_Release(); //씬 해제

	//공통된 씬에서 쓰였던 객체들 해제
	SAFE_DELETE(pCamera);
	SAFE_DELETE(pDirLight);
	SAFE_DELETE(pDirLightCamera);
}

void SceneBase::Update()
{
	if (Keyboard::Get()->Down('1')) isWireFrame = !isWireFrame; //와이어프레임 세팅

	pCamera->CameraControl(); //카메라 움직임

	this->Scene_Update(); //씬의 업데이트
}

void SceneBase::Render()
{
	g_pD3DDevice->SetRenderState(D3DRS_FILLMODE, isWireFrame ? D3DFILL_WIREFRAME : D3DFILL_SOLID); //와이어프레임 그릴꺼냐 안그릴꺼냐?
	
	this->pCamera->RenderTextureBegin(0x00101010);

	pCamera->UpdateFrustum();

	this->Scene_Render(); //씬의 렌더

	g_pD3DDevice->SetRenderState(D3DRS_FILLMODE,D3DFILL_SOLID); //와이어프레임 그릴꺼냐 안그릴꺼냐?
	
	this->pCamera->RenderTextureEnd();
	
	//포스트이펙트 렌더
	
	if (this->pPostEffect)
	{
		this->pPostEffect->SetTechnique("PostEffect");
	
		this->pPostEffect->SetTexture("screenTex", this->pCamera->GetRenderTexture()); //메인카메라의 렌더타겟
		float pixelU = 1.0f / WINSIZE_X;
		float pixelV = 1.0f / WINSIZE_Y;
	
		//픽셀 UV 좌표
		this->pPostEffect->SetFloat("pixelSizeU", pixelU);
		this->pPostEffect->SetFloat("pixelSizeV", pixelV);
		this->pPostEffect->SetFloat("pixelHalfSizeU", pixelU*0.5f);
		this->pPostEffect->SetFloat("pixelHalfSizeV", pixelV*0.5f);
		
		switch (nPassNumber)
		{
		case 1 : //아웃라인 쉐이더
			this->pPostEffect->SetTexture("normalTex", this->pCamera->GetRenderTexture());
			break;
			 
		case 2: case 3: //블러 X, 블러 Y
			this->pPostEffect->SetFloat("blurScale", 3.0f);
			break;
	
		case 4: //BBO (얘도 블룸)
			this->pPostEffect->SetTexture("blurTex", this->pCamera->GetRenderTexture());
			this->pPostEffect->SetFloat("blurAmount", fBloomIntensity);
	
		case 5: //블룸
			this->pPostEffect->SetTexture("normalTex", this->pCamera->GetRenderTexture());
			this->pPostEffect->SetTexture("blurTex", this->pCamera->GetRenderTexture());
		}
	
		this->pPostEffect->Begin();
	
		this->pPostEffect->BeginPass(nPassNumber);
	
		g_pD3DDevice->SetFVF(SCENEVERTEX::FVF);
		g_pD3DDevice->DrawIndexedPrimitiveUP(D3DPT_TRIANGLELIST, 0, 4, 2, this->dwSceneIndex, D3DFMT_INDEX32, this->stSceneVertex, sizeof(SCENEVERTEX));

		this->pPostEffect->EndPass();
		
	
		this->pPostEffect->End();
	}
}

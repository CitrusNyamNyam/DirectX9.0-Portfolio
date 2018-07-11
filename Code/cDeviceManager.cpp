#include "stdafx.h"
#include "cDeviceManager.h"


cDeviceManager::cDeviceManager()
{
	m_pD3D = Direct3DCreate9(D3D_SDK_VERSION);

	// ����̽� ���� �˾Ƴ���
	int	nVertexProcessing;
	D3DCAPS9 d3dCaps;
	m_pD3D->GetDeviceCaps(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, &d3dCaps);
	if (d3dCaps.DevCaps & D3DDEVCAPS_HWTRANSFORMANDLIGHT)
	{
		nVertexProcessing = D3DCREATE_HARDWARE_VERTEXPROCESSING;
	}
	else
	{
		nVertexProcessing = D3DCREATE_SOFTWARE_VERTEXPROCESSING;
	}
	D3DPRESENT_PARAMETERS d3dPP;
	ZeroMemory(&d3dPP, sizeof(D3DPRESENT_PARAMETERS));
	d3dPP.SwapEffect = D3DSWAPEFFECT_DISCARD;
	d3dPP.Windowed = TRUE;
	d3dPP.BackBufferFormat = D3DFMT_UNKNOWN;
	d3dPP.EnableAutoDepthStencil = TRUE;
	d3dPP.AutoDepthStencilFormat = D3DFMT_D24X8;

	m_pD3D->CreateDevice(D3DADAPTER_DEFAULT,
		D3DDEVTYPE_HAL,
		g_hWnd,
		nVertexProcessing,
		&d3dPP,
		&m_pD3DDevice);
}


cDeviceManager::~cDeviceManager()
{
}

LPDIRECT3DDEVICE9 cDeviceManager::GetDevice()
{
	return m_pD3DDevice;
}

void cDeviceManager::Destroy()
{
	SAFE_RELEASE(m_pD3D);
	ULONG ul = m_pD3DDevice->Release();

#ifdef _DEBUG
		assert(ul == 0 && "����̽��� �̿��� ������ ��ü �� �������� ���� ��ü�� �ִ�.");
#endif

	//SAFE_RELEASE(m_pD3DDevice);
}
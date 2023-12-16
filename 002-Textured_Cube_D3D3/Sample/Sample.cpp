#include <windows.h>
#include <math.h>

#include <ddraw.h>
#include <d3d.h>
#include <d3dtypes.h>
#include <d3dcaps.h>

#pragma comment (lib, "ddraw.lib")
#pragma comment (lib, "dxguid.lib")

#define PI 3.14159265358979f
#define PI2 (PI * 2.0f)

LPDIRECTDRAW         g_pDD1           = NULL;
LPDIRECTDRAW4        g_pDD4           = NULL;
LPDIRECTDRAWSURFACE4 g_pDdsPrimary    = NULL;
LPDIRECTDRAWSURFACE4 g_pDdsBackBuffer = NULL;
LPDIRECT3D3          g_pD3D           = NULL;
LPDIRECT3DDEVICE3    g_pD3dDevice     = NULL;
LPDIRECT3DVIEWPORT3  g_pViewport     = NULL;
RECT                 g_RcScreenRect;
RECT                 g_RcViewportRect;
LPDIRECT3DTEXTURE2	 g_pCubeTexture  = NULL;

HWND g_hWnd;

struct vector3
{
	float x,y,z;
};

/*
Из DX документации описание структуры D3DVERTEX

struct D3DVERTEX {
	float x,y,z;	//позиция
	float nx,ny,nz,	//нормаль
	float tu, tv	//текстурные координаты
};
*/

//наш куб 24 вершины, с текстурными координатами, 12 треугольников
//освещение не используем нулевые нормали
D3DVERTEX g_VertBuff[24] = {
-5.000000,-5.000000,-5.000000,	0.000000, 0.000000, 0.000000,	1.0,1.0,
-5.000000,-5.000000,5.000000,	0.000000, 0.000000, 0.000000,	1.0,0.0,
5.000000,-5.000000,5.000000,	0.000000, 0.000000, 0.000000,	0.0,0.0,
5.000000,-5.000000,-5.000000,	0.000000, 0.000000, 0.000000,	0.0,1.0,
-5.000000,5.000000,-5.000000,	0.000000, 0.000000, 0.000000,	0.0,1.0,
5.000000,5.000000,-5.000000,	0.000000, 0.000000, 0.000000,	1.0,1.0,
5.000000,5.000000,5.000000,		0.000000, 0.000000, 0.000000,	1.0,0.0,
-5.000000,5.000000,5.000000,	0.000000, 0.000000, 0.000000,	0.0,0.0,
-5.000000,-5.000000,-5.000000,	0.000000, 0.000000, 0.000000,	0.0,1.0,
5.000000,-5.000000,-5.000000,	0.000000, 0.000000, 0.000000,	1.0,1.0,
5.000000,5.000000,-5.000000,	0.000000, 0.000000, 0.000000,	1.0,0.0,
-5.000000,5.000000,-5.000000,	0.000000, 0.000000, 0.000000,	0.0,0.0,
5.000000,-5.000000,-5.000000,	0.000000, 0.000000, 0.000000,	0.0,1.0,
5.000000,-5.000000,5.000000,	0.000000, 0.000000, 0.000000,	1.0,1.0,
5.000000,5.000000,5.000000,		0.000000, 0.000000, 0.000000,	1.0,0.0,
5.000000,5.000000,-5.000000,	0.000000, 0.000000, 0.000000,	0.0,0.0,
5.000000,-5.000000,5.000000,	0.000000, 0.000000, 0.000000,	0.0,1.0,
-5.000000,-5.000000,5.000000,	0.000000, 0.000000, 0.000000,	1.0,1.0,
-5.000000,5.000000,5.000000,	0.000000, 0.000000, 0.000000,	1.0,0.0,
5.000000,5.000000,5.000000,		0.000000, 0.000000, 0.000000,	0.0,0.0,
-5.000000,-5.000000,5.000000,	0.000000, 0.000000, 0.000000,	0.0,1.0,
-5.000000,-5.000000,-5.000000,	0.000000, 0.000000, 0.000000,	1.0,1.0,
-5.000000,5.000000,-5.000000,	0.000000, 0.000000, 0.000000,	1.0,0.0,
-5.000000,5.000000,5.000000,	0.000000, 0.000000, 0.000000,	0.0,0.0 };

WORD g_IndexBuff[36] = {
		0,2,1, 		// 1 triangle
		2,0,3,		// 2 triangle
		4,6,5,		// 3 triangle
		6,4,7,		// 4 triangle
		8,10,9,		// 5 triangle
		10,8,11,	// 6 triangle	
		12,14,13,	// 7 triangle
		14,12,15,	// 8 triangle
		16,18,17,	// 9 triangle
		18,16,19,	// 10 triangle
		20,22,21,	// 11 triangle
		22,20,23};	// 12 triangle



HRESULT CALLBACK Texture_Search_Callback( DDPIXELFORMAT* pddpf, VOID* param )
{
    DDSURFACEDESC2* pddsd = (DDSURFACEDESC2*)param;

    // Skip unwanted formats
    if( pddpf->dwRGBBitCount != pddsd->dwFlags )
        return DDENUMRET_OK;
    if( pddpf->dwFlags & (DDPF_LUMINANCE|DDPF_ALPHAPIXELS) )
        return DDENUMRET_OK;
    if( pddpf->dwFlags & (DDPF_BUMPLUMINANCE|DDPF_BUMPDUDV) )
        return DDENUMRET_OK;
    if( 0 != pddpf->dwFourCC )
        return DDENUMRET_OK;

    memcpy( &pddsd->ddpfPixelFormat, pddpf, sizeof(DDPIXELFORMAT) );
    
	return DDENUMRET_CANCEL;
}

LPDIRECT3DTEXTURE2 Get_Texture(char *szFilename)
{
	HRESULT hr;
	LPDIRECT3DTEXTURE2 FloorTexture  = NULL;
	LPDIRECTDRAWSURFACE4 TexSurface = NULL;

	//загружаем наше изображение BMP
	HBITMAP hbmBitmap = (HBITMAP)LoadImage(NULL, szFilename, 
				IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE | LR_CREATEDIBSECTION);

	//получаем параметры изображения
	//это необходимо что бы создать поверхность
	//например такие параметры как ширина
	//и высота изображения BMP
    BITMAP bm;
    GetObject( hbmBitmap, sizeof(BITMAP), &bm );

	DDSURFACEDESC2 ddsd;
    ZeroMemory( &ddsd, sizeof(DDSURFACEDESC2) );
    ddsd.dwSize          = sizeof(DDSURFACEDESC2);
    ddsd.dwFlags         = DDSD_CAPS|DDSD_WIDTH|DDSD_HEIGHT|DDSD_PIXELFORMAT;
    ddsd.ddsCaps.dwCaps  = DDSCAPS_TEXTURE;
    ddsd.dwWidth         = bm.bmWidth;
    ddsd.dwHeight        = bm.bmHeight;

	//сначала ищем 32 битный формат текстуры
    DDSURFACEDESC2 ddsdSearch;
    ddsdSearch.dwFlags = 32;
    g_pD3dDevice->EnumTextureFormats( Texture_Search_Callback, &ddsdSearch );

	//если не нашли 32 битный, ищем 16 битный формат текстуры
    if( 32 != ddsdSearch.ddpfPixelFormat.dwRGBBitCount )
    {
        ddsdSearch.dwFlags = 16;
        g_pD3dDevice->EnumTextureFormats( Texture_Search_Callback,
                                                  &ddsdSearch );
        if( 16 != ddsdSearch.ddpfPixelFormat.dwRGBBitCount )
			//return E_FAIL;
			return NULL;
    }

    //если мы получили нужный формат текстуры (32 бит)
	//используем его что бы создать поверхность
    memcpy( &ddsd.ddpfPixelFormat, &ddsdSearch.ddpfPixelFormat,
            sizeof(DDPIXELFORMAT) );

	//создаем поверхность для текстуры
	hr = g_pDD4->CreateSurface( &ddsd, &TexSurface, NULL );
	if( FAILED( hr ) )
		return NULL;

	//копируем изображение текстуры BMP в нашу поверхность
	HDC hdcBitmap = CreateCompatibleDC( NULL );
	SelectObject( hdcBitmap, hbmBitmap );

	HDC hdcSurface;
    if( SUCCEEDED( TexSurface->GetDC( &hdcSurface ) ) )
    {
        BitBlt( hdcSurface, 0, 0, bm.bmWidth, bm.bmHeight, hdcBitmap, 0, 0,
                SRCCOPY );

        TexSurface->ReleaseDC( hdcSurface );
    }
    DeleteDC( hdcBitmap );

	//скопировали изображение BMP в поверхность
	//у поверхности запрашиваем интерфейс текстуры
	TexSurface->QueryInterface( IID_IDirect3DTexture2,
                                         (VOID**)&FloorTexture ) ;
	
	TexSurface->Release();

	//возвращаем текстуру в которую скопировано изображение BMP
	return FloorTexture;
}

float Vec3_Dot(vector3 v1, vector3 v2)
{
	return v1.x * v2.x + v1.y * v2.y + v1.z * v2.z;
}

vector3 Vec3_Normalize(vector3 v)
{
	float len = sqrtf((v.x * v.x) + (v.y * v.y) + (v.z * v.z));
	vector3 t = { v.x / len, v.y / len, v.z / len };
	return t;
}

vector3 Vec3_Cross(vector3 v1, vector3 v2)
{
	vector3 t = { v1.y * v2.z - v1.z * v2.y,
			v1.z * v2.x - v1.x * v2.z,
			v1.x * v2.y - v1.y * v2.x };

	return t;
}

HRESULT Initialize_3DEnvironment()
{
	HRESULT hr;

    //-------------------------------------------------------------------------
	// Step 1: Create DirectDraw and set the coop level
    //-------------------------------------------------------------------------

	// Create the IDirectDraw interface. The first parameter is the GUID,
	// which is allowed to be NULL. If there are more than one DirectDraw
	// drivers on the system, a NULL guid requests the primary driver. For 
	// non-GDI hardware cards like the 3DFX and PowerVR, the guid would need
	// to be explicity specified . (Note: these guids are normally obtained
	// from enumeration, which is convered in a subsequent tutorial.)
	hr = DirectDrawCreate( NULL, &g_pDD1, NULL );
	if( FAILED( hr ) )
		return hr;

	// Get a ptr to an IDirectDraw4 interface. This interface to DirectDraw
	// represents the DX6 version of the API.
	hr = g_pDD1->QueryInterface( IID_IDirectDraw4, (VOID**)&g_pDD4 );
	if( FAILED( hr ) )
		return hr;

    // Set the Windows cooperative level. This is where we tell the system
	// whether wew will be rendering in fullscreen mode or in a window. Note
	// that some hardware (non-GDI) may not be able to render into a window.
	// The flag DDSCL_NORMAL specifies windowed mode. Using fullscreen mode
	// is the topic of a subsequent tutorial. The DDSCL_FPUSETUP flag is a 
	// hint to DirectX to optomize floating points calculations. See the docs
	// for more info on this. Note: this call could fail if another application
	// already controls a fullscreen, exclusive mode.
    hr = g_pDD4->SetCooperativeLevel( g_hWnd, DDSCL_NORMAL );
	if( FAILED( hr ) )
		return hr;

    //-------------------------------------------------------------------------
	// Step 2: Create DirectDraw surfaces used for rendering
    //-------------------------------------------------------------------------

	// Initialize a surface description structure for the primary surface. The
	// primary surface represents the entire display, with dimensions and a
	// pixel format of the display. Therefore, none of that information needs
	// to be specified in order to create the primary surface.
	DDSURFACEDESC2 ddsd;
	ZeroMemory( &ddsd, sizeof(DDSURFACEDESC2) );
	ddsd.dwSize         = sizeof(DDSURFACEDESC2);
	ddsd.dwFlags        = DDSD_CAPS;
	ddsd.ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE;

	// Create the primary surface.
	hr = g_pDD4->CreateSurface( &ddsd, &g_pDdsPrimary, NULL );
	if( FAILED( hr ) )
		return hr;

	// Setup a surface description to create a backbuffer. This is an
	// offscreen plain surface with dimensions equal to our window size.
	// The DDSCAPS_3DDEVICE is needed so we can later query this surface
	// for an IDirect3DDevice interface.
	ddsd.dwFlags        = DDSD_WIDTH | DDSD_HEIGHT | DDSD_CAPS;
	ddsd.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN | DDSCAPS_3DDEVICE;

	// Set the dimensions of the backbuffer. Note that if our window changes
	// size, we need to destroy this surface and create a new one.
	GetClientRect( g_hWnd, &g_RcScreenRect );
	GetClientRect( g_hWnd, &g_RcViewportRect );
	ClientToScreen( g_hWnd, (POINT*)&g_RcScreenRect.left );
	ClientToScreen( g_hWnd, (POINT*)&g_RcScreenRect.right );
	ddsd.dwWidth  = g_RcScreenRect.right - g_RcScreenRect.left;
	ddsd.dwHeight = g_RcScreenRect.bottom - g_RcScreenRect.top;

	// Create the backbuffer. The most likely reason for failure is running
	// out of video memory. (A more sophisticated app should handle this.)
	hr = g_pDD4->CreateSurface( &ddsd, &g_pDdsBackBuffer, NULL );
	if( FAILED( hr ) )
		return hr;

	// Note: if using a z-buffer, the zbuffer surface creation would go around
	// here. However, z-buffer usage is the topic of a subsequent tutorial.

	// Create a clipper object which handles all our clipping for cases when
	// our window is partially obscured by other windows. This is not needed
	// for apps running in fullscreen mode.
	LPDIRECTDRAWCLIPPER pcClipper;
	hr = g_pDD4->CreateClipper( 0, &pcClipper, NULL );
	if( FAILED( hr ) )
		return hr;

	// Associate the clipper with our window. Note that, afterwards, the
	// clipper is internally referenced by the primary surface, so it is safe
	// to release our local reference to it.
	pcClipper->SetHWnd( 0, g_hWnd );
	g_pDdsPrimary->SetClipper( pcClipper );
	pcClipper->Release();

    //-------------------------------------------------------------------------
	// Step 3: Create the Direct3D interfaces
    //-------------------------------------------------------------------------

    // Query DirectDraw for access to Direct3D
    g_pDD4->QueryInterface( IID_IDirect3D3, (VOID**)&g_pD3D );
    if( FAILED( hr) )
		return hr;

	// Before creating the device, check that we are NOT in a palettized
	// display. That case will cause CreateDevice() to fail, since this simple 
	// tutorial does not bother with palettes.
	ddsd.dwSize = sizeof(DDSURFACEDESC2);
	g_pDD4->GetDisplayMode( &ddsd );
	if( ddsd.ddpfPixelFormat.dwRGBBitCount <= 8 )
		return DDERR_INVALIDMODE;

	// Create the device. The GUID is hardcoded for now, but should come from
	// device enumeration, which is the topic of a future tutorial. The device
	// is created off of our back buffer, which becomes the render target for
	// the newly created device.
    hr = g_pD3D->CreateDevice( IID_IDirect3DHALDevice, g_pDdsBackBuffer,
                               &g_pD3dDevice, NULL );

	if( FAILED( hr ) )
		return hr;

	    //-------------------------------------------------------------------------
	// Step 4: Create the viewport
    //-------------------------------------------------------------------------

    // Set up the viewport data parameters
    D3DVIEWPORT2 vdData;
    ZeroMemory( &vdData, sizeof(D3DVIEWPORT2) );
    vdData.dwSize       = sizeof(D3DVIEWPORT2);
	vdData.dwWidth      = g_RcScreenRect.right - g_RcScreenRect.left;
	vdData.dwHeight     = g_RcScreenRect.bottom - g_RcScreenRect.top;
    vdData.dvClipX      = -1.0f;
    vdData.dvClipWidth  = 2.0f;
    vdData.dvClipY      = 1.0f;
    vdData.dvClipHeight = 2.0f;
    vdData.dvMaxZ       = 1.0f;

    // Create the viewport
    hr = g_pD3D->CreateViewport( &g_pViewport, NULL );
	if( FAILED( hr ) )
		return hr;

    // Associate the viewport with the D3DDEVICE object
    g_pD3dDevice->AddViewport( g_pViewport );

    // Set the parameters to the new viewport
    g_pViewport->SetViewport2( &vdData );

    // Set the viewport as current for the device
    g_pD3dDevice->SetCurrentViewport( g_pViewport );


	return hr;
}

void Init_Scene()
{
	//MATRIX VIEW CALCULATION
	vector3 VecRight = { 1.0f, 0.0f, 0.0 };
	vector3 VecUp = { 0.0f, 1.0f, 0.0f }; 
	vector3 VecCamPos = { 0.0f, 0.0f, -15.0f };
	vector3 VecLook = { -1.0f * VecCamPos.x, -1.0f * VecCamPos.y, -1.0f * VecCamPos.z };
	
	VecLook = Vec3_Normalize(VecLook);

	VecUp = Vec3_Cross(VecLook, VecRight);
	VecUp = Vec3_Normalize(VecUp);
	VecRight = Vec3_Cross(VecUp, VecLook);
	VecRight = Vec3_Normalize(VecRight);

	float xp = -Vec3_Dot(VecCamPos, VecRight);
	float yp = -Vec3_Dot(VecCamPos, VecUp);
	float zp = -Vec3_Dot(VecCamPos, VecLook);
	
	D3DMATRIX  MatView = {
		VecRight.x,	VecUp.x,	VecLook.x,	0.0,
		VecRight.y,	VecUp.y,	VecLook.y,	0.0,
		VecRight.z,	VecUp.z,	VecLook.z,	0.0,
		xp,			yp,			zp,			1.0 };

	//MATRIX PROJECTION CALCULATION
	RECT rc;
	GetClientRect(g_hWnd, &rc);

	float fFov = 3.14f / 2.0f; // FOV 90 degree
	float fAspect = (float)rc.right / (float)rc.bottom;
	float fZFar = 100.0f;
	float fZNear = 1.0f;

	float    h, w, Q;

	w = (1.0f / tanf(fFov * 0.5f)) / fAspect;
	h = 1.0f / tanf(fFov * 0.5f);
	Q = fZFar / (fZFar - fZNear);

	D3DMATRIX MatProj = {
		w,		0.0,	0.0,			0.0,
		0.0,	h,		0.0,			0.0,
		0.0,	0.0,	Q,				1.0,
		0.0,	0.0,	-Q * fZNear,	0.0 };

    g_pD3dDevice->SetTransform( D3DTRANSFORMSTATE_VIEW,       &MatView );
    g_pD3dDevice->SetTransform( D3DTRANSFORMSTATE_PROJECTION, &MatProj );

	//отбрасывание задних поверхностей
	//вершины куба по часовой стрелке
	g_pD3dDevice->SetRenderState(D3DRENDERSTATE_CULLMODE, D3DCULL_CCW);
	g_pD3dDevice->SetRenderState(D3DRENDERSTATE_TEXTUREPERSPECTIVE, true);

	g_pCubeTexture = Get_Texture("texture24.bmp");
}

VOID On_Move(int x, int y)
{
	DWORD dwWidth  = g_RcScreenRect.right - g_RcScreenRect.left;
	DWORD dwHeight = g_RcScreenRect.bottom - g_RcScreenRect.top;
    SetRect( &g_RcScreenRect, x, y, x + dwWidth, y + dwHeight );
}

void Update_Scene()
{
	float static Angle = 0.0f;

	//MATRIX WORLD
	//вращение по оси Y
	D3DMATRIX MatWorld = {
		cosf(Angle),	0.0,	-sinf(Angle),	0.0,
		0.0,			1.0,	0.0,			0.0,
		sinf(Angle),	0.0,	cosf(Angle),	0.0,
		0.0,			0.0,	0.0,			1.0 };

	Angle += PI / 10000.0f;
	if(Angle > PI2)
		Angle = 0.0f;

	g_pD3dDevice->SetTransform( D3DTRANSFORMSTATE_WORLD, &MatWorld );

}

HRESULT Render_Scene()
{

	HRESULT hr = g_pViewport->Clear2( 1UL, (D3DRECT*)&g_RcViewportRect, D3DCLEAR_TARGET,
		                0x00ffffff, 1.0f, 0L );
	if(FAILED( hr))
		return E_FAIL;

    // Begin the scene
    if( FAILED( g_pD3dDevice->BeginScene() ) )
    {
        // Don't return an error, unless we want the app to exit
        return S_OK;
    }

	g_pD3dDevice->SetTextureStageState( 0, D3DTSS_MINFILTER, D3DTFN_LINEAR );
    g_pD3dDevice->SetTextureStageState( 0, D3DTSS_MAGFILTER, D3DTFG_LINEAR );

    g_pD3dDevice->SetTexture( 0, g_pCubeTexture );

	if( FAILED( g_pD3dDevice->DrawIndexedPrimitive( D3DPT_TRIANGLELIST, D3DFVF_VERTEX, 
                               g_VertBuff, 24,
							   g_IndexBuff, 36, NULL ) ) )
    {
        return S_OK;
    }

	// End the scene.
    g_pD3dDevice->EndScene();

	//приложение отслеживет сообщение On_Move()
	//что бы правильно рассчитать позицию окна на экране
	g_pDdsPrimary->Blt( &g_RcScreenRect, g_pDdsBackBuffer, 
                               &g_RcViewportRect, DDBLT_WAIT, NULL );

	return S_OK;

}

void Destroy_App()
{
	if(g_pViewport)
	{
		g_pD3dDevice->DeleteViewport(g_pViewport);
		g_pViewport->Release();
		g_pViewport = NULL;
	}

	if(g_pD3dDevice)
	{
		g_pD3dDevice->Release();
		g_pD3dDevice = NULL;
	}

	if(g_pCubeTexture)
	{
		g_pCubeTexture->Release();
		g_pCubeTexture = NULL;
	}

	if(g_pDdsBackBuffer)
	{
		g_pDdsBackBuffer->Release();
		g_pDdsBackBuffer = NULL;
	}
	if(g_pDdsPrimary)
	{
		g_pDdsPrimary->Release();
		g_pDdsPrimary = NULL;
	}
	
	if(g_pD3D)
	{
		g_pD3D->Release();
		g_pD3D = NULL;
	}

	if(g_pDD4)
	{
		g_pDD4->Release();
		g_pDD4 = NULL;
	}

	if(g_pDD1)
	{
		g_pDD1->Release();
		g_pDD1 = NULL;
	}
}

LRESULT CALLBACK WndProc(HWND g_hWnd,
						 UINT uMsg,
						 WPARAM wParam,
						 LPARAM lParam)
{
	switch(uMsg)
	{
		case WM_CLOSE:
			PostQuitMessage(0);
			break;
		case WM_MOVE:
			// Move messages need to be tracked to update the screen rects
			// used for blitting the backbuffer to the primary.
			On_Move( (SHORT)LOWORD(lParam), (SHORT)HIWORD(lParam) );
            break;

		default:
			return DefWindowProc(g_hWnd, uMsg, wParam, lParam);
	}

	return 0;
	
}

int PASCAL WinMain(HINSTANCE hInstance,
				   HINSTANCE hPrevInstance,
					LPSTR lpCmdLine,
					int nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

	WNDCLASS wcl;
	wcl.style = CS_HREDRAW | CS_VREDRAW;
	wcl.lpfnWndProc = WndProc;
	wcl.cbClsExtra = 0L;
	wcl.cbWndExtra = 0L;
	wcl.hInstance = hInstance;
	wcl.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	wcl.hCursor = LoadCursor(NULL, IDC_ARROW);
	wcl.hbrBackground = (HBRUSH)(COLOR_WINDOW+1);
	wcl.lpszMenuName = NULL;
	wcl.lpszClassName = "Sample";

	/*
	WNDCLASS wcl = { CS_HREDRAW|CS_VREDRAW, WndProc,
			0, 0, hInstance, NULL, LoadCursor(NULL, IDC_ARROW),
			(HBRUSH)(COLOR_WINDOW+1),
			NULL, "Sample"};

	*/

	if(!RegisterClass(&wcl))
		return 0;

	g_hWnd = CreateWindow("Sample", "Sample Application",
					WS_OVERLAPPEDWINDOW,
					0, 0,
					640, 480,
					NULL,
					NULL,
					hInstance,
					NULL);
	if(!g_hWnd)
		return 0;

	ShowWindow(g_hWnd, nCmdShow);
	UpdateWindow(g_hWnd);

	Initialize_3DEnvironment();

	Init_Scene();

	MSG msg;

	while(true)
	{
		if(PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			if(msg.message ==	WM_QUIT)
				break;
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}

		if(GetKeyState(VK_ESCAPE) & 0xFF00)
			break;

		Update_Scene();
		Render_Scene();
	}

	Destroy_App();

	DestroyWindow(g_hWnd);
	UnregisterClass(wcl.lpszClassName, wcl.hInstance);

	return (int)msg.wParam;
}
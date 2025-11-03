
// GLIMDlg.cpp: 구현 파일
//

#include "pch.h"
#include "framework.h"
#include "GLIM.h"
#include "GLIMDlg.h"
#include "afxdialogex.h"
#include <cstdlib>			// 랜덤 이동에서 난수 생성하기 위함(rand, srand)
#include <ctime>			// 난수 시드를 매번 다르게 설정 위함(time)
#include <cmath>			// 외접원 계산, 테두리 그릴 때 경계 박스 계산, 중심 좌표 정수화 위함(fabs, sqrt)

#define WM_APP_RANDTICK  (WM_APP + 1)
#define WM_APP_RANDDONE  (WM_APP + 2)

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// 응용 프로그램 정보에 사용되는 CAboutDlg 대화 상자입니다.

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// 대화 상자 데이터입니다.
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ABOUTBOX };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 지원입니다.

// 구현입니다.
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(IDD_ABOUTBOX)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


// CGLIMDlg 대화 상자

CGLIMDlg::CGLIMDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_GLIM_DIALOG, pParent)         
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME); 
	m_clickRadius = 0;								// 기본 포인트 점 반지름(입력 전 0 = 무효)
	m_clickGray = 0;								// 점 색(0=검정)
	m_clickCount = 0;								// 찍은 점(클릭) 개수
	m_dragIdx = -1;									// 드래그 중인 점 인덱스(-1 = 없음)
}

void CGLIMDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CGLIMDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_WM_LBUTTONDOWN()				// 마우스 왼쪽 클릭 시 이벤트 처리
	ON_WM_MOUSEMOVE()
	ON_WM_LBUTTONUP()

	ON_BN_CLICKED(IDC_BTN_IMAGE, &CGLIMDlg::OnBnClickedBtnImage)
	ON_BN_CLICKED(IDC_BTN_RESET, &CGLIMDlg::OnBnClickedBtnReset)
	ON_BN_CLICKED(IDC_BTN_RANDOM, &CGLIMDlg::OnBnClickedBtnRandom)
	ON_MESSAGE(WM_APP_RANDTICK, &CGLIMDlg::OnRandTick)
	ON_MESSAGE(WM_APP_RANDDONE, &CGLIMDlg::OnRandDone)
END_MESSAGE_MAP()


// CGLIMEDlg 메시지 처리기

BOOL CGLIMDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// 시스템 메뉴에 "정보..." 메뉴 항목을 추가합니다.

	// IDM_ABOUTBOX는 시스템 명령 범위에 있어야 합니다.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != nullptr)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// 이 대화 상자의 아이콘을 설정합니다.  응용 프로그램의 주 창이 대화 상자가 아닐 경우에는
	//  프레임워크가 이 작업을 자동으로 수행합니다.
	SetIcon(m_hIcon, TRUE);			// 큰 아이콘을 설정합니다.
	SetIcon(m_hIcon, FALSE);		// 작은 아이콘을 설정합니다.

	// TODO: 여기에 추가 초기화 작업을 추가합니다.

	SetDlgItemText(IDC_EDIT_RADIUS, _T("")); // Edit 기본 텍스트

	return TRUE;  // 포커스를 컨트롤에 설정하지 않으면 TRUE를 반환합니다.
}

void CGLIMDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();	
	}
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
}

// 대화 상자에 최소화 단추를 추가할 경우 아이콘을 그리려면
//  아래 코드가 필요합니다.  문서/뷰 모델을 사용하는 MFC 애플리케이션의 경우에는
//  프레임워크에서 이 작업을 자동으로 수행합니다.

void CGLIMDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 그리기를 위한 디바이스 컨텍스트입니다.

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// 클라이언트 사각형에서 아이콘을 가운데에 맞춥니다.
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// 아이콘을 그립니다.
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

// 사용자가 최소화된 창을 끄는 동안에 커서가 표시되도록 시스템에서
//  이 함수를 호출합니다.
HCURSOR CGLIMDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}


// 이미지(캔버스) 만들기
void CGLIMDlg::OnBnClickedBtnImage()
{
	int nWidth = 800;					
	int nHeight = 600;					
	int nBpp = 8;						

	if (m_image != NULL) m_image.Destroy();
	m_image.Create(nWidth, -nHeight, nBpp); 

	if (nBpp == 8) 
	{
		static RGBQUAD table[256];
		for (int i = 0; i < 256; ++i)
		{
			table[i].rgbRed = table[i].rgbGreen = table[i].rgbBlue = (BYTE)i;
		}
		m_image.SetColorTable(0, 256, table);
	}

	int pitch = m_image.GetPitch();
	BYTE* fm = (BYTE*)m_image.GetBits();
	for (int j = 0; j < nHeight; ++j)
	{
		memset(fm + j * pitch, 0xFF, nWidth);
	}
		
	//배경 초기화 시 점 개수 리셋
	m_clickCount = 0;	 
	m_numPts = 0;		
	m_dragIdx = -1;		
	m_ringDrawn = false;

	UpdateDisplay();
}

// 화면에 이미지 그리기
void CGLIMDlg::UpdateDisplay()
{
	if (m_image == NULL)
	{
		return;
	}
	CClientDC dc(this);
	m_image.Draw(dc, 0, 0);
}

// 마우스 왼쪽 클릭 지점에 원 그리기 
void CGLIMDlg::OnLButtonDown(UINT nFlags, CPoint point)
{
	// 배경이 없으면 클릭을 무시하기
	if (m_image == NULL)	
	{
		CDialogEx::OnLButtonDown(nFlags, point);  
		return;
	}
		
	// 포인트 점 3개가 생겼을 때
	if (m_numPts == 3 && !m_ringDrawn) 
	{
		BOOL thicknessOk = FALSE;												// 두께 입력이 기본은 FALSE
		UINT thick = GetDlgItemInt(IDC_EDIT_THICKNESS, &thicknessOk, FALSE);	// 두께 정수 입력 읽기
		
		// 입력된 두께가 없으면 메세지 박스
		if (!thicknessOk || thick == 0)
		{
			AfxMessageBox(_T("테두리 두께를 먼저 입력하세요."));
			return;
		}

		double ux = 0;	// 세 점을 모두 지나는 원의 중심 X 좌표
		double uy = 0;	// 세 점을 모두 지나는 원의 중심 y 좌표
		double ur = 0;	// 세 점을 모두 지나는 원의 반지름
		
		unsigned char* fm = (unsigned char*)m_image.GetBits();

		// 8bpp 회색 버퍼에 입력 받은 두께만큼 큰 원 그리기
		drawRingBand8bpp(fm, m_image.GetWidth(), m_image.GetHeight(), m_image.GetPitch(),
			ux, uy, ur, (int)thick, 0);
		UpdateDisplay();
 		m_ringDrawn = true;
		return;					
	}

	const int hit = m_clickRadius; // 점 반지름만 사용

	// 기존 점을 클릭했을 때 중심점을 찾고 Edit에 좌표 표시
	for (int i = 0; i < m_numPts; ++i) 
	{
		int dx = point.x - m_pts[i].x, dy = point.y - m_pts[i].y;
		if (dx * dx + dy * dy <= hit * hit) 
		{
			// 계산 전용 이미지(화면x)
			CImage tmp;
			CreateGray8MaskLike(m_image, tmp);
			int r = (m_ptRadius[i] > 0) ? m_ptRadius[i] : m_clickRadius;
			PaintMarkerToMask(tmp, m_pts[i], r);
						
			unsigned char* fm = (unsigned char*)tmp.GetBits();
			int nWidth = tmp.GetWidth();
			int nHeight = tmp.GetHeight();
			int nPitch = tmp.GetPitch();

			int nTh = 0x80;
			int nSumX = 0;		// 밝은 픽셀들의 x합
			int nSumY = 0;		// 밝은 픽셀들의 y합
			int nCount = 0;		// 개수 증가

			CRect rect(0, 0, nWidth, nHeight);
			for (int j = rect.top; j < rect.bottom; j++) 
			{
				for (int k = rect.left; k < rect.right; k++) 
				{
					if (fm[j * nPitch + k] > nTh) 
					{
						nSumX += k;
						nSumY += j;
						nCount++;
					}
				}
			}
			if (nCount > 0) 
			{
				// 무게중심을 계산해서 좌표로 만드는 로직
				int cx = (int)std::round((double)nSumX / nCount);
				int cy = (int)std::round((double)nSumY / nCount);
				CString s; s.Format(_T("%d, %d"), cx, cy);
				SetDlgItemText(IDC_EDIT_CENTER, s);				// Edit에 중심 좌표 표시
			}

			m_dragIdx = i;     // 이 점을 드래그 대상으로 지정
			SetCapture();      // 마우스 캡처 시작(창 밖으로 나가도 드래그 유지)

			return;
		}

	}

	// 점은 최대 3개까지만 생성
	if (m_numPts >= 3)
	{
		return;
	}

	// 반지름 읽기
	BOOL radiusOk = FALSE;
	UINT radius = GetDlgItemInt(IDC_EDIT_RADIUS, &radiusOk, FALSE); // FALSE: 음수 입력 불가
	
	// 반지름 입력 검증 (숫자가 아니거나 0 일 경우)
	if (!radiusOk || radius == 0)
	{
		AfxMessageBox(_T("반지름을 입력해 주세요."));
		return; 
	}

	// 사용자가 반지름 크기 설정하기
	if (CWnd* pEdit = GetDlgItem(IDC_EDIT_RADIUS)) 
	{
		CString t; pEdit->GetWindowText(t);
		int v = _ttoi(t);
		if (v > 0)
		{
			m_clickRadius = v;
		}
	}

	// 이미지 크기와 반지름을 가져온다.
	int imgW = m_image.GetWidth();
	int imgH = m_image.GetHeight();
	int r = m_clickRadius;

	// 캔버스가 아직 생성되지 않았거나 반지름이 0/미설정 원을 그릴 수 없다.
	if (imgW <= 0 || imgH <= 0 || r <= 0) 
	{ 
		CDialogEx::OnLButtonDown(nFlags, point); 
		return; 
	}

	// drawCircle은 좌상단+반지름 기준 → 클릭점을 중심으로 좌상단 구하고 경계 보정
	int tlx = max(0, min(point.x - r, imgW - r * 2));
	int tly = max(0, min(point.y - r, imgH - r * 2));

	unsigned char* fm = (unsigned char*)m_image.GetBits();
	drawCircle(fm, tlx, tly, r, m_clickGray);
	UpdateDisplay();

	// 점 저장
	int cx = tlx + r;
	int cy = tly + r;

	if (m_numPts < 3) 
	{
		m_pts[m_numPts] = CPoint(cx, cy);		 // 중심 저장
		m_ptRadius[m_numPts] = r;				 // 당시 반지름 저장
		m_numPts++;
	}
	
	// 3개가 모이면 정원(테두리) 그리기
	if (m_numPts == 3) 
	{		
		BOOL thicknessOk = FALSE;
		UINT thick = GetDlgItemInt(IDC_EDIT_THICKNESS, &thicknessOk, FALSE);
		
		if (!thicknessOk || thick == 0)
		{
			AfxMessageBox(_T("테두리 두께를 먼저 입력하세요."));
			return; 
		}

		// 외접원 계산
		double ux = 0;
		double uy = 0;
		double ur = 0;

		if (!computeCircumcircle(m_pts[0], m_pts[1], m_pts[2], ux, uy, ur)) 
		{	
			return;
		}

		// 테두리 그리기
		unsigned char* fm = (unsigned char*)m_image.GetBits();
		drawRingBand8bpp(fm, m_image.GetWidth(), m_image.GetHeight(), m_image.GetPitch(),
			ux, uy, ur, (int)thick, 0);		// 0 = 검정
		UpdateDisplay();
		return;
	}

	m_clickCount++;							// 생성된 점 +1
	CDialogEx::OnLButtonDown(nFlags, point);
}

// 픽셀 직접 칠해 채워진 원을 실제로 그리는 함수
void CGLIMDlg::drawCircle(unsigned char* fm, int x, int y, int nRadius, int nGray)
{
	int nCenterX = x + nRadius;
	int nCenterY = y + nRadius;
	int nPitch = m_image.GetPitch();

	for (int j = y; j < y + nRadius * 2; ++j) {
		for (int i = x; i < x + nRadius * 2; ++i) {
			if (isInCircle(i, j, nCenterX, nCenterY, nRadius))
				fm[j * nPitch + i] = (BYTE)nGray;
		}
	}
}

// 원 내부 여부를 판정하는 보조 함수
bool CGLIMDlg::isInCircle(int i, int j, int nCenterX, int nCenterY, int nRadius)
{
	bool bRet = false;

	double dX = i - nCenterX;
	double dY = j - nCenterY;
	double dDist = dX * dX + dY * dY;

	if (dDist < nRadius * nRadius) 
	{
		bRet = true;
	}

	return bRet;
}

// 임시 마스크 만들기(중심 계산용)
void CGLIMDlg::CreateGray8MaskLike(const CImage& src, CImage& mask) 
{
	if (mask != NULL)
	{
		mask.Destroy();
	}
	mask.Create(src.GetWidth(), -src.GetHeight(), 8); // top-down
	static RGBQUAD table[256];
	
	for (int i = 0; i < 256; ++i) 
	{
		table[i].rgbRed = table[i].rgbGreen = table[i].rgbBlue = (BYTE)i;
	}
	mask.SetColorTable(0, 256, table);

	int pitch = mask.GetPitch();
	BYTE* m = (BYTE*)mask.GetBits();
	
	for (int y = 0; y < src.GetHeight(); ++y) 
	{
		memset(m + y * pitch, 0x00, src.GetWidth()); // 전부 0(검정)
	}
}

//  선택한 점의 원을 흰색(255)으로 만들기 (중심 계산용)
void CGLIMDlg::PaintMarkerToMask(CImage& mask, CPoint c, int r)
{
	int w = mask.GetWidth(), h = mask.GetHeight(), pitch = mask.GetPitch();
	BYTE* m = (BYTE*)mask.GetBits();
	int x0 = max(0, c.x - r), x1 = min(w - 1, c.x + r);
	int y0 = max(0, c.y - r), y1 = min(h - 1, c.y + r);
	for (int y = y0; y <= y1; ++y) 
	{
		for (int x = x0; x <= x1; ++x) 
		{
			int dx = x - c.x, dy = y - c.y;
			if (dx * dx + dy * dy <= r * r) 
			{
				m[y * pitch + x] = 255; // 흰색(밝은 픽셀)
			}
				
		}
	}
}

bool CGLIMDlg::computeCircumcircle(const CPoint& A, const CPoint& B, const CPoint& C,
	double& cx, double& cy, double& r)
{
	// 좌표 읽기
	const double x1 = A.x;
	const double y1 = A.y;
	const double x2 = B.x;
	const double y2 = B.y;
	const double x3 = C.x;
	const double y3 = C.y;

	const double D = 2.0 * (x1 * (y2 - y3) + x2 * (y3 - y1) + x3 * (y1 - y2));
	if (std::fabs(D) < 1e-6) 
	{
		return false; // 세 점이 일직선
	}

	// 각 점까지의 원점 거리 제곱
	const double s1 = (x1 * x1) + (y1 * y1);	
	const double s2 = (x2 * x2) + (y2 * y2);
	const double s3 = (x3 * x3) + (y3 * y3);

	cx = (s1 * (y2 - y3) + s2 * (y3 - y1) + s3 * (y1 - y2)) / D;	// 외접원 중심의 x좌표 공식
	cy = (s1 * (x3 - x2) + s2 * (x1 - x3) + s3 * (x2 - x1)) / D;	// 외접원 중심의 y좌표 공식
	r = std::sqrt((cx - x1) * (cx - x1) + (cy - y1) * (cy - y1));
	return r > 0.0;
}

// 원 테두리 그리기
void CGLIMDlg::drawRingBand8bpp(unsigned char* fm, int imgW, int imgH, int pitch,
	double cx, double cy, double r, int thickness, int gray)
{
	if (thickness <= 0 || r <= 0.0)
	{
		return;
	}

	const double halfT = thickness * 0.5;
	double inner = r - halfT; 

	if (inner < 0.0) 
	{
		inner = 0.0;
	}

	const double inner2 = inner * inner;
	const double outer = r + halfT;
	const double outer2 = outer * outer;

	int x0 = (int) std::floor(cx - outer); 
	if (x0 < 0)
	{
		x0 = 0;
	}

	int x1 = (int) std::ceil(cx + outer); 
	if (x1 > imgW - 1)
	{
		x1 = imgW - 1;
	}

	int y0 = (int) std::floor(cy - outer); 
	if (y0 < 0)
	{
		y0 = 0;
	}

	int y1 = (int) std::ceil(cy + outer);
	if (y1 > imgH - 1)
	{
		y1 = imgH - 1;
	}

	for (int y = y0; y <= y1; ++y) 
	{
		unsigned char* row = fm + y * pitch;
		const double dy = y - cy;
		for (int x = x0; x <= x1; ++x) 
		{
			const double dx = x - cx;
			const double d2 = dx * dx + dy * dy;
			if (d2 >= inner2 && d2 <= outer2)
			{
				row[x] = (unsigned char)gray;
			}
		}
	}
}

// 포인트 점 드래그 했을 때 외접원 다시 그리기
void CGLIMDlg::OnMouseMove(UINT nFlags, CPoint point)
{
	// 드래그 중일 때만 반응하기
	if (m_dragIdx == -1 || !(nFlags & MK_LBUTTON) || m_image == NULL) 
	{
		CDialogEx::OnMouseMove(nFlags, point);
		return;
	}

	// 경계 검사용
	int imgW = m_image.GetWidth(), imgH = m_image.GetHeight();
	if (point.x < 0)
	{
		point.x = 0;
	}
	if (point.y < 0)
	{
		point.y = 0;
	}
	if (point.x > imgW - 1)
	{
		point.x = imgW - 1;
	}
	if (point.y > imgH - 1)
	{
		point.y = imgH - 1;
	}

	// 좌표가 바뀐 경우에만 갱신
	if (point != m_pts[m_dragIdx]) 
	{
		m_pts[m_dragIdx] = point;		// 드래그 중인 점의 중심 좌표를 최신 마우스 위치로 
		renderAll();					// 다시 그리기 ( 점 이동에 맞춰 원 테두리가 실시간 따라온다. )
	}
}

// 드래그 종료 ( 마우스 왼쪽 버튼 떼는 시점 )
void CGLIMDlg::OnLButtonUp(UINT nFlags, CPoint point)
{
	// 현재 드래그 중인지 확인
	if (m_dragIdx != -1) 
	{
		ReleaseCapture();	// 마우스 캡쳐 해제
		m_dragIdx = -1;		// 드래그 중 아니다. 상태를 초기화
	}
	CDialogEx::OnLButtonUp(nFlags, point);
}

// 전체 다시 그리기
void CGLIMDlg::renderAll()
{
	if (m_image == NULL) return;

	// 이미지 폭 / 높이 / 피치(한 줄 바이트 간격)
	int w = m_image.GetWidth(); 
	int h = m_image.GetHeight();
	int pitch = m_image.GetPitch();

	unsigned char* fm = (unsigned char*)m_image.GetBits();

	// 1) 배경 흰색
	for (int y = 0; y < h; ++y) 
	{
		memset(fm + y * pitch, 0xFF, w);
	}

	// 2) 점 다시 그리기 (중심 m_pts[k], 반지름 m_ptRadius[k])
	for (int k = 0; k < m_numPts; ++k) 
	{
		int r = m_ptRadius[k];
		int tl = m_pts[k].x - r, tt = m_pts[k].y - r;
		if (tl < 0)
		{
			tl = 0;
		}

		if (tt < 0)
		{
			tt = 0;
		}

		if (tl > w - r * 2)
		{
			tl = w - r * 2;
		}

		if (tt > h - r * 2)
		{
			tt = h - r * 2;
		}

		drawCircle(fm, tl, tt, r, m_clickGray);
	}

	// 3) 정원(외접원 테두리)
	if (m_numPts == 3) {
		BOOL thicknessOk = FALSE;
		UINT thick = GetDlgItemInt(IDC_EDIT_THICKNESS, &thicknessOk, FALSE);
		if (thicknessOk && thick > 0)
		{
			double cx = 0, cy = 0, rr = 0;
			if (computeCircumcircle(m_pts[0], m_pts[1], m_pts[2], cx, cy, rr)) 
			{
				drawRingBand8bpp(fm, w, h, pitch, cx, cy, rr, (int)thick, 0);
			}
		}
	}

	// 4) 화면 반영
	UpdateDisplay();
}

// 초기화 버튼 클릭 시 전부 초기화 하기
void CGLIMDlg::OnBnClickedBtnReset()
{
	m_isSimulating = false;
	m_ringDrawn = false;

	// 1) 상태 값 리셋
	m_clickCount = 0;
	m_numPts = 0;
	// 드래그 중이면 종료
	if (m_dragIdx != -1) { ReleaseCapture(); }
	m_dragIdx = -1;

	// 점/반지름 배열 초기화
	for (int i = 0; i < 3; ++i) {
		m_pts[i] = CPoint(0, 0);
		m_ptRadius[i] = 0;
	}

	// 2) UI 입력칸/표시 초기화
	SetDlgItemText(IDC_EDIT_RADIUS, _T(""));
	SetDlgItemText(IDC_EDIT_THICKNESS, _T(""));
	SetDlgItemText(IDC_EDIT_CENTER, _T(""));

	// 3) 화면(이미지) 지우기: 흰색으로 클리어 후 표시
	if (m_image != NULL) {
		const int w = m_image.GetWidth();
		const int h = m_image.GetHeight();
		const int p = m_image.GetPitch();
		unsigned char* fm = (unsigned char*)m_image.GetBits();
		for (int y = 0; y < h; ++y) memset(fm + y * p, 0xFF, w); // 0xFF = 흰색
		UpdateDisplay();
	}
}

// 엔터 키 눌렀을 때 프로그램 종료 방지
void CGLIMDlg::OnOK()
{
	// TODO: 여기에 특수화된 코드를 추가 및/또는 기본 클래스를 호출합니다.
	
	/*CDialogEx::OnOK();*/
}

// 랜덤 이동 버튼 클릭 시 
void CGLIMDlg::OnBnClickedBtnRandom()
{
	// 캔버스가 없거나 세 점이 아니면 실행 안 되게 하기.
	if (m_image == NULL || m_numPts != 3) 
	{
		AfxMessageBox(_T("정원이 준비된 상태에서 실행하세요."));
		return;
	}

	// 중복 스레드 생성 차단
	if (m_isSimulating) 
	{
		return;
	}

	// 난수 시드 1회 ( 실행마다 다 다른 결과가 나오게 한다. ) 
	std::srand((unsigned) std::time(nullptr));

	m_isSimulating = true;

	// 진행중에 버튼 비활성화 하기
	if (CWnd* w = GetDlgItem(IDC_BTN_RANDOM))
	{
		w->EnableWindow(FALSE);
	}

	// 워커 스레드 시작
	m_pRandThread = AfxBeginThread(&CGLIMDlg::RandThreadProc, this);
}

// 워커 스레드 (틱만 보낸다.)
UINT AFX_CDECL CGLIMDlg::RandThreadProc(LPVOID pParam)
{
	CGLIMDlg* dlg = reinterpret_cast<CGLIMDlg*>(pParam);
	
	// 요구사항대로 초당 2회, 총 10번 자동으로 반복
	for (int i = 0; i < 10 && dlg->m_isSimulating; ++i)
	{
		dlg->PostMessage(WM_APP_RANDTICK, 0, 0);
		::Sleep(500); // 초당 2회(2Hz)
	}
	dlg->PostMessage(WM_APP_RANDDONE, 0, 0);
	return 0;
}

// 메인 스레드에서 좌표 생성 + 재그림
LRESULT CGLIMDlg::OnRandTick(WPARAM, LPARAM)
{
	if (!m_isSimulating || m_image == NULL || m_numPts != 3)
	{
		return 0;
	}

	const int w = m_image.GetWidth();
	const int h = m_image.GetHeight();

	// 세 점을 각각 랜덤 이동
	for (int i = 0; i < 3; ++i) 
	{
		const int r = (m_ptRadius[i] > 0) ? m_ptRadius[i] : m_clickRadius;
		const int minX = r, maxX = max(r, w - 1 - r);
		const int minY = r, maxY = max(r, h - 1 - r);
		int x = (maxX > minX) ? (minX + (std::rand() % (maxX - minX + 1))) : minX;
		int y = (maxY > minY) ? (minY + (std::rand() % (maxY - minY + 1))) : minY;
		m_pts[i] = CPoint(x, y);	// i번째 점의 중심 좌표를 새 랜덤 좌표로 갱신
	}

	renderAll(); // 다시 그리기
	return 0;
}

// 랜덤 이동 끝나고 버튼 복구 (버튼 복구)
LRESULT CGLIMDlg::OnRandDone(WPARAM, LPARAM)
{
	m_isSimulating = false;
	
	// 버튼 활성화
	if (CWnd* w = GetDlgItem(IDC_BTN_RANDOM))
	{
		w->EnableWindow(TRUE);
	}
	return 0;
}

// GLIMDlg.h: 헤더 파일
//

#pragma once
#include <atlimage.h>


// CGLIMDlg 대화 상자
class CGLIMDlg : public CDialogEx
{

// 생성입니다.
public:
	CGLIMDlg(CWnd* pParent = nullptr);	// 표준 생성자입니다.

// 대화 상자 데이터입니다.
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_TEST_DIALOG };	
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 지원입니다.
	virtual void OnOK();								// 엔터키 종료 방지

// 구현입니다.
protected:
	HICON m_hIcon;

	// 생성된 메시지 맵 함수
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();

	afx_msg void OnBnClickedBtnImage();						// 배경 생성
	afx_msg void OnBnClickedBtnRandom();					// 랜덤 이동 
	afx_msg void OnBnClickedBtnReset();						// 초기화

	afx_msg void OnMouseMove(UINT nFlags, CPoint point);	// 마우스 이동(드래그 중 좌표 갱신)
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);	// 드래그 종료
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);	// 마우스 클릭 처리
	
	afx_msg LRESULT OnRandTick(WPARAM, LPARAM);				// 워커 스레드 틱 처리(좌표 랜덤/재그림)
	afx_msg LRESULT OnRandDone(WPARAM, LPARAM);				// 시뮬레이션 종료 처리
	DECLARE_MESSAGE_MAP()

private:

	CImage m_image;					// 8bpp 캔버스 버퍼
	int m_clickRadius;  			// 클릭 포인트 점의 기본 반지름 입력값
	int m_clickGray;    			// 클릭 포인트 점 색(0 = 검정, 255 = 흰색)
	int m_clickCount;	    		// 찍은 점의 개수
	
	CPoint m_pts[3];				// 클릭 포인트 점의 중심 좌표
	int m_ptRadius[3];				// 각 클릭 포인트 점을 찍을 때의 반지름
	int m_numPts = 0;				// 생성된 클릭 포인트 점의 개수
	bool m_ringDrawn = false;		// 3점이 만나서 생성되는 정원을 그렸는 지 확인
	int m_dragIdx;                  // 드래그 중인 점 인덱스

	void UpdateDisplay();			// 화면에 그리기
	void renderAll();               // 전체 다시 그리기
	
	
	// 마우스 클릭 시 포인트 점 그리기/판정 함수
	void drawCircle(unsigned char* fm, int x, int y, int nRadius, int nGray);
	
	// 원을 채울 때, 각 픽셀이 원“안”인지 여부를 판정하는 검사 함수
	bool isInCircle(int i, int j, int nCenterX, int nCenterY, int nRadius);

	// 정원(외접원) 계산
	static bool computeCircumcircle(const CPoint& A, const CPoint& B, const CPoint& C,
		double& cx, double& cy, double& r);

	// 정원 테두리(선) 그리기
	static void drawRingBand8bpp(unsigned char* fm, int imgW, int imgH, int pitch,
		double cx, double cy, double r, int thickness, int gray);

	// 중심(무게중심) 구하기용 마스크 유틸
	static void CreateGray8MaskLike(const CImage& src, CImage& mask);

	// 중심 계산용
	static void PaintMarkerToMask(CImage& mask, CPoint c, int r);

	// 스레드 상태(랜덤 이동 시뮬레이션)
	bool        m_isSimulating = false;          // 시뮬레이션 진행 여부(중복 실행/중단 플래그)
	CWinThread* m_pRandThread = nullptr;         // AfxBeginThread가 반환한 워커 스레드 포인터

	// 워커 스레드(랜덤 이동용)
	static UINT AFX_CDECL RandThreadProc(LPVOID pParam);	

};

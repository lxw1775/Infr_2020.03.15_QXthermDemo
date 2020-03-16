// XthermDemoDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "XthermDemo.h"
#include "XthermDemoDlg.h"
#include "XthermDll.h"
#include "process.h"
 
#include <dshow.h>
#pragma comment (lib,"Strmiids")
#pragma comment (lib,"quartz")

#pragma comment (lib,"XthermDll")
//#include <uuids.h>
#include <strmif.h>
#include "SampleGrabber.h"
//#include <devicetopology.h>
#include <Ks.h>
#include <Ksproxy.h>
#include <Ksmedia.h>

#include <qedit.h>
#include <mtype.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define SAFE_RELEASE(x) { if (x) x->Release(); x = NULL; } 

typedef void(WINAPI *CallBack)( BYTE* pData );

int ImageID;

   COMPVARS       lCOMPVARS;

	COMPVARS       mCOMPVARS;
	IAVIFile*      mAVIFile;
	IAVIStream*    mAVIStream;
	IAVIStream*    mAVICompressedStream;
	unsigned char *mTempBuffer;
	bool           mCompressing;
	unsigned short mSizeX;
	unsigned short mSizeY;
	unsigned short mBPP;
	unsigned long  mImageSize;
	unsigned long  mLastSample;

 //   const CyPixelConverter* lConverter = NULL;

bool tempflag=0;


typedef struct TMPPOS_t
{
	int x;
	int y;
} TMPPOS, * P_TMPPOS;

TMPPOS m_pMax,m_pMin;

TMPPOS m_pPos[3];

int rowadd=1;
int Tmpflag=0;

 int LEL=5;
 int WIDTH;  //图像宽度
 int HEIGHT; //图像高度
 int Datalength; //图像数据数
  float  fix,Refltmp,Airtmp,humi,emiss;
   unsigned short distance;

CDC* pDCDs;
int ParaFlag=1;
 CRect rectST;
HWND hWndST;
DWORD t,t1,tp;
 volatile HANDLE m_hVideothread=NULL; //实时图像获取进程句柄

int cardtype; 
CString SFilePath;//文件目录

int saveflag=0;

float Alltemp[1024*512];
// 用于应用程序“关于”菜单项的 CAboutDlg 对话框

class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

// 对话框数据
	enum { IDD = IDD_ABOUTBOX };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

// 实现
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
END_MESSAGE_MAP()


// CXthermDemoDlg 对话框

HWND hWin;
   int temptype=0;
 int frate;
bool Shutflag=0;
BOOL Connected=false;

	CString DevName[255];
  int DevNum;

   float tempmax,tempmin,tempcenter;
   float postmp[3];

  	 IMediaControl     *pMediaControl;         //媒体控制
	 	IMoniker          *pMoniker;               //监视器
	 IBaseFilter       *pSrc ,*pGrabFilter ,*pWrite,*pMux;  //过滤器
	IGraphBuilder     *pGraph;                 //过滤图表
		ICreateDevEnum	  *pDevEnum;
	IEnumMoniker	  *pClassEnum;
	ISampleGrabber	  *pGrabber;
	IVideoWindow	  *pWindow;
	ICaptureGraphBuilder2 *pBuilder;

		SampleGrabberCallback mCB;
	CallBack gCallBack =NULL;

HRESULT GetPin(IBaseFilter *pFilter, PIN_DIRECTION PinDir, IPin **ppPin)
{
	IEnumPins  *pEnum;
	IPin       *pPin;
	pFilter->EnumPins(&pEnum);
	while(pEnum->Next(1, &pPin, 0) == S_OK)
	{
		PIN_DIRECTION PinDirThis;
		pPin->QueryDirection(&PinDirThis);
		if (PinDir == PinDirThis)
		{
			pEnum->Release();
			*ppPin = pPin;
			return S_OK;
		}
		pPin->Release();
	}
	pEnum->Release();
	return E_FAIL;  
}

HRESULT ConnectFilters(IGraphBuilder *pGraph, IBaseFilter *pFirst, IBaseFilter *pSecond)
{
	IPin *pOut = NULL, *pIn = NULL;
	HRESULT hr = GetPin(pFirst, PINDIR_OUTPUT, &pOut);
	if (FAILED(hr)) return hr;
	hr = GetPin(pSecond, PINDIR_INPUT, &pIn);
	if (FAILED(hr)) 
	{
		pOut->Release();
		return E_FAIL;
	}
	hr = pGraph->Connect(pOut, pIn);
	pIn->Release();
	pOut->Release();
	return hr;
}




BOOL  UvcControl(IBaseFilter *pBF, WORD cmd, BYTE *data)
{
	IKsControl *pCtl = NULL;
	HRESULT hr;
	hr = pBF->QueryInterface(IID_IKsControl, (void **)&pCtl);
	if(FAILED(hr)) return (0);



	KSPROPERTY ksprop;  
	ZeroMemory(&ksprop, sizeof(ksprop));
	PVOID pData = NULL;
	ULONG valueSize = 0;
	ULONG ksLength = 0;
	ULONG dataLength = 0;
	ULONG resLength = 100;


	KSPROPERTY_CAMERACONTROL_S cameraControl;
	ksLength = sizeof(cameraControl);
	dataLength = sizeof(cameraControl);

	ZeroMemory(&cameraControl, sizeof(cameraControl));
	ksprop.Set = PROPSETID_VIDCAP_CAMERACONTROL; //PROPSETID_VIDCAP_VIDEOPROCAMP; // 
	ksprop.Id = KSPROPERTY_CAMERACONTROL_ZOOM; //KSPROPERTY_CAMERACONTROL_ZOOM;
	ksprop.Flags = KSPROPERTY_TYPE_SET;//KSPROPERTY_TYPE_GET;//
	cameraControl.Property = ksprop;
	cameraControl.Flags = KSPROPERTY_CAMERACONTROL_FLAGS_MANUAL;
	cameraControl.Capabilities = KSPROPERTY_CAMERACONTROL_FLAGS_MANUAL;
	cameraControl.Value = cmd;
	hr = pCtl->KsProperty((PKSPROPERTY)&cameraControl, ksLength, &cameraControl, dataLength, &valueSize); 

	SAFE_RELEASE(pCtl);

	return (TRUE);
}

HRESULT InitUVC()
{
	HRESULT hr;
	hr = CoCreateInstance(CLSID_SystemDeviceEnum, NULL, CLSCTX_INPROC, IID_ICreateDevEnum, (void **)&pDevEnum);

	if (FAILED(hr))
		return hr;

 DevNum=0;
	hr = pDevEnum->CreateClassEnumerator(CLSID_VideoInputDeviceCategory, &pClassEnum, 0);

	if (hr == S_OK)
	{
		ULONG cFetched;
		VARIANT varName;
		VariantInit(&varName);
		while (pClassEnum->Next(1, &pMoniker, &cFetched) == S_OK)     
		{
			IPropertyBag *pProgBag;
			hr = pMoniker->BindToStorage(0, 0, IID_IPropertyBag, (void**)&pProgBag);
			if (SUCCEEDED(hr))
			{
				hr = pProgBag->Read(L"FriendlyName", &varName, 0);
			 DevName[DevNum]=varName.bstrVal ;
             DevNum++;
			}
			hr = pMoniker->BindToObject(0, 0, IID_IBaseFilter, (void**)&pSrc);
			pMoniker->Release(); 
		}
		//else
		//{
		//	AfxMessageBox(_T("No device found"));
		//}
		pClassEnum->Release();
	}
	CoCreateInstance(CLSID_CaptureGraphBuilder2, 0, CLSCTX_INPROC_SERVER, IID_ICaptureGraphBuilder2, (void**)&pBuilder);
	CoCreateInstance(CLSID_FilterGraph, NULL, CLSCTX_INPROC_SERVER, IID_IGraphBuilder, (void **)&pGraph); //Create Filter Graph
	pBuilder->SetFiltergraph(pGraph);
	hr = pGraph->QueryInterface(IID_IVideoWindow, (void**)&pWindow);
	hr = pGraph->QueryInterface(IID_IMediaControl,(void**)&pMediaControl);

	return hr;
}



HRESULT OpenUVC(int devc)
{
	HRESULT hr;
	hr = CoCreateInstance(CLSID_SystemDeviceEnum, NULL, CLSCTX_INPROC, IID_ICreateDevEnum, (void **)&pDevEnum);

	if (FAILED(hr))
		return hr;

 int Num=0;
	hr = pDevEnum->CreateClassEnumerator(CLSID_VideoInputDeviceCategory, &pClassEnum, 0);

	if (hr == S_OK)
	{
		ULONG cFetched;
		VARIANT varName;
		VariantInit(&varName);
		while (pClassEnum->Next(1, &pMoniker, &cFetched) == S_OK)     
		{
			IPropertyBag *pProgBag;
			hr = pMoniker->BindToStorage(0, 0, IID_IPropertyBag, (void**)&pProgBag);
			if (SUCCEEDED(hr))
			{
				hr = pProgBag->Read(L"FriendlyName", &varName, 0);
              Num++;
			}
			hr = pMoniker->BindToObject(0, 0, IID_IBaseFilter, (void**)&pSrc);
			pMoniker->Release(); 
			if (devc==Num) break;
		}
		//else
		//{
		//	AfxMessageBox(_T("No device found"));
		//}
		pClassEnum->Release();
	}
	CoCreateInstance(CLSID_CaptureGraphBuilder2, 0, CLSCTX_INPROC_SERVER, IID_ICaptureGraphBuilder2, (void**)&pBuilder);
	CoCreateInstance(CLSID_FilterGraph, NULL, CLSCTX_INPROC_SERVER, IID_IGraphBuilder, (void **)&pGraph); //Create Filter Graph
	pBuilder->SetFiltergraph(pGraph);
	hr = pGraph->QueryInterface(IID_IVideoWindow, (void**)&pWindow);
	hr = pGraph->QueryInterface(IID_IMediaControl,(void**)&pMediaControl);

	return hr;
}

BOOL  DllOpen(int devc)
{
	AM_MEDIA_TYPE   mt;
	ZeroMemory(&mt, sizeof(AM_MEDIA_TYPE));
	mt.majortype  = MEDIATYPE_Video;
	mt.subtype = MEDIASUBTYPE_YUY2;
	mt.formattype = FORMAT_VideoInfo;

	SAFE_RELEASE(pMediaControl);
	SAFE_RELEASE(pGraph);
	SAFE_RELEASE(pBuilder);
	SAFE_RELEASE(pWindow);

	HRESULT hr;
	hr = OpenUVC(devc);
	hr = CoCreateInstance(CLSID_SampleGrabber, NULL, CLSCTX_INPROC_SERVER, IID_IBaseFilter, (LPVOID *)&pGrabFilter);
	hr = CoCreateInstance(CLSID_NullRenderer, NULL, CLSCTX_INPROC_SERVER, IID_IBaseFilter, (LPVOID*) &pWrite);

	hr = pGrabFilter->QueryInterface(IID_ISampleGrabber, (void**)&pGrabber);
	hr = pGrabber->SetMediaType(&mt);

	pGraph->AddFilter(pSrc, L"Capture Filter");
	pGraph->AddFilter(pGrabFilter, L"Sample Grabber");
	pGraph->AddFilter(pWrite, L"Null Renderer");

	hr = ConnectFilters(pGraph, pSrc, pGrabFilter);
	hr = ConnectFilters(pGraph, pGrabFilter, pWrite);

	hr = pGrabber->GetConnectedMediaType(&mt);


	VIDEOINFOHEADER * vih = (VIDEOINFOHEADER*) mt.pbFormat;  
	mCB.m_lWidth = vih->bmiHeader.biWidth;  
	mCB.m_lHeight = vih->bmiHeader.biHeight; 

	WIDTH=mCB.m_lWidth ;
	HEIGHT=mCB.m_lHeight;

	if(rowadd) HEIGHT-=4;

	Datalength=WIDTH *HEIGHT;

	mCB.m_bGetPicture = true;

	hr = pGrabber->SetBufferSamples( FALSE ); //TRUE
	hr = pGrabber->SetOneShot( FALSE );
	hr = pGrabber->SetCallback( &mCB, 1 );


   return TRUE;
}



BOOL  DllInit(void *pHandle)
{

	 pMediaControl=NULL;
	 pGraph=NULL;
	 pBuilder=NULL;
	 pWindow=NULL;

	 pSrc=NULL;

 
        return TRUE;
}

BOOL  DllSet(void *pHandle,void *pParam)
{
	//HCG *hcgG =(HCG *)pHandle;
	BYTE *pBuff = (BYTE *)pParam;

	WORD temp;
	memcpy(&temp, pParam, sizeof(WORD));
	if (NULL == pSrc)
		return FALSE;
	UvcControl(pSrc, temp, &pBuff[1]);

	return TRUE;
}

//
//BOOL DllCapture(void *pHandle,BYTE* pData)
//{
//
// if(ppBuffer)
// {
//   pData = ppBuffer;
//   ppBuffer=NULL;
//   return TRUE;
// }
// else
//      return FALSE;
//
//}

BOOL DllStart(void *pHandle)
{
    if(pMediaControl)
	{
		pMediaControl->Run();
       return TRUE;
	}
	else
		return FALSE;
}

BOOL DllStop(void *pHandle)
{

  if(pMediaControl) 
  {
	  pMediaControl->Stop();
	 return TRUE;
  }
  else
	  return FALSE;
}

BOOL DllClose(void *pHandle)
{
 
		
	SAFE_RELEASE(pMediaControl);
	SAFE_RELEASE(pGraph);
	SAFE_RELEASE(pBuilder);
	SAFE_RELEASE(pWindow);

  return TRUE;
}

BOOL   DllCallback(void *pHandle,CallBack  Func)
{
  if(Func)
  {
	  gCallBack=Func;
	  return TRUE;
  }
  else
	  return FALSE;
   
}





int convert_yuv_to_rgb_pixel(int y, int u, int v)
{
        unsigned int pixel32 = 0;
        unsigned char *pixel = (unsigned char *)&pixel32;
        int r, g, b;
        r = y + (1.370705 * (v-128));
        g = y - (0.698001 * (v-128)) - (0.337633 * (u-128));
        b = y + (1.732446 * (u-128));
        if(r > 255) r = 255;
        if(g > 255) g = 255;
        if(b > 255) b = 255;
        if(r < 0) r = 0;
        if(g < 0) g = 0;
        if(b < 0) b = 0;
        pixel[0] = r ;
        pixel[1] = g ;
        pixel[2] = b ;
        return pixel32;
}

int convert_yuv_to_rgb_buffer(unsigned char *yuv, unsigned char *rgb, unsigned int width, unsigned int height)
{
        unsigned int in, out = 0;
        unsigned int pixel_16;
        unsigned char pixel_24[3];
        unsigned int pixel32;
        int y0, u, y1, v;

        for(in = 0; in < width * height * 2; in += 4)
        {
                pixel_16 =
                                yuv[in + 3] << 24 |
                                yuv[in + 2] << 16 |
                                yuv[in + 1] <<  8 |
                                yuv[in + 0];
                y0 = (pixel_16 & 0x000000ff);
                u  = (pixel_16 & 0x0000ff00) >>  8;
                y1 = (pixel_16 & 0x00ff0000) >> 16;
                v  = (pixel_16 & 0xff000000) >> 24;
                pixel32 = convert_yuv_to_rgb_pixel(y0, u, v);
                pixel_24[0] = (pixel32 & 0x000000ff);
                pixel_24[1] = (pixel32 & 0x0000ff00) >> 8;
                pixel_24[2] = (pixel32 & 0x00ff0000) >> 16;
                rgb[out++] = pixel_24[2];
                rgb[out++] = pixel_24[1];
                rgb[out++] = pixel_24[0];
                pixel32 = convert_yuv_to_rgb_pixel(y1, u, v);
                pixel_24[0] = (pixel32 & 0x000000ff);
                pixel_24[1] = (pixel32 & 0x0000ff00) >> 8;
                pixel_24[2] = (pixel32 & 0x00ff0000) >> 16;
                rgb[out++] = pixel_24[2];
                rgb[out++] = pixel_24[1];
                rgb[out++] = pixel_24[0];
        }
        return 0;

}



void onShowLoc(short x,short y,short x1,short y1)
{

 TMPPOS tmp;
tmp.x=rectST.Width()/2;
tmp.y=rectST.Height() /2;

CString str;

LOGFONT logfont;       //改变输出字体  
ZeroMemory(&logfont, sizeof(LOGFONT));  
logfont.lfCharSet = GB2312_CHARSET;  
logfont.lfHeight = 50;      //设置字体的大小  
HFONT hFont =CreateFontIndirect(&logfont);  
  
SelectObject(pDCDs->m_hDC,hFont );

     	CPen pen; 

	pen.CreatePen(PS_SOLID,1,RGB(0,255,255)); 
	pDCDs->SelectObject( &pen );
	////换成红色
	//pen.DeleteObject() ;   //*//
	//pen.CreatePen(PS_SOLID,1,RGB(255,0,0)); 
	//pDC->SelectObject( &pen );

		SetTextColor(pDCDs->m_hDC, RGB(0,255,255));  
		
    str.Format("%0.1f",tempcenter);
              pDCDs->TextOut(tmp.x,tmp.y,str);

			  pDCDs->MoveTo(tmp.x,tmp.y-LEL);   

			  pDCDs->LineTo(tmp.x,tmp.y+LEL);
		
			  pDCDs->MoveTo(tmp.x-LEL,tmp.y);    

	  
			  pDCDs->LineTo(tmp.x+LEL,tmp.y);  

			 

 tmp.x=rectST.Width()*x/WIDTH;
 tmp.y=rectST.Height()*y /HEIGHT;

			  	pen.DeleteObject() ;   //*//
	pen.CreatePen(PS_SOLID,1,RGB(255,0,0)); 
	pDCDs->SelectObject( &pen );

	SetTextColor(pDCDs->m_hDC, RGB(255, 0, 0));  

str.Format("%0.1f",tempmax);
              pDCDs->TextOut(tmp.x,tmp.y,str);

			  pDCDs->MoveTo(tmp.x,tmp.y-LEL);   
			  pDCDs->LineTo(tmp.x,tmp.y+LEL);
			  pDCDs->MoveTo(tmp.x-LEL,tmp.y);    
			  pDCDs->LineTo(tmp.x+LEL,tmp.y);  

			  pen.DeleteObject() ;   //*//

 tmp.x=rectST.Width()*x1/WIDTH;
 tmp.y=rectST.Height()*y1 /HEIGHT;

	pen.CreatePen(PS_SOLID,1,RGB(0,0,255)); 
	pDCDs->SelectObject( &pen );
	SetTextColor(pDCDs->m_hDC, RGB(0, 0, 255)); 
	str.Format("%0.1f",tempmin);
              pDCDs->TextOut(tmp.x,tmp.y,str);
			  pDCDs->MoveTo(tmp.x,tmp.y-LEL);   
			  pDCDs->LineTo(tmp.x,tmp.y+LEL);
			  pDCDs->MoveTo(tmp.x-LEL,tmp.y);    
			  pDCDs->LineTo(tmp.x+LEL,tmp.y);  



   pen.DeleteObject() ;	
   DeleteObject(hFont);
}


void OnVideoDraw(unsigned char *data)  //图像转换实现函数
{
 
BITMAPINFO *m_pBMI;
CBitmap bmp;
HANDLE hdib;
unsigned char *BmpData=new unsigned char[3*Datalength];
int biBitCount;//图像类型

biBitCount=24;
 int lineByte=(WIDTH * biBitCount/8+3)/4*4;
  //if(SRC==3)  //UYVY2BMP(BmpData,data);
  convert_yuv_to_rgb_buffer(data,BmpData,WIDTH,HEIGHT);
	
//  else
//  {
//	  for(long i=0;i<Datalength;i++)
//    {
////   memcpy(&BmpData[3*i],&ColorMap[data[i]*3],3);//data[i];
//	   BmpData[3*i+0]=data[i];//data[i];
//       BmpData[3*i+1]=data[i];
//       BmpData[3*i+2]=data[i];
//	  }
//   }


m_pBMI=new BITMAPINFO;//生成彩色图像的信息头
m_pBMI->bmiHeader.biBitCount=biBitCount;
m_pBMI->bmiHeader.biClrImportant=0;
m_pBMI->bmiHeader.biClrUsed=0;
m_pBMI->bmiHeader.biCompression=BI_RGB;
m_pBMI->bmiHeader.biHeight=-HEIGHT;
m_pBMI->bmiHeader.biWidth=WIDTH;
m_pBMI->bmiHeader.biPlanes=1;
m_pBMI->bmiHeader.biSize=sizeof(BITMAPINFOHEADER);
m_pBMI->bmiHeader.biXPelsPerMeter=0;
m_pBMI->bmiHeader.biYPelsPerMeter=0;
m_pBMI->bmiHeader.biSizeImage=lineByte*HEIGHT;


    CDC memDC;  
  BOOL flg=  memDC.CreateCompatibleDC(pDCDs);  
 if(flg)
 {
   bmp.CreateCompatibleBitmap(pDCDs, WIDTH,HEIGHT);  
   memDC.SelectObject(&bmp);  
      
  
    SetDIBits(pDCDs->m_hDC, bmp, 0, abs(m_pBMI->bmiHeader.biHeight), BmpData,m_pBMI, DIB_RGB_COLORS);  
 


	pDCDs->StretchBlt(0,0,rectST.Width(),rectST.Height(),&memDC,0,0,WIDTH,HEIGHT,SRCCOPY);

   memDC.DeleteDC();
   bmp.DeleteObject();
 }
delete m_pBMI;
delete BmpData;

}


void  SaveRawData(unsigned short* m_pRawBuffer,unsigned long ImageSize ,CString Bla )
{
	FILE      *stream; 
    int       i;   
    CString   str; 
	unsigned char TempData;
		ImageSize++;
     if((stream=fopen(SFilePath+Bla, "w+t ")) != NULL) 
	 { 
 	  		  for(i=1;i<ImageSize;i++) 
			  {
				  fprintf(stream, "%d\t",  m_pRawBuffer[i-1]); 
				  	if((i%WIDTH)==0) fprintf(stream, "\n");

			  }

  	        
	  fclose(stream); 
	 } 
}
void  SaveTempData(float* m_pRawBuffer,unsigned long ImageSize ,CString Bla )
{
	FILE      *stream; 
    int       i;   
    CString   str; 
	unsigned char TempData;
	ImageSize++;
     if((stream=fopen(SFilePath+Bla, "w+t ")) != NULL) 
	 { 
 	  		  for(i=1;i<ImageSize;i++) 
			  {
				  fprintf(stream, "%0.1f\t",  m_pRawBuffer[i-1]); 
				  	if((i%WIDTH)==0) fprintf(stream, "\n");

			  }

  	        
	  fclose(stream); 
	 } 
}
void TmpThread(void *arg)
{

     	CString str;
 GetTmpData(ParaFlag,(BYTE*) arg,&tempmax,&m_pMax.x,&m_pMax.y,&tempmin,&m_pMin.x,&m_pMin.y,&tempcenter,postmp,Alltemp);
 int fpaavg,outavg;
 float fpatmp,coretemp;
 GetDevData(&fpatmp,&coretemp,&fpaavg,&outavg);


		str.Format("fpatemp:%.1f,devtmp:%.1f,fpaAvg:%d,ave:%d--max:%.1f℃,min:%.1f℃,center:%.1f℃",fpatmp,coretemp,fpaavg,outavg,tempmax,tempmin,tempcenter);
	//str.Format("time :%d",t2);
		//m_cyDisplaydrc->SetWindowTitle(str);
			char m_port[255]="";

 //   FILE *stream=fopen(SFilePath+"info.log","w");
 // if(stream)
 // {
	//fprintf(stream,"%s,", str); 

	//	   fprintf(stream, "\n");
	//	   fclose(stream); 
 // }

	sscanf(str,"%s",m_port);
		::SendMessage(hWndST, WM_SETTEXT,0,(LPARAM)m_port); 
}


int Compress(void *aBuffer)
{
	//CyAssert( mCompressing );
	//CyAssert( mTempBuffer != NULL );

	long lSamplesWritten, lBytesWritten;

	for( unsigned short lLine = 0; lLine < mSizeY; lLine++ )
	{
		unsigned char *lCurLine    = (unsigned char *)aBuffer + (lLine             ) * mSizeX * mBPP;
		unsigned char *lCurLineInv = mTempBuffer              + (mSizeY - lLine - 1) * mSizeX * mBPP;
		memcpy( lCurLineInv, lCurLine, mSizeX * mBPP );
	}

	if(mTempBuffer == NULL || AVIStreamWrite( mAVICompressedStream, mLastSample, 1, mTempBuffer, mImageSize, 0, 
			&lSamplesWritten, &lBytesWritten ) != 0 ||
		lSamplesWritten < 1 ||
		lBytesWritten < 1 )
	{
		return 1;/*SetErrorInfo( "Cannot compress image!",
						   CY_RESULT_UNEXPECTED_EXCEPTION,
						   __FILE__,
						   __LINE__,
						   GetLastError() );*/
	}

	mLastSample ++;

	return 0;
}

void SaveAVI(void *arg)
{
	BYTE * pData=(BYTE *)arg;
   BYTE * pDatas=new BYTE[WIDTH*HEIGHT*3];
  // int t;
  //for(int i=0;i<Datalength;i++)
  //{
  //     t=3*i;
	 //  pDatas[t]=pData[i];
	 //  pDatas[t+1]=pData[i];
  //     pDatas[t+2]=pData[i];
  //}
  convert_yuv_to_rgb_buffer(pData,pDatas,WIDTH,HEIGHT);

		Compress( pDatas );

		delete []pDatas;
}


void  UVCDRCThread(void *arg) 
{
 
  BYTE * pDataGet =(BYTE *)arg;
   //WORD NUCforTmp[200];
 if(tempflag)
 {
	 if(!Tmpflag)
	 {

         Tmpflag=1;
     //    memcpy(NUCforTmp,&pDataGet[2*Datalength],200);
		 UpdateParam(temptype,pDataGet);
          GetFixParam(&emiss,&Refltmp,&Airtmp,&humi,&distance,&fix );
	 }

		 _beginthread(TmpThread,0,pDataGet); 
 }
	 if(saveflag)
	 {

		 if(saveflag==1)
			  SaveTempData(Alltemp,WIDTH*HEIGHT,"temperture.txt");
		 if(saveflag==2)
			 SaveRawData((WORD *)pDataGet,WIDTH*HEIGHT,"Raw.txt");

		 ParaFlag=1;
     saveflag=0;
 	frate=0x8005;
  
    if(DllSet)
	    BOOL  status= DllSet((void *)&hWin,(void *)&frate);

	 }

//char Ver[64]={0};
// if(WIDTH==384)
//  memcpy(Ver,&NUCforTmp[24+3*WIDTH],64);
// else
//  memcpy(Ver,&NUCforTmp[24+WIDTH],16);

    m_hVideothread=NULL;

	if(ParaFlag)


 	OnVideoDraw(pDataGet);

if(tempflag)	onShowLoc(m_pMax.x,m_pMax.y,m_pMin.x,m_pMin.y);
 
	
  if (mCompressing)

	   {
 
		  _beginthread(SaveAVI,0,pDataGet);  
	   }


	return;

}

void CALLBACK UVCProc( 
BYTE * pData) 
{
  if(m_hVideothread==NULL)
  {
          m_hVideothread=(HANDLE) _beginthread(UVCDRCThread,0,pData);
   }

 //  t1++;
//if(t1==10)
//{
// 	tp=GetTickCount();
//	t1=tp;
//    tp-=t;
//	t=t1;
//	t1=0;
//	if(tp)
//	{
//CString str;
//		str.Format("帧频：%.2fHz",10000.0/tp);
// 			char m_port[255]="";
//
// 
//	sscanf(str,"%s",m_port);
//		::SendMessage(hWndST, WM_SETTEXT,0,(LPARAM)m_port); 
//	}
//}

}


CXthermDemoDlg::CXthermDemoDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CXthermDemoDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CXthermDemoDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CXthermDemoDlg, CDialog)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	//}}AFX_MSG_MAP
	ON_BN_CLICKED(IDC_BUT_REFL, &CXthermDemoDlg::OnBnClickedButRefl)
	ON_BN_CLICKED(IDC_BUT_START, &CXthermDemoDlg::OnBnClickedButStart)
	ON_BN_CLICKED(IDC_BUT_STOP, &CXthermDemoDlg::OnBnClickedButStop)
	ON_CBN_SELCHANGE(IDC_COMBO_COLOR, &CXthermDemoDlg::OnCbnSelchangeComboColor)
	ON_BN_CLICKED(IDC_CHECK_TMPTYPE, &CXthermDemoDlg::OnBnClickedCheckTmptype)
	ON_CBN_SELCHANGE(IDC_COMBO_SRC, &CXthermDemoDlg::OnCbnSelchangeComboSrc)
 	ON_BN_CLICKED(IDC_BUT_TMP, &CXthermDemoDlg::OnBnClickedButTmp)
	ON_BN_CLICKED(IDC_BUT_NUC, &CXthermDemoDlg::OnBnClickedButNuc)
	ON_CBN_SELCHANGE(IDC_COMBO_CARD, &CXthermDemoDlg::OnCbnSelchangeComboCard)
	ON_BN_CLICKED(IDC_BUT_POS1, &CXthermDemoDlg::OnBnClickedButPos1)
	ON_BN_CLICKED(IDC_BUT_TMPSET, &CXthermDemoDlg::OnBnClickedButTmpset)
	ON_BN_CLICKED(IDC_BUT_SAVET, &CXthermDemoDlg::OnBnClickedButSavet)
	ON_BN_CLICKED(IDC_BUT_SAVEORG, &CXthermDemoDlg::OnBnClickedButSaveorg)
	ON_BN_CLICKED(IDC_BUT_TEST, &CXthermDemoDlg::OnBnClickedButTest)
	ON_BN_CLICKED(IDC_CHECK_RECORD, &CXthermDemoDlg::OnBnClickedCheckRecord)
	ON_BN_CLICKED(IDC_CHECK_TEMP, &CXthermDemoDlg::OnBnClickedCheckTemp)
END_MESSAGE_MAP()


// CXthermDemoDlg 消息处理程序

BOOL CXthermDemoDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// 将“关于...”菜单项添加到系统菜单中。

	// IDM_ABOUTBOX 必须在系统命令范围内。
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		CString strAboutMenu;
		strAboutMenu.LoadString(IDS_ABOUTBOX);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// 设置此对话框的图标。当应用程序主窗口不是对话框时，框架将自动
	//  执行此操作
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标

	// TODO: 在此添加额外的初始化代码

   pDCDs=GetDlgItem(IDC_STATIC_SHOW)->GetWindowDC();
   
   SetBkMode(pDCDs->m_hDC, TRANSPARENT); 

     GetDlgItem(IDC_STATIC_SHOW)->GetClientRect(&rectST); 

	 hWndST=GetDlgItem(IDC_STATIC_ST)->m_hWnd;


	    GetDlgItem(IDC_EDIT_EMISS)->SetWindowText("0.98");
   GetDlgItem(IDC_EDIT_FIXTMP)->SetWindowText("0.0");
      GetDlgItem(IDC_EDIT_TMP)->SetWindowText("25.0");
      GetDlgItem(IDC_EDIT_AIR)->SetWindowText("25.0");
      GetDlgItem(IDC_EDIT_HMI)->SetWindowText("0.45");
       GetDlgItem(IDC_STATIC_ST)->SetWindowText("");

	  
   GetDlgItem(IDC_EDIT_DIS)->SetWindowText("3");

     ((CComboBox*)GetDlgItem(IDC_COMBO_SRC))->SetCurSel(1);
     ((CComboBox*)GetDlgItem(IDC_COMBO_COLOR))->SetCurSel(0);


	   CString des="";
  GetModuleFileName(NULL,des.GetBuffer(255),255);//得到程序模块名称，全路径
  des.ReleaseBuffer();
  des = des.Left(des.ReverseFind('\\'));
	CString filename;
	  filename=des;//filename.Format("%s",path);
	SFilePath=filename+"\\";



	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

void CXthermDemoDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialog::OnSysCommand(nID, lParam);
	}
}

// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。

void CXthermDemoDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 用于绘制的设备上下文

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// 使图标在工作矩形中居中
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// 绘制图标
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();
	}
}

//当用户拖动最小化窗口时系统调用此函数取得光标显示。
//
HCURSOR CXthermDemoDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}


void CXthermDemoDlg::OnBnClickedButRefl()
{

  InitUVC();

		((CComboBox*)GetDlgItem(IDC_COMBO_CARD))->ResetContent();

for(int i=0;i<DevNum;i++)
{
	CString str;
	str.Format("%d:",i);
	str+=DevName[i];
	((CComboBox*)GetDlgItem(IDC_COMBO_CARD))->AddString(str);
}
	((CComboBox*)GetDlgItem(IDC_COMBO_CARD))->SetCurSel(0);
}

void CXthermDemoDlg::OnBnClickedButStart()
{
	// TODO: 在此添加控件通知处理程序代码
	GetDlgItem(IDC_BUT_START)->EnableWindow(false);

	  BOOL status=DllInit((void *)&hWin);
  status= DllCallback((void *)&hWin,(CallBack) UVCProc);
 
  status=DllOpen(cardtype);
   if(!status)
   {
	   AfxMessageBox(_T("连接失败!"));
      return;
   }

   DataInit(WIDTH,HEIGHT);
     Tmpflag=0;

	 Shutflag=1;
	frate=0x8000;

  if(DllSet)
	    BOOL  status= DllSet((void *)&hWin,(void *)&frate);

 
   status=DllStart((void *)&hWin);
   //SetTimer( 2, 1000, NULL );
   Connected=true;

}

void CXthermDemoDlg::OnBnClickedButStop()
{
	// TODO: 在此添加控件通知处理程序代码
BOOL  status= DllStop((void *)&hWin);

Connected=false;
  Sleep(1000);
	GetDlgItem(IDC_BUT_START)->EnableWindow(true);
}

void CXthermDemoDlg::OnCbnSelchangeComboColor()
{
    int st= ((CComboBox*)GetDlgItem(IDC_COMBO_COLOR))->GetCurSel();

 	frate=0x8800|(st);
	if(Connected)

    if(DllSet)
	    BOOL  status= DllSet((void *)&hWin,(void *)&frate);
}

void CXthermDemoDlg::OnBnClickedCheckTmptype()
{
	BOOL flag= ((CButton *)GetDlgItem(IDC_CHECK_TMPTYPE))->GetCheck();

	temptype=flag;

	if(flag)
	 frate=0x8021;
	else
     frate=0x8020;
	
	if(Connected)
		  if(DllSet)
	    BOOL  status= DllSet((void *)&hWin,(void *)&frate);
}

void CXthermDemoDlg::OnCbnSelchangeComboSrc()
{
	// TODO: 在此添加控件通知处理程序代码
    int st= ((CComboBox*)GetDlgItem(IDC_COMBO_SRC))->GetCurSel();

	ParaFlag=st;
 	frate=0x8000|(st+4);
 	//if(Connected)
		//  Tmpflag=0;

    if(DllSet)
	    BOOL  status= DllSet((void *)&hWin,(void *)&frate);
 }


void CXthermDemoDlg::OnBnClickedButTmp()
{
	// TODO: 在此添加控件通知处理程序代码

	CString str;
	   GetDlgItem(IDC_EDIT_EMISS)->GetWindowText(str);
   emiss=atof(str);
   GetDlgItem(IDC_EDIT_FIXTMP)->GetWindowText(str);
  fix=atof(str);
      GetDlgItem(IDC_EDIT_TMP)->GetWindowText(str);
    Refltmp=atof(str);
      GetDlgItem(IDC_EDIT_AIR)->GetWindowText(str);
    Airtmp=atof(str);

      GetDlgItem(IDC_EDIT_HMI)->GetWindowText(str);
    humi=atof(str);


   GetDlgItem(IDC_EDIT_DIS)->GetWindowText(str);
    distance=atoi(str);
   UpdateFixParam(emiss,Refltmp,Airtmp,humi,distance,fix );



OnBnClickedButNuc();
}

void SaveParam()
{
	char pData[128]={0};


	memcpy(pData,&fix,sizeof(float));
	memcpy(&pData[4],&Refltmp,sizeof(float));
	memcpy(&pData[8],&Airtmp,sizeof(float));
	memcpy(&pData[12],&humi,sizeof(float));
	memcpy(&pData[16],&emiss,sizeof(float));
	memcpy(&pData[20],&distance,sizeof(short));

	//if(Connected)
       for(int i=0;i<24;i++)
	   {
		   frate=(i<<8)|pData[i];
	    BOOL  status= DllSet((void *)&hWin,(void *)&frate);
	   }

}
void CXthermDemoDlg::OnBnClickedButNuc()
{
    Tmpflag=0;

 	frate=0x8000 ;
	if(Connected)
 
    if(DllSet)
	    BOOL  status= DllSet((void *)&hWin,(void *)&frate);
}

void CXthermDemoDlg::OnCbnSelchangeComboCard()
{
   cardtype=((CComboBox*)GetDlgItem(IDC_COMBO_CARD))->GetCurSel();
}



// 设置测温点位置

void Settemppixel1(int x,int y)
{

     int pos=0xf000|(0xffff&x);
	
		  if(DllSet)
	    BOOL  status= DllSet((void *)&hWin,(void *)&pos);
	
		  pos=0xf200|(0xffff&y);
	
		  if(DllSet)
	    BOOL  status= DllSet((void *)&hWin,(void *)&pos);

}

void Settemppixel2(int x,int y)
{

     int pos=0xf400|(0xffff&x);
	
		  if(DllSet)
	    BOOL  status= DllSet((void *)&hWin,(void *)&pos);
	
		  pos=0xf600|(0xffff&y);
	
		  if(DllSet)
	    BOOL  status= DllSet((void *)&hWin,(void *)&pos);

}
void Settemppixel3(int x,int y)
{

     int pos=0xf800|(0xffff&x);
	
		  if(DllSet)
	    BOOL  status= DllSet((void *)&hWin,(void *)&pos);
	
		  pos=0xfa00|(0xffff&y);
	
		  if(DllSet)
	    BOOL  status= DllSet((void *)&hWin,(void *)&pos);

}
void CXthermDemoDlg::OnBnClickedButPos1()
{
	// TODO: 在此添加控件通知处理程序代码

	Settemppixel1(10,10);
}

void CXthermDemoDlg::OnBnClickedButTmpset()
{

    frate=41;
 	frate=0x8000|frate;

   if(DllSet)
	    BOOL  status= DllSet((void *)&hWin,(void *)&frate);


    	frate=0x80fe;
     if(DllSet)
	    BOOL  status= DllSet((void *)&hWin,(void *)&frate);

}

void CXthermDemoDlg::OnBnClickedButSavet()
{
	// TODO: 在此添加控件通知处理程序代码
 	frate=0x8004;
 	//if(Connected)
		//  Tmpflag=0;

    if(DllSet)
	    BOOL  status= DllSet((void *)&hWin,(void *)&frate);

	//Sleep(200);

	//ParaFlag=0;

 //   saveflag=1;
}

void CXthermDemoDlg::OnBnClickedButSaveorg()
{
	// TODO: 在此添加控件通知处理程序代码
 	frate=0x8004;
  
    if(DllSet)
	    BOOL  status= DllSet((void *)&hWin,(void *)&frate);

	Sleep(200);

	ParaFlag=0;
	saveflag=2;
}

void CXthermDemoDlg::OnBnClickedButTest()
{
	CString sztemp;
	GetDlgItem(IDC_EDIT_TEST)->GetWindowText(sztemp);
    frate=atoi(sztemp);
	//if(frate==0) Shutflag=1;
	frate=0x8000|frate;

	if(Connected)
  if(DllSet)
	    BOOL  status= DllSet((void *)&hWin,(void *)&frate);
}



int CXthermDemoDlg::ChooseCompressor(COMPVARS *aCOMPVARS, HWND aParent )
{
	COMPVARS lCOMPVARS;
	memcpy( &lCOMPVARS, aCOMPVARS, sizeof( lCOMPVARS ) );
	lCOMPVARS.cbSize = sizeof( lCOMPVARS );

	// prompt the user for the compressor to use
	if( !ICCompressorChoose( aParent, ICMF_CHOOSE_KEYFRAME, NULL, NULL, &lCOMPVARS, "Video Codec Selection" ) )
		return 1;

	memcpy( aCOMPVARS, &lCOMPVARS, sizeof( lCOMPVARS ) );

	return 0;
}

int CXthermDemoDlg::GetCompressorName(COMPVARS *aCOMPVARS, CString &aName)
{
//	CyAssert( aCOMPVARS != NULL );

	// init IC info
	ICINFO lICINFO;
	memset( &lICINFO, 0, sizeof( lICINFO ) );
	lICINFO.dwSize = sizeof( lICINFO );

	// if there are is no compressor selected, return an empty string
	if( aCOMPVARS->fccType == 0 || aCOMPVARS->fccHandler == 0 )
	{
		aName = "";
		return 0;
	}

	// first check if this is the Uncompressed Frames selection ('DIB ')
	if( mmioFOURCC('D','I','B',' ') == aCOMPVARS->fccHandler )
	{
		aName = "Full Frames (Uncompressed)";
		return 0;
	}

	HIC lIC = ICOpen( aCOMPVARS->fccType, aCOMPVARS->fccHandler, ICMODE_QUERY );

	if( lIC == NULL )
	{
		//throw CyErrorInfo( "Cannot open the video compressor!", 33,
		//	__FILE__, __LINE__, GetLastError() );
	}

	if( ICGetInfo( lIC, &lICINFO, sizeof( lICINFO ) ) == 0 )
	{
		//throw CyErrorInfo( "Cannot get compressor information!", 33,
		//	__FILE__, __LINE__, GetLastError() );
	}

	if( ICClose( lIC ) != ICERR_OK )
	{
		//throw CyErrorInfo( "Cannot close compressor!", 33,
		//	__FILE__, __LINE__, GetLastError() );
	}

	aName = "";
	for( int lIdx = 0; lIdx < sizeof( lICINFO.szDescription ) && lICINFO.szDescription[ lIdx ] != 0; lIdx++ )
	{
		aName += (char)lICINFO.szDescription[ lIdx ];
	}

	return 0;
}

//int Compress(void *aBuffer)
//{
//	//CyAssert( mCompressing );
//	//CyAssert( mTempBuffer != NULL );
//
//	long lSamplesWritten, lBytesWritten;
//
//	for( unsigned short lLine = 0; lLine < mSizeY; lLine++ )
//	{
//		unsigned char *lCurLine    = (unsigned char *)aBuffer + (lLine             ) * mSizeX * mBPP;
//		unsigned char *lCurLineInv = mTempBuffer              + (mSizeY - lLine - 1) * mSizeX * mBPP;
//		memcpy( lCurLineInv, lCurLine, mSizeX * mBPP );
//	}
//
//	if(mTempBuffer == NULL || AVIStreamWrite( mAVICompressedStream, mLastSample, 1, mTempBuffer, mImageSize, 0, 
//			&lSamplesWritten, &lBytesWritten ) != 0 ||
//		lSamplesWritten < 1 ||
//		lBytesWritten < 1 )
//	{
//		return 1;/*SetErrorInfo( "Cannot compress image!",
//						   CY_RESULT_UNEXPECTED_EXCEPTION,
//						   __FILE__,
//						   __LINE__,
//						   GetLastError() );*/
//	}
//
//	mLastSample ++;
//
//	return 0;
//}



int Start(const CString& aFileName, unsigned short aSizeX, unsigned short aSizeY, unsigned short aBPP, double aFPS)
{
	IAVIFile *lAVIFile = NULL;
	IAVIStream *lAVIStream = NULL;
	IAVIStream *lAVICompressedStream = NULL;
	AVISTREAMINFO lAVISTREAMINFO;
	AVICOMPRESSOPTIONS lAVICOMPRESSOPTIONS;

	//try
	{
		// Try to match the image format with the Video Compressor capabilities
		BITMAPINFO lTempBI;
		lTempBI.bmiHeader.biSize          = sizeof( BITMAPINFO );
		lTempBI.bmiHeader.biWidth         = aSizeX;
		lTempBI.bmiHeader.biHeight        = aSizeY;
		lTempBI.bmiHeader.biPlanes        = 1;
		lTempBI.bmiHeader.biBitCount      = aBPP * 8;
		lTempBI.bmiHeader.biCompression   = BI_RGB;
		lTempBI.bmiHeader.biSizeImage     = aSizeX * aSizeY * aBPP;
		lTempBI.bmiHeader.biXPelsPerMeter = 10000;
		lTempBI.bmiHeader.biYPelsPerMeter = 10000;
		lTempBI.bmiHeader.biClrUsed       = 0;
		lTempBI.bmiHeader.biClrImportant  = 0;

		if( ( mCOMPVARS.hic != NULL ) && // if not the "Full Frames (uncompressed)"
			( ICCompressQuery( mCOMPVARS.hic, &lTempBI, NULL ) != ICERR_OK ) )
		{
			// clean-up
		if( lAVICompressedStream ) 
		{
			AVIStreamRelease( lAVICompressedStream );
			lAVICompressedStream = NULL;
		}

		if( lAVIStream )
		{
			AVIStreamRelease( lAVIStream );
			lAVIStream = NULL;
		}

		if( lAVIFile )
		{
			AVIFileRelease( lAVIFile );
			lAVIFile = NULL;
		}

		return 1;		//throw CyErrorInfo( "Image format not accepted by compressor!",
			//				   CY_RESULT_UNEXPECTED_EXCEPTION, __FILE__, __LINE__, GetLastError() );
		}


		// Try to open the stream for writing
		if( mTempBuffer )
			delete [] mTempBuffer;

		mTempBuffer = new unsigned char[ aSizeX * aSizeY * aBPP ];
		if( mTempBuffer == NULL )
		{
		// clean-up
		if( lAVICompressedStream ) 
		{
			AVIStreamRelease( lAVICompressedStream );
			lAVICompressedStream = NULL;
		}

		if( lAVIStream )
		{
			AVIStreamRelease( lAVIStream );
			lAVIStream = NULL;
		}

		if( lAVIFile )
		{
			AVIFileRelease( lAVIFile );
			lAVIFile = NULL;
		}

		return 1;			//throw CyErrorInfo( "Cannot allocate memory for a temporary buffer!",
			//				   CY_RESULT_CANNOT_OPEN_FILE, __FILE__, __LINE__, GetLastError() );
		}

 //int iii=(int) AVIFileOpen( &lAVIFile, "C:\Documents and Settings\Administrator\桌面\movie.avi", OF_CREATE | OF_WRITE, NULL );
	    CoInitialize(NULL);
		
		if( AVIFileOpen( &lAVIFile, aFileName, OF_CREATE | OF_WRITE, NULL ) != 0 )
		{
		// clean-up
		if( lAVICompressedStream ) 
		{
			AVIStreamRelease( lAVICompressedStream );
			lAVICompressedStream = NULL;
		}

		if( lAVIStream )
		{
			AVIStreamRelease( lAVIStream );
			lAVIStream = NULL;
		}

		if( lAVIFile )
		{
			AVIFileRelease( lAVIFile );
			lAVIFile = NULL;
		}

		return 1;			//throw CyErrorInfo( "Cannot open movie file for writing!",
			//				   CY_RESULT_CANNOT_OPEN_FILE, __FILE__, __LINE__, GetLastError() );
		}

		// Fill out AVIStream information
		memset( &lAVISTREAMINFO, 0, sizeof( AVISTREAMINFO ));
		lAVISTREAMINFO.fccType               = streamtypeVIDEO;
		lAVISTREAMINFO.fccHandler            = mCOMPVARS.fccHandler;
		lAVISTREAMINFO.dwFlags               = 0;
		lAVISTREAMINFO.dwCaps                = 0;
		lAVISTREAMINFO.wPriority             = 0;
		lAVISTREAMINFO.wLanguage             = 0;
		lAVISTREAMINFO.dwScale               = 100;
		lAVISTREAMINFO.dwRate                = (unsigned long)( aFPS * 100.0 );
		lAVISTREAMINFO.dwStart               = 0;
		lAVISTREAMINFO.dwLength              = 0;
		lAVISTREAMINFO.dwInitialFrames       = 0;
		lAVISTREAMINFO.dwQuality             = mCOMPVARS.lQ;
		lAVISTREAMINFO.dwSuggestedBufferSize = aSizeX * aSizeY * aBPP;
		lAVISTREAMINFO.dwSampleSize          = aSizeX * aSizeY * aBPP;
		SetRect(&lAVISTREAMINFO.rcFrame, 0, aSizeY, aSizeX, 0);
		strcpy( lAVISTREAMINFO.szName,  "Video Stream" );


		if( AVIFileCreateStream( lAVIFile, &lAVIStream, &lAVISTREAMINFO ) != 0 )
		{
		// clean-up
		if( lAVICompressedStream ) 
		{
			AVIStreamRelease( lAVICompressedStream );
			lAVICompressedStream = NULL;
		}

		if( lAVIStream )
		{
			AVIStreamRelease( lAVIStream );
			lAVIStream = NULL;
		}

		if( lAVIFile )
		{
			AVIFileRelease( lAVIFile );
			lAVIFile = NULL;
		}

		return 1;			//throw CyErrorInfo( "Cannot create video stream!",
			//				   CY_RESULT_UNEXPECTED_EXCEPTION, __FILE__, __LINE__, GetLastError() );
		}

		BITMAPINFOHEADER lBIH;
		lBIH.biSize          = sizeof( BITMAPINFOHEADER );
		lBIH.biWidth         = aSizeX;
		lBIH.biHeight        = aSizeY;
		lBIH.biPlanes        = 1;
		lBIH.biBitCount      = aBPP * 8;
		lBIH.biCompression   = BI_RGB;
		lBIH.biSizeImage     = aSizeX * aSizeY * aBPP;
		lBIH.biXPelsPerMeter = 10000;
		lBIH.biYPelsPerMeter = 10000;
		lBIH.biClrUsed       = 0;
		lBIH.biClrImportant  = 0;

		memset( &lAVICOMPRESSOPTIONS, 0, sizeof( AVICOMPRESSOPTIONS ) );
		lAVICOMPRESSOPTIONS.fccType           = streamtypeVIDEO;
		lAVICOMPRESSOPTIONS.fccHandler        = mCOMPVARS.fccHandler;
		lAVICOMPRESSOPTIONS.dwKeyFrameEvery   = 15;
		lAVICOMPRESSOPTIONS.dwQuality         = mCOMPVARS.lQ;
		lAVICOMPRESSOPTIONS.dwBytesPerSecond  = 0;
		lAVICOMPRESSOPTIONS.dwFlags           = AVICOMPRESSF_KEYFRAMES; //|AVICOMPRESSF_VALID;//|AVICOMPRESSF_DATARATE;
		lAVICOMPRESSOPTIONS.lpFormat          = &lBIH;
		lAVICOMPRESSOPTIONS.cbFormat          = sizeof( lBIH );
		lAVICOMPRESSOPTIONS.lpParms           = 0;
		lAVICOMPRESSOPTIONS.cbParms           = 0;
		lAVICOMPRESSOPTIONS.dwInterleaveEvery = 0;
		
		HRESULT lR = AVIMakeCompressedStream( &lAVICompressedStream, lAVIStream, &lAVICOMPRESSOPTIONS, NULL);
		if( lR == AVIERR_NOCOMPRESSOR )
		{
		// clean-up
		if( lAVICompressedStream ) 
		{
			AVIStreamRelease( lAVICompressedStream );
			lAVICompressedStream = NULL;
		}

		if( lAVIStream )
		{
			AVIStreamRelease( lAVIStream );
			lAVIStream = NULL;
		}

		if( lAVIFile )
		{
			AVIFileRelease( lAVIFile );
			lAVIFile = NULL;
		}

		return 1;			//throw CyErrorInfo( "Cannot find a suitable compressor!",
			//				   CY_RESULT_UNEXPECTED_EXCEPTION, __FILE__, __LINE__, GetLastError() );
		} 
		else if( lR == AVIERR_MEMORY )
		{
		// clean-up
		if( lAVICompressedStream ) 
		{
			AVIStreamRelease( lAVICompressedStream );
			lAVICompressedStream = NULL;
		}

		if( lAVIStream )
		{
			AVIStreamRelease( lAVIStream );
			lAVIStream = NULL;
		}

		if( lAVIFile )
		{
			AVIFileRelease( lAVIFile );
			lAVIFile = NULL;
		}

		return 1;			//throw CyErrorInfo( "Not enough memory to start the compressor!",
			//				   CY_RESULT_UNEXPECTED_EXCEPTION, __FILE__, __LINE__, GetLastError() );
		}
		else if( lR == AVIERR_UNSUPPORTED )
		{
		// clean-up
		if( lAVICompressedStream ) 
		{
			AVIStreamRelease( lAVICompressedStream );
			lAVICompressedStream = NULL;
		}

		if( lAVIStream )
		{
			AVIStreamRelease( lAVIStream );
			lAVIStream = NULL;
		}

		if( lAVIFile )
		{
			AVIFileRelease( lAVIFile );
			lAVIFile = NULL;
		}

		return 1;			//throw CyErrorInfo( "Compression is not supported for this image buffer!",
			//				   CY_RESULT_UNEXPECTED_EXCEPTION, __FILE__, __LINE__, GetLastError() );
		}

		if( AVIStreamSetFormat( lAVICompressedStream, 0, &lBIH, sizeof( lBIH ) ) != 0 )
		{
		// clean-up
		if( lAVICompressedStream ) 
		{
			AVIStreamRelease( lAVICompressedStream );
			lAVICompressedStream = NULL;
		}

		if( lAVIStream )
		{
			AVIStreamRelease( lAVIStream );
			lAVIStream = NULL;
		}

		if( lAVIFile )
		{
			AVIFileRelease( lAVIFile );
			lAVIFile = NULL;
		}

		return 1;			//throw CyErrorInfo( "Cannot set stream format. It probably isn't supported by the Codec!",
			//				   CY_RESULT_UNEXPECTED_EXCEPTION, __FILE__, __LINE__, GetLastError() );
		}
	}
	//catch( CyErrorInfo& lEI )
	{

	}

	// finishing up
	mAVIFile             = lAVIFile;
	mAVIStream           = lAVIStream;
	mAVICompressedStream = lAVICompressedStream;

	mSizeX     = aSizeX;
	mSizeY     = aSizeY;
	mBPP       = aBPP;
	mImageSize = aSizeX * aSizeY * aBPP;

	mLastSample = 0;

	mCompressing = true;

	return 0;
}


void CXthermDemoDlg::OnBnClickedCheckRecord()
{
	// TODO: 在此添加控件通知处理程序代码
	CButton * pBtn = NULL ;  
 
     pBtn = (CButton * )GetDlgItem(IDC_CHECK_RECORD);

	if (pBtn->GetCheck()) 
	{
		   	 CString Buffer;
			 char path[255]; 
			 GetCurrentDirectory( 255,path );//得到本进程当前的路径
			Buffer.Format ("%s\\Record%08lu.avi",path,ImageID); 
            ImageID++;

    if ( ChooseCompressor( &mCOMPVARS, GetSafeHwnd() ) != 0 )
		return;
   
    CString lCodecName;
     GetCompressorName( &mCOMPVARS, lCodecName );

  //  mFormatMovieCodec.SetWindowText( lCodecName );
	/*		 unsigned long lPixelTypeID = CyBGR32::ID;

        const CyPixelType* lDestPixel = CyPixelTypeFactory::CreatePixelType( lPixelTypeID );
        if( lDestPixel != NULL
            && CyPixelTypeFactory::HasConverter( m_cyImageBuffer->GetFormat().GetTypeID(), lDestPixel->GetTypeID() ) )
        {
            lConverter = &CyPixelTypeFactory::GetConverter(  m_cyImageBuffer->GetFormat().GetTypeID(), 
                lDestPixel->GetTypeID() );
        }
*/
        //lOutputBuffer = 
        //    new unsigned char[WIDTH*HEIGHT*6];
        //if ( lOutputBuffer == NULL )
        //    return;

		  if( Start( Buffer,WIDTH,HEIGHT,3,25) != 0 ) //lPerfMonitor.GetFrameRate() ) != CY_RESULT_OK )
		  {
			    MessageBox(Buffer,
	            "Cannot start Video Compressor!",
			    MB_OK | MB_ICONEXCLAMATION );
				    return;

		  }
	}
	else         //Stop AVI Save
	{
		Sleep(400);
 //   CyAssert( mTempBuffer != NULL );
	//CyAssert( mCompressing );

	mCompressing = false;


	AVIStreamRelease( mAVICompressedStream );
	mAVICompressedStream = NULL;

	AVIStreamRelease( mAVIStream );
	mAVIStream = NULL;

	AVIFileRelease( mAVIFile );
	mAVIFile = NULL;
	
	//delete [] mTempBuffer;
	//mTempBuffer = NULL;

	//if( mCOMPVARS.hic )
	//	ICClose( mCOMPVARS.hic );

	return;
	}
}

void CXthermDemoDlg::OnBnClickedCheckTemp()
{
	// TODO: 在此添加控件通知处理程序代码
	CButton * pBtn = NULL ;  
 
     pBtn = (CButton * )GetDlgItem(IDC_CHECK_TEMP);
	if (pBtn->GetCheck()) 
	    tempflag=1;
	else
		tempflag=0;

}

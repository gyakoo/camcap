// The MIT License (MIT)
// 
// Copyright (c) 2015 Manu Marin / @gyakoo
// 
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
// 
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#include <stdio.h>
#include <stdlib.h>
#include <tchar.h>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image_write.h>
#define CC_DSHOW
#define CC_IMPLEMENTATION
#include <camcap.hpp>

//===-----------------------------------------------------------===//
//===-----------------------------------------------------------===//
//class SampleGrabberCallback : public ISampleGrabberCB 
//{
//public:
//  SampleGrabberCallback() 
//  {
//    InitializeCriticalSection(&critSection);
//    freezeCheck = 0;
//    bufferSetup = false;
//    newFrame = false;
//    latestBufferLength = 0;
//    refcount = 1;
//    hEvent = CreateEvent(NULL, true, false, NULL);
//  }
//
//  ~SampleGrabberCallback() 
//  {
//    ptrBuffer = NULL;
//    DeleteCriticalSection(&critSection);
//    CloseHandle(hEvent);
//    SafeFree(pixels);
//  }
//
//  bool setupBuffer(int numBytesIn) 
//  {
//    if (bufferSetup) return false;
//    numBytes = numBytesIn;
//    pixels = (unsigned char*)malloc(numBytesIn);
//    if (!pixels) return false;
//    bufferSetup = true;
//    newFrame = false;
//    latestBufferLength = 0;
//    return true;
//  }
//
//  STDMETHODIMP_(ULONG) AddRef() 
//  { 
//    InterlockedIncrement(&refcount);
//    return (ULONG)refcount;
//  }
//  STDMETHODIMP_(ULONG) Release() 
//  { 
//    InterlockedDecrement(&refcount);
//    if ( !refcount )
//      delete this;
//    return (ULONG)refcount;
//  }
//  
//  STDMETHODIMP QueryInterface(REFIID riid, void **ppvObject) 
//  {
//    UNREFERENCED_PARAMETER(riid);
//    *ppvObject = static_cast<ISampleGrabberCB*>(this);
//    return S_OK;
//  }
//
//  //This method is meant to have less overhead
//  STDMETHODIMP SampleCB(double Time, IMediaSample *pSample) 
//  {
//    UNREFERENCED_PARAMETER(Time);
//    if (WaitForSingleObject(hEvent, 0) == WAIT_OBJECT_0) return S_OK;
//
//    HRESULT hr = pSample->GetPointer(&ptrBuffer);
//    if (hr == S_OK) 
//    {
//      latestBufferLength = pSample->GetActualDataLength();
//      if (latestBufferLength == numBytes) 
//      {
//        EnterCriticalSection(&critSection);
//        memcpy(pixels, ptrBuffer, latestBufferLength);
//        newFrame = true;
//        freezeCheck = 1;
//        LeaveCriticalSection(&critSection);
//        SetEvent(hEvent);
//      }
//      else 
//      {
//        printf("ERROR: SampleCB() - buffer sizes do not match\n");
//      }
//    }
//    return S_OK;
//  }
//
//  //This method is meant to have more overhead
//  STDMETHODIMP BufferCB(double Time, BYTE *pBuffer, long BufferLen) 
//  {
//    UNREFERENCED_PARAMETER(Time);
//    UNREFERENCED_PARAMETER(pBuffer);
//    UNREFERENCED_PARAMETER(BufferLen);
//    return E_NOTIMPL;
//  }
//
//  volatile unsigned long long refcount;
//  int freezeCheck;
//  int latestBufferLength;
//  int numBytes;
//  bool newFrame;
//  bool bufferSetup;
//  unsigned char * pixels;
//  unsigned char * ptrBuffer;
//  CRITICAL_SECTION critSection;
//  HANDLE hEvent;
//};

//===-----------------------------------------------------------===//
//===-----------------------------------------------------------===//
void CheckErrExit(HRESULT hr, const char* format, ...)
{
  if (FAILED(hr))
  {
    va_list args;
    fprintf(stderr, "Error: ");
    va_start(args, format);
    vfprintf(stderr, format, args);
    va_end(args);
    exit(E_FAIL);
  }
}

//===-----------------------------------------------------------===//
//===-----------------------------------------------------------===//
struct CaptureDeviceInfo
{
  CaptureDeviceInfo() 
    : name(NULL), desc(NULL), devpath(NULL), waveInId(0), pMoniker(NULL)
    , width(0), height(0), caps(NULL), capsCount(0)
  {
  }

  ~CaptureDeviceInfo() 
  { 
    Release(); 
  }

  void Release()
  {
    SafeFree(name);
    SafeFree(desc);
    SafeFree(devpath);
    waveInId = 0;
    SafeRelease(pMoniker);
    width = height = 0;
    SafeFree(caps);
    capsCount = 0;
  }

  wchar_t*     name;
  wchar_t*     desc;
  wchar_t*     devpath;
  unsigned int  waveInId;
  IMoniker* pMoniker;
  unsigned int width;
  unsigned int height;
  VIDEO_STREAM_CONFIG_CAPS* caps;
  unsigned int capsCount;
};

//===-----------------------------------------------------------===//
//===-----------------------------------------------------------===//
int ExtractDevicesInfo(REFGUID category, CaptureDeviceInfo* out_devices, unsigned int max_out_devs)
{
  if (!out_devices || !max_out_devs)
    return 0;

  // Create the System Device Enumerator.
  ICreateDevEnum *pDevEnum;
  HRESULT hr = CoCreateInstance(CLSID_SystemDeviceEnum, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&pDevEnum));
  if (FAILED(hr))
    return 0;  

  // Create an enumerator for the category.
  IEnumMoniker* pEnum = NULL;
  hr = pDevEnum->CreateClassEnumerator(category, &pEnum, 0);
  SafeRelease(pDevEnum);
  if (FAILED(hr) || hr == S_FALSE || !pEnum) // false is when the category is empty
    return 0;

  VARIANT varprop;
  IPropertyBag *pPropBag = NULL;
  IMoniker *pMoniker = NULL;
  unsigned int devCount = 0;
  CaptureDeviceInfo* devinfo=out_devices;

  // all devices of this enumerator
  while ( pEnum->Next(1, &pMoniker, NULL) == S_OK && devCount < max_out_devs )
  {
    // get properties
    if ( SUCCEEDED(pMoniker->BindToStorage(0,0,IID_IPropertyBag, (void**)&pPropBag)) )
    {
      devinfo->Release();

      VariantInit(&varprop);
      if ( SUCCEEDED(pPropBag->Read(L"Description", &varprop, 0)) )
      {
        devinfo->desc = BSTR2WChar(varprop.bstrVal);
        VariantClear(&varprop);
      }

      VariantInit(&varprop);
      if ( SUCCEEDED(pPropBag->Read(L"FriendlyName", &varprop, 0)) )
      {
        devinfo->name = BSTR2WChar(varprop.bstrVal);
        VariantClear(&varprop);
      }

      VariantInit(&varprop);
      if ( SUCCEEDED(pPropBag->Read(L"DevicePath", &varprop, 0)) )
      {
        devinfo->devpath = BSTR2WChar(varprop.bstrVal);
        VariantClear(&varprop);
      }

      VariantInit(&varprop);
      if ( SUCCEEDED(pPropBag->Read(L"WaveInID", &varprop, 0)) )
      {
        devinfo->waveInId = varprop.uintVal;
        VariantClear(&varprop);
      }
      devinfo->pMoniker = pMoniker;
      devinfo->pMoniker->AddRef(); // we keep a ref to this

      SafeRelease(pPropBag);
      ++devCount;
      ++devinfo;
    }
    SafeRelease(pMoniker);
  }

  pEnum->Release();
  return devCount;
}

//===-----------------------------------------------------------===//
//===-----------------------------------------------------------===//
HRESULT GetDeviceInfoCaps(CaptureDeviceInfo* devinfo, IAMStreamConfig* pStreamCfg)
{
  HRESULT hr = S_OK;
  // some caps
  int capCount = 0, capSize = 0;
  if (SUCCEEDED(pStreamCfg->GetNumberOfCapabilities(&capCount, &capSize)) && capSize == sizeof(VIDEO_STREAM_CONFIG_CAPS) && capCount > 0)
  {
    devinfo->capsCount = (unsigned int)capCount;
    devinfo->caps = (VIDEO_STREAM_CONFIG_CAPS*)realloc(devinfo->caps, sizeof(VIDEO_STREAM_CONFIG_CAPS)*capCount);    
    VIDEO_STREAM_CONFIG_CAPS* vscc = devinfo->caps;

    AM_MEDIA_TYPE *pmtCfg = NULL;
    for (int iFormat = 0; iFormat < capCount; ++iFormat, ++vscc)
    {
      ZeroMemory(vscc, sizeof(VIDEO_STREAM_CONFIG_CAPS));
      if (FAILED(pStreamCfg->GetStreamCaps(iFormat, &pmtCfg, (BYTE*)vscc))) continue;
      ReleaseMediaType(pmtCfg);
    }
  }
  else
  {
    hr = E_FAIL;
  }
  return hr;
}

//===-----------------------------------------------------------===//
// From VideoInput
//===-----------------------------------------------------------===//
HRESULT RouteCrossbar(ICaptureGraphBuilder2 **ppBuild, IBaseFilter **pVidInFilter, int conType, GUID captureMode)
{
  //create local ICaptureGraphBuilder2
  ICaptureGraphBuilder2 *pBuild = NULL;
  pBuild = *ppBuild;

  //create local IBaseFilter
  IBaseFilter *pVidFilter = NULL;
  pVidFilter = *pVidInFilter;

  // Search upstream for a crossbar.
  IAMCrossbar *pXBar1 = NULL;
  HRESULT hr = pBuild->FindInterface(&LOOK_UPSTREAM_ONLY, NULL, pVidFilter, IID_IAMCrossbar, (void**)&pXBar1);
  if (SUCCEEDED(hr))
  {
    bool foundDevice = false;
    printf("SETUP: You are not a webcam! Setting Crossbar\n");
    pXBar1->Release();

    IAMCrossbar *Crossbar;
    hr = pBuild->FindInterface(&captureMode, &MEDIATYPE_Interleaved, pVidFilter, IID_IAMCrossbar, (void **)&Crossbar);

    if (hr != NOERROR) {
      hr = pBuild->FindInterface(&captureMode, &MEDIATYPE_Video, pVidFilter, IID_IAMCrossbar, (void **)&Crossbar);
    }

    LONG lInpin, lOutpin;
    hr = Crossbar->get_PinCounts(&lOutpin, &lInpin);

    BOOL IPin = TRUE; LONG pIndex = 0, pRIndex = 0, pType = 0;

    while (pIndex < lInpin)
    {
      hr = Crossbar->get_CrossbarPinInfo(IPin, pIndex, &pRIndex, &pType);

      if (pType == conType) {
        printf("SETUP: Found Physical Interface");

        switch (conType) {

        case PhysConn_Video_Composite:
          printf(" - Composite\n");
          break;
        case PhysConn_Video_SVideo:
          printf(" - S-Video\n");
          break;
        case PhysConn_Video_Tuner:
          printf(" - Tuner\n");
          break;
        case PhysConn_Video_USB:
          printf(" - USB\n");
          break;
        case PhysConn_Video_1394:
          printf(" - Firewire\n");
          break;
        }

        foundDevice = true;
        break;
      }
      pIndex++;

    }

    if (foundDevice) {
      BOOL OPin = FALSE; LONG pOIndex = 0, pORIndex = 0, pOType = 0;
      while (pOIndex < lOutpin)
      {
        hr = Crossbar->get_CrossbarPinInfo(OPin, pOIndex, &pORIndex, &pOType);
        if (pOType == PhysConn_Video_VideoDecoder)
          break;
      }
      Crossbar->Route(pOIndex, pIndex);
    }
    else {
      printf("SETUP: Didn't find specified Physical Connection type. Using Defualt. \n");
    }

    //we only free the crossbar when we close or restart the device
    //we were getting a crash otherwise
    //if(Crossbar)Crossbar->Release();
    //if(Crossbar)Crossbar = NULL;

    if (pXBar1)pXBar1->Release();
    if (pXBar1)pXBar1 = NULL;

  }
  else {
    printf("SETUP: You are a webcam or snazzy firewire cam! No Crossbar needed\n");
    return hr;
  }

  return hr;
}

//===-----------------------------------------------------------===//
//===-----------------------------------------------------------===//
int GetMediaSubtypeBytes(GUID mediatype)
{
  if      (mediatype == MEDIASUBTYPE_RGB24) return 3;
  else if (mediatype == MEDIASUBTYPE_RGB32) return 4;
  else if (mediatype == MEDIASUBTYPE_RGB555) return 2;
  else if (mediatype == MEDIASUBTYPE_RGB565) return 2;
  else
  {
    DebugBreak(); // implement other types
  }
  return 0;
}

//===-----------------------------------------------------------===//
//===-----------------------------------------------------------===//
HRESULT SetSizeFormat(AM_MEDIA_TYPE* pMediaType, IAMStreamConfig* pStreamCfg, int attemptWidth, int attemptHeight, GUID mediatype, int attemptFPS=-1)
{
  VIDEOINFOHEADER *pVih = reinterpret_cast<VIDEOINFOHEADER*>(pMediaType->pbFormat);

  // store current config
  int lastWidth = pVih->bmiHeader.biWidth;
  int lastHeight = pVih->bmiHeader.biHeight;
  GUID lastFormatType = pMediaType->formattype;
  GUID lastMajorType = pMediaType->majortype;
  GUID lastSubType = pMediaType->subtype;
  ULONG lastSampleSize = pMediaType->lSampleSize;
  REFERENCE_TIME lastAvgTimePerFrame = pVih->AvgTimePerFrame;

  // try these config
  pVih->bmiHeader.biWidth = attemptWidth;
  pVih->bmiHeader.biHeight = attemptHeight;
  pMediaType->formattype = FORMAT_VideoInfo;
  pMediaType->majortype = MEDIATYPE_Video;
  pMediaType->subtype = mediatype;
  pMediaType->lSampleSize = attemptWidth * attemptHeight * GetMediaSubtypeBytes(mediatype); // tries RGB
  if (attemptFPS > 0)
    pVih->AvgTimePerFrame = (REFERENCE_TIME)( (1.0/attemptFPS)*1e7 ); // time per frame in units of 100ns

  // try this format, if error reverts to saved one
  HRESULT hr = pStreamCfg->SetFormat(pMediaType);
  if (hr != S_OK)
  {
    pVih->bmiHeader.biWidth = lastWidth;
    pVih->bmiHeader.biHeight= lastHeight;
    pMediaType->formattype = lastFormatType;
    pMediaType->majortype = lastMajorType;
    pMediaType->subtype = lastSubType;
    pMediaType->lSampleSize = lastSampleSize;
    pVih->AvgTimePerFrame= lastAvgTimePerFrame;
    pStreamCfg->SetFormat(pMediaType);
  }
  return hr;
}

void test_camcap()
{
  camcap* cc = 0;
  camcap_opts opts = { 0 };
  opts.input_flags = CC_FLAG_VIDEOINPUT;
  cc_init(&cc, &opts);
  if (cc_idev_count(cc, CC_FLAG_VIDEOINPUT) > 0)
  {
    CAMCAPIDEV idev = cc_idev_get(cc, 0, CC_FLAG_VIDEOINPUT);
    if (cc_idev_init(cc, idev) == CC_OK)
    {
      int modescount = cc_idev_modes(cc, idev, NULL, 0);
      if (modescount > 0)
      {
        camcapidev_mode* modes = (camcapidev_mode*)calloc(modescount, sizeof(camcapidev_mode));
        if (modes)
        {
          cc_idev_modes(cc, idev, modes, modescount);
          for (int i = 0; i < modescount; ++i)
          {

          }
          SafeFree(modes);
        }
      }
    }
    cc_idev_deinit(cc, idev);
  }
  cc_deinit(&cc);
}
//===-----------------------------------------------------------===//
// https://msdn.microsoft.com/en-us/library/windows/desktop/dd377566(v=vs.85).aspx
//===-----------------------------------------------------------===//
int main(int argc, const char** argv)
{
  UNREFERENCED_PARAMETER(argc);
  UNREFERENCED_PARAMETER(argv);

  test_camcap();
  exit(0);




  HRESULT hr;
  // Initializing COM
  {
    hr = CoInitializeEx(NULL, COINIT_MULTITHREADED);
    //hr = CoInitialize(NULL);
    CheckErrExit(hr, "Cannot initialize COM\n");    
  }

  // List capture devices
  static const int MAX_DEVICES = 4;
  CaptureDeviceInfo deviceInfos[MAX_DEVICES];
  int devicesCount=0;
  {
    // CLSID_AudioInputDeviceCategory
    devicesCount = ExtractDevicesInfo(CLSID_VideoInputDeviceCategory, deviceInfos, MAX_DEVICES); 
    if (devicesCount)
    {
      printf("** Video Capture Devices:\n");
      for (int i = 0; i < devicesCount; ++i)
        if (deviceInfos[i].name)
          printf("\tName: %S", deviceInfos[i].name);
    }    
  }

  // no video input, no continue
  if (!devicesCount)
    CheckErrExit(E_FAIL, "No video input found\n");

  // Build the objects to start the capture from the device
  ICaptureGraphBuilder2* pCaptureGraph = NULL;
  IGraphBuilder* pGraphBuilder = NULL;
  IMediaControl* pMediaControl = NULL;
  IBaseFilter* pVideoCaptureFilter = NULL;
  IBaseFilter* pGrabberFilter = NULL;
  IBaseFilter* pNullRenderFilter = NULL;
  IAMStreamConfig* pStreamCfg = NULL;
  AM_MEDIA_TYPE* pMediaType = NULL;
  ISampleGrabber* pGrabber = NULL;
  GUID captureMode = PIN_CATEGORY_CAPTURE;
  //SampleGrabberCallback* pGrabberCB = NULL;
  char* pFrameBuffer = NULL;
  {
    hr = CoCreateInstance(CLSID_CaptureGraphBuilder2, NULL, CLSCTX_INPROC_SERVER, IID_ICaptureGraphBuilder2, (void **)&pCaptureGraph);
    CheckErrExit(hr, "Cannot create capture graph builder\n");
    hr = CoCreateInstance(CLSID_FilterGraph, 0, CLSCTX_INPROC_SERVER, IID_IGraphBuilder, (void**)&pGraphBuilder);
    CheckErrExit(hr, "Cannot create graph builder\n");
    hr = pCaptureGraph->SetFiltergraph(pGraphBuilder);
    CheckErrExit(hr, "Cannot set filter graph\n");    
    hr = deviceInfos[0].pMoniker->BindToObject(NULL, NULL, IID_IBaseFilter, (void**)&pVideoCaptureFilter);
    CheckErrExit(hr, "Cannot bind device to create capture filter object\n");
    hr = pGraphBuilder->AddFilter(pVideoCaptureFilter, deviceInfos[0].name);
    CheckErrExit(hr, "Cannot add capture base filter to graph builder\n");
    // do we have a preview pin through Smart Tee filter?
    hr = pCaptureGraph->FindInterface(&PIN_CATEGORY_PREVIEW, &MEDIATYPE_Video, pVideoCaptureFilter, IID_IAMStreamConfig, (void **)&pStreamCfg);
    if (SUCCEEDED(hr))
    {
      captureMode = PIN_CATEGORY_PREVIEW;
      SafeRelease(pStreamCfg);
    }
    // route crossbar (just in case)
    hr = RouteCrossbar(&pCaptureGraph, &pVideoCaptureFilter, PhysConn_Video_Composite, captureMode);    
    // config stream for preview mode
    hr = pCaptureGraph->FindInterface(&captureMode, &MEDIATYPE_Video, pVideoCaptureFilter, IID_IAMStreamConfig, (void **)&pStreamCfg);
    CheckErrExit(hr, "Cannot configure the stream\n");
    hr = pStreamCfg->GetFormat(&pMediaType);
    CheckErrExit(hr, "Cannot get format of media\n");    

    hr = GetDeviceInfoCaps(deviceInfos, pStreamCfg);
    //SetSizeFormat(pMediaType, pStreamCfg, 320, 240, MEDIASUBTYPE_RGB24); // forcing specific size/format
    //SetSizeFormatFromCaps(pMediaType, pStreamCfg, deviceInfos[0].caps[0]); // or getting one from caps
    
    VIDEOINFOHEADER *pVideoInfo = reinterpret_cast<VIDEOINFOHEADER*>(pMediaType->pbFormat);
    deviceInfos[0].width = pVideoInfo->bmiHeader.biWidth;
    deviceInfos[0].height = pVideoInfo->bmiHeader.biHeight;

    hr = CoCreateInstance(CLSID_SampleGrabber, NULL, CLSCTX_INPROC_SERVER, IID_IBaseFilter, (void**)&pGrabberFilter);
    CheckErrExit(hr, "Cannot create frame grabber filter\n");

    hr = pGraphBuilder->AddFilter(pGrabberFilter, L"Sample Grabber");
    CheckErrExit(hr, "Cannot add grabber filter\n");

    hr = pGrabberFilter->QueryInterface(IID_ISampleGrabber, (void**)&pGrabber);
    CheckErrExit(hr, "Cannot create/query grabber object\n");

    // frame grabber.
    //pGrabberCB = new SampleGrabberCallback;
    //pGrabberCB->setupBuffer(pMediaType->lSampleSize);
    pFrameBuffer = (char*)malloc(pMediaType->lSampleSize);
    pGrabber->SetOneShot(FALSE); // for video, not photo
    pGrabber->SetBufferSamples(TRUE);
    //hr = pGrabber->SetCallback(pGrabberCB,0);
    //CheckErrExit(hr, "Cannot set callback for grabber\n");

    AM_MEDIA_TYPE mt;
    ZeroMemory(&mt, sizeof(AM_MEDIA_TYPE));
    mt.majortype = pMediaType->majortype;
    mt.subtype = pMediaType->subtype;
    mt.formattype = pMediaType->formattype;
    pGrabber->SetMediaType(&mt);

    SafeRelease(pStreamCfg); // don't need it anymore

    // need to specify a destination filter, even if it's null
    hr = CoCreateInstance(CLSID_NullRenderer, NULL, CLSCTX_INPROC_SERVER, IID_IBaseFilter, (void**)&pNullRenderFilter);
    CheckErrExit(hr, "Cannot create Null Renderer\n");
    hr = pGraphBuilder->AddFilter(pNullRenderFilter, L"NullRenderer");

    // Connects an output pin on a source filter to a sink filter, optionally through an intermediate filter.
    // https://msdn.microsoft.com/en-us/library/aa930715.aspx (samples)
    hr = pCaptureGraph->RenderStream(&PIN_CATEGORY_PREVIEW, &MEDIATYPE_Video, pVideoCaptureFilter, pGrabberFilter, pNullRenderFilter);
    CheckErrExit(hr, "Cannot connect the Preview PIN (RenderStream)\n");

    // Set null the Sync Source, so filters will run as quickly as possible
    IMediaFilter *pMediaFilter = NULL;
    hr = pGraphBuilder->QueryInterface(IID_IMediaFilter, (void**)&pMediaFilter);
    CheckErrExit(hr, "Cannot get IID_IMediaFilter\n");
    pMediaFilter->SetSyncSource(NULL);
    SafeRelease(pMediaFilter);

    hr = pGraphBuilder->QueryInterface(IID_IMediaControl, (void **)&pMediaControl);
    CheckErrExit(hr, "Cannot create media control object\n");
    hr = pMediaControl->Run();
    CheckErrExit(hr, "Cannot run the stream/start the graph\n");

    // In case of NO Callback, block until we're getting frames
    {
      long bufferSize = pMediaType->lSampleSize;
      while (hr != S_OK) 
      {
        hr = pGrabber->GetCurrentBuffer(&bufferSize, (long *)pFrameBuffer);
        Sleep(10);
      }
    }
  }

  // release filters
  SafeRelease(pNullRenderFilter);
  SafeRelease(pGrabberFilter);
  SafeRelease(pVideoCaptureFilter);

  // Capture loop
  {
    //regular capture method
    long bufferSize = pMediaType->lSampleSize;
    hr = pGrabber->GetCurrentBuffer(&bufferSize, (long *)pFrameBuffer);
    if (hr == S_OK) 
    {
      if (bufferSize == (long)pMediaType->lSampleSize)
      {
        stbi_write_png("capture.png", deviceInfos[0].width, deviceInfos[0].height, 3, pFrameBuffer, deviceInfos[0].width*3);
      }
    }
  }


  SafeFree(pFrameBuffer);
  //SafeRelease(pGrabberCB);
  SafeRelease(pGrabber);
  ReleaseMediaType(pMediaType);
  SafeRelease(pMediaControl);
  SafeRelease(pGraphBuilder);
  SafeRelease(pCaptureGraph);

  for (int i = 0; i < MAX_DEVICES; ++i)
    deviceInfos[i].Release();

  // Deinitializing COM
  {
    CoUninitialize();
  }
  return S_OK;
}
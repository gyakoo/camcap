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

/*
DOCUMENTATION

*/

#ifndef _CC_H_
#define _CC_H_

#ifndef _MSC_VER
#error CamCap Only compiles under Windows VisualStudio platform
#endif
#pragma warning(disable:4127)

#if !defined(CC_DSHOW) && !defined(CC_WMC)
#define CC_DSHOW
#endif

#include <stdint.h>
#ifdef CC_DSHOW
#include <dshow.h>
#include <uuids.h>
#include <aviriff.h>
#include <Windows.h>
#include <comutil.h>
#endif

// /////////////////////////////////////////////////////////////////////////////////////////////////
//                                  PUBLIC API                                   
// /////////////////////////////////////////////////////////////////////////////////////////////////
#define CC_OK (0)
#define CC_FAIL (-1)
#define CC_TRUE (1)
#define CC_FALSE (0)
#define CC_FLAG_VIDEOINPUT (1<<0)
#define CC_FLAG_AUDIOINPUT (1<<1)
#define CC_IDEV_INVALID (0xffffffff)

#ifdef __cplusplus
extern "C" {
#endif

  struct camcap;
  typedef void(*camcap_err_callabck)(int err, const char* msg);
  typedef struct camcap_opts
  {
    int32_t input_flags;
    camcap_err_callabck errcb; // optional
  }camcap_opts;

  typedef uint32_t CAMCAPIDEV;
  typedef struct camcapidev_mode
  {
    int32_t width;
    int32_t height;
    int64_t min_frame_interval;
    int64_t max_frame_interval;
    int32_t min_bps;
    int32_t max_bps;
  }camcapidev_mode;

  int cc_init(camcap** cc, camcap_opts* opts);
  void cc_deinit(camcap** cc);
  int cc_idev_count(camcap* cc, int itype);
  CAMCAPIDEV cc_idev_get(camcap* cc, int index, int itype);
  int cc_idev_init(camcap* cc, CAMCAPIDEV idev);
  int cc_idev_deinit(camcap* cc, CAMCAPIDEV idev);
  int cc_idev_is_initialized(camcap* cc, CAMCAPIDEV idev);
  int cc_idev_modes(camcap* cc, CAMCAPIDEV idev, camcapidev_mode* modes, unsigned int max_modes);

#ifdef __cplusplus
};
#endif
/*
- init
- list capture devices
- pick one
- list device capabilities (sizes/formats)
- start device (size/format, loop/callback)
- grab buffer directly or through callback
- stop device
- deinit

*/


#ifdef CC_IMPLEMENTATION
#if defined(__x86_64__) || defined(_M_X64)  ||  defined(__aarch64__)   || defined(__64BIT__) || \
  defined(__mips64)     || defined(__powerpc64__) || defined(__ppc64__)
#	define CC_PLATFORM_64
# define CC_ALIGNMENT 16
#else
#	define CC_PLATFORM_32
# define CC_ALIGNMENT 8
#endif //

#if defined(_DEBUG) || defined(DEBUG)
# define CC_BREAK { __debugbreak(); }
# define CC_ASSERT(c) { if (!(c)){ CC_BREAK; } }
#else
# define CC_BREAK {(void*)0;}
# define CC_ASSERT(c)
#endif

#ifndef cc_malloc
# define cc_malloc(sz) malloc(sz)
#endif
#ifndef cc_free
# define cc_free(p) free(p)
#endif
#ifndef cc_calloc
# define cc_calloc(count,size) calloc(count,size)
#endif
#ifndef cc_realloc
# define cc_realloc(p,sz) realloc(p,sz)
#endif

#define CC_BREAK_ALWAYS { __debugbreak(); }

// Common functions
template<typename T>
static void SafeRelease(T& ptr)
{
  if (ptr)
  {
    ptr->Release();
    ptr = NULL;
  }
}

template<typename T>
static void SafeFree(T& ptr)
{
  if (ptr)
  {
    free(ptr);
    ptr = NULL;
  }
}

//===-----------------------------------------------------------===//
// User code should free the returned pointer
//===-----------------------------------------------------------===//
wchar_t* BSTR2WChar(BSTR bstr)
{
  if (!bstr)
    return NULL;
  _bstr_t _b(bstr);
  const unsigned int l = _b.length();
  if (!l)
    return NULL;
  wchar_t* retstr = (wchar_t*)malloc(sizeof(wchar_t)*(l + 1));
  wcscpy_s(retstr, l + 1, (const wchar_t*)_b);
  return retstr;
}

#ifdef CC_DSHOW  /*    DIRECT SHOW      */
#pragma comment(lib, "uuid.lib")
#pragma comment(lib, "strmiids.lib")
#pragma comment(lib, "comsuppw.lib")
#define CC_IDEV_EXTRACT(idev,typ,ndx) { typ=(idev>>16); ndx=(idev&0xffff); }

// Due to a missing qedit.h in recent Platform SDKs
MIDL_INTERFACE("0579154A-2B53-4994-B0D0-E773148EFF85")
ISampleGrabberCB : public IUnknown
{
public:
  virtual HRESULT STDMETHODCALLTYPE SampleCB(
    double SampleTime,
    IMediaSample *pSample) = 0;

  virtual HRESULT STDMETHODCALLTYPE BufferCB(
    double SampleTime,
    BYTE *pBuffer,
    long BufferLen) = 0;

};

MIDL_INTERFACE("6B652FFF-11FE-4fce-92AD-0266B5D7C78F")
ISampleGrabber : public IUnknown
{
public:
  virtual HRESULT STDMETHODCALLTYPE SetOneShot(
    BOOL OneShot) = 0;

  virtual HRESULT STDMETHODCALLTYPE SetMediaType(
    const AM_MEDIA_TYPE *pType) = 0;

  virtual HRESULT STDMETHODCALLTYPE GetConnectedMediaType(
    AM_MEDIA_TYPE *pType) = 0;

  virtual HRESULT STDMETHODCALLTYPE SetBufferSamples(
    BOOL BufferThem) = 0;

  virtual HRESULT STDMETHODCALLTYPE GetCurrentBuffer(
    /* [out][in] */ long *pBufferSize,
    /* [out] */ long *pBuffer) = 0;

  virtual HRESULT STDMETHODCALLTYPE GetCurrentSample(
    /* [retval][out] */ IMediaSample **ppSample) = 0;

  virtual HRESULT STDMETHODCALLTYPE SetCallback(
    ISampleGrabberCB *pCallback,
    long WhichMethodToCallback) = 0;

};
EXTERN_C const CLSID CLSID_SampleGrabber;
EXTERN_C const IID IID_ISampleGrabber;
EXTERN_C const CLSID CLSID_NullRenderer;

void ReleaseMediaType(AM_MEDIA_TYPE* pMediaType)
{
  if (pMediaType->cbFormat)
    CoTaskMemFree((void*)pMediaType->pbFormat);
  SafeRelease(pMediaType->pUnk);
  CoTaskMemFree(pMediaType);
}

//===-----------------------------------------------------------===//
//===-----------------------------------------------------------===//
typedef struct dshowdevinfo
{
  dshowdevinfo()
    : name(NULL), desc(NULL), devpath(NULL), waveInId(0)
    , pMoniker(NULL), caps(NULL), capsCount(0)
    , pCaptureGraph(NULL), pGraphBuilder(NULL), pVideoCaptureFilter(NULL)
    , pStreamCfg(NULL)
  {
  }

  ~dshowdevinfo()
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

    ReleaseGraph();
  }

  void ReleaseGraph()
  {
    SafeFree(caps);
    capsCount = 0;
    SafeRelease(pStreamCfg);
    SafeRelease(pVideoCaptureFilter);
    SafeRelease(pGraphBuilder);
    SafeRelease(pCaptureGraph);
  }

  wchar_t*     name;
  wchar_t*     desc;
  wchar_t*     devpath;
  unsigned int  waveInId;
  IMoniker* pMoniker;  
  VIDEO_STREAM_CONFIG_CAPS* caps;
  unsigned int capsCount;

  ICaptureGraphBuilder2* pCaptureGraph;
  IGraphBuilder* pGraphBuilder;
  IBaseFilter* pVideoCaptureFilter;
  IAMStreamConfig* pStreamCfg;

}dshowdevinfo;

typedef struct camcap
{
  dshowdevinfo* devinfos;
  int devinfos_count;
  camcap_opts opts;
}cc;

#define cc_checkhr(hr,msg) \
  { if ( FAILED(hr) ) { if ( cc->opts.errcb ) cc->opts.errcb((int)hr,msg); } }

#define cc_checkhr_ret(hr,ec,msg) \
  { if ( FAILED(hr) ) { if ( cc->opts.errcb ) cc->opts.errcb((int)hr,msg); return ec; } }

#define cc_checkmem(ptr) \
  { if (!ptr){ if ( cc->opts.errcb ) cc->opts.errcb((int)CC_FAIL,"Out of mem"); return CC_FAIL; } }

HRESULT ccdshow_extract_dev_info(camcap* cc, REFGUID category);
HRESULT ccdshow_get_dev_caps(dshowdevinfo* devinfo);
HRESULT ccdshow_route_crossbar(camcap* cc, ICaptureGraphBuilder2 **ppBuild, IBaseFilter **pVidInFilter, int conType, GUID captureMode);

// //////////////////////////////////////////////////////////////////////////////////
// //////////////////////////////////////////////////////////////////////////////////
int cc_init(camcap** _cc, camcap_opts* opts)
{
  camcap* cc = (camcap*)cc_calloc(1, sizeof(camcap));
  *_cc = cc;
  if (!cc)
    return CC_FAIL;

  if ( opts )
    memcpy_s(&cc->opts, sizeof(camcap_opts), opts, sizeof(camcap_opts));

  HRESULT hr;

  // Initializing COM
  hr = CoInitializeEx(NULL, COINIT_MULTITHREADED);
  cc_checkhr_ret(hr, CC_FAIL, "Cannot initialize COM");
  
  // get dev info for video input category (todo: audio)
  hr = ccdshow_extract_dev_info(cc, CLSID_VideoInputDeviceCategory);
  cc_checkhr_ret(hr, CC_FAIL, "Cannot retrieve video input device info");
   

  return CC_OK;
}

// //////////////////////////////////////////////////////////////////////////////////
// //////////////////////////////////////////////////////////////////////////////////
void cc_deinit(camcap** cc)
{
  if (!cc || !*cc) return;
  
  // dev infos
  for (int i = 0; (*cc)->devinfos && i < (*cc)->devinfos_count; ++i)
    (*cc)->devinfos[i].Release();
  SafeFree((*cc)->devinfos);

  // camcap struct
  SafeFree(*cc);

  CoUninitialize();
}

// //////////////////////////////////////////////////////////////////////////////////
// //////////////////////////////////////////////////////////////////////////////////
int cc_idev_count(camcap* cc, int itype)
{
  int retval = 0;
  switch (itype)
  {
  case CC_FLAG_VIDEOINPUT: retval = cc->devinfos_count; break;
  }
  return retval;
}

// //////////////////////////////////////////////////////////////////////////////////
// //////////////////////////////////////////////////////////////////////////////////
CAMCAPIDEV cc_idev_get(camcap* cc, int index, int itype)
{
  if (itype == CC_FLAG_AUDIOINPUT || (index < 0 || index >= cc->devinfos_count) )
    return CC_IDEV_INVALID;
  return (CAMCAPIDEV)( (itype<<16) | (index&0xffff));
}

// //////////////////////////////////////////////////////////////////////////////////
// //////////////////////////////////////////////////////////////////////////////////
int cc_idev_init(camcap* cc, CAMCAPIDEV idev)
{
  // checking parameters
  if (idev == CC_IDEV_INVALID)
    cc_checkhr_ret(E_FAIL, CC_FAIL, "Idev invalid");

  int itype, ndxdev; 
  CC_IDEV_EXTRACT(idev, itype, ndxdev);

  if (itype == CC_FLAG_AUDIOINPUT)
    cc_checkhr_ret(E_FAIL, CC_FAIL, "Not supported audio input");

  if (ndxdev >= cc->devinfos_count)
    cc_checkhr_ret(E_FAIL, CC_FAIL, "Invalid video device index");

  if (cc_idev_is_initialized(cc, idev))
    cc_checkhr_ret(E_FAIL, CC_FAIL, "Device already initialized");

  // initializing objects
  dshowdevinfo* devinfo = cc->devinfos+ndxdev;
  HRESULT hr;
  hr = CoCreateInstance(CLSID_CaptureGraphBuilder2, NULL, CLSCTX_INPROC_SERVER, IID_ICaptureGraphBuilder2, (void **)&devinfo->pCaptureGraph);
  cc_checkhr_ret(hr, CC_FAIL, "Cannot create Capture Graph Builder");

  hr = CoCreateInstance(CLSID_FilterGraph, 0, CLSCTX_INPROC_SERVER, IID_IGraphBuilder, (void**)&devinfo->pGraphBuilder);
  cc_checkhr_ret(hr, CC_FAIL, "Cannot create Filter Graph");

  hr = devinfo->pCaptureGraph->SetFiltergraph(devinfo->pGraphBuilder);
  cc_checkhr_ret(hr, CC_FAIL, "Cannot set filter graph");

  hr = devinfo->pMoniker->BindToObject(NULL, NULL, IID_IBaseFilter, (void**)&devinfo->pVideoCaptureFilter);
  cc_checkhr_ret(hr, CC_FAIL, "Cannot bind device to create capture filter object");
  
  hr = devinfo->pGraphBuilder->AddFilter(devinfo->pVideoCaptureFilter, devinfo->name);
  cc_checkhr_ret(hr, CC_FAIL, "Cannot add capture base filter to graph builder");

  // check if we have pins in this order (smart tee needed for PREVIEW)
  const int CAPMODES_MAX = 2;
  GUID modes[CAPMODES_MAX] = { PIN_CATEGORY_PREVIEW, PIN_CATEGORY_CAPTURE };
  int iCapMode = 0;
  for (; iCapMode < CAPMODES_MAX; ++iCapMode)
  {
    hr = devinfo->pCaptureGraph->FindInterface(&modes[iCapMode], &MEDIATYPE_Video, devinfo->pVideoCaptureFilter, IID_IAMStreamConfig, (void **)&devinfo->pStreamCfg);
    if (SUCCEEDED(hr))
      break;
  }

  if (iCapMode == CAPMODES_MAX)
    cc_checkhr_ret(E_FAIL, CC_FAIL, "Cannot initialize the stream for preview or capture");

  // route crossbar just in case
  GUID captureMode = modes[iCapMode];
  hr = ccdshow_route_crossbar(cc, &devinfo->pCaptureGraph, &devinfo->pVideoCaptureFilter, PhysConn_Video_Composite, captureMode);
  if (FAILED(hr) && cc->opts.errcb)
    cc->opts.errcb(hr, "Canot route crossbar");

  // getting caps
  hr = ccdshow_get_dev_caps(devinfo);
  cc_checkhr_ret(hr, CC_FAIL, "Cannot retrieve device caps");
  
  return CC_OK;
}

// //////////////////////////////////////////////////////////////////////////////////
// //////////////////////////////////////////////////////////////////////////////////
int cc_idev_deinit(camcap* cc, CAMCAPIDEV idev)
{
  // checking parameters
  if (idev == CC_IDEV_INVALID)
    cc_checkhr_ret(E_FAIL, CC_FAIL, "Idev invalid");

  int itype, ndxdev;
  CC_IDEV_EXTRACT(idev, itype, ndxdev);

  if (itype == CC_FLAG_AUDIOINPUT)
    cc_checkhr_ret(E_FAIL, CC_FAIL, "Not supported audio input");

  if (ndxdev >= cc->devinfos_count)
    cc_checkhr_ret(E_FAIL, CC_FAIL, "Invalid video device index");

  if (!cc_idev_is_initialized(cc, idev))
    cc_checkhr_ret(E_FAIL, CC_FAIL, "Device not initialized");

  dshowdevinfo* devinfo = cc->devinfos + ndxdev;
  devinfo->ReleaseGraph();

  return CC_OK;
}

// //////////////////////////////////////////////////////////////////////////////////
// //////////////////////////////////////////////////////////////////////////////////
int cc_idev_is_initialized(camcap* cc, CAMCAPIDEV idev)
{
  // checking parameters
  if (idev == CC_IDEV_INVALID)
    cc_checkhr_ret(E_FAIL, CC_FAIL, "Idev invalid");

  int itype, ndxdev;
  CC_IDEV_EXTRACT(idev, itype, ndxdev);

  if (itype == CC_FLAG_AUDIOINPUT)
    cc_checkhr_ret(E_FAIL, CC_FAIL, "Not supported audio input");

  if (ndxdev >= cc->devinfos_count)
    cc_checkhr_ret(E_FAIL, CC_FAIL, "Invalid video device index");

  dshowdevinfo* devinfo = cc->devinfos + ndxdev;
  return (devinfo->pCaptureGraph) ? CC_TRUE : CC_FALSE;
}

// //////////////////////////////////////////////////////////////////////////////////
// //////////////////////////////////////////////////////////////////////////////////
int cc_idev_modes(camcap* cc, CAMCAPIDEV idev, camcapidev_mode* modes, unsigned int max_modes)
{
  // checking parameters
  if (idev == CC_IDEV_INVALID)
    cc_checkhr_ret(E_FAIL, CC_FAIL, "Idev invalid");

  int itype, ndxdev;
  CC_IDEV_EXTRACT(idev, itype, ndxdev);

  if (itype == CC_FLAG_AUDIOINPUT)
    cc_checkhr_ret(E_FAIL, CC_FAIL, "Not supported audio input");

  if (ndxdev >= cc->devinfos_count)
    cc_checkhr_ret(E_FAIL, CC_FAIL, "Invalid video device index");

  if (!cc_idev_is_initialized(cc, idev))
    cc_checkhr_ret(E_FAIL, CC_FAIL, "Device not initialized");
  
  dshowdevinfo* devinfo = cc->devinfos + ndxdev;

    // when no pointer passed, returns the actual count
  if (!modes)
    return (int)devinfo->capsCount;

  if (!devinfo->caps)
    cc_checkhr_ret(E_FAIL, CC_FAIL, "No caps in dev");

  // avoid overflow in any array, gets the minimum
  unsigned int min_count = devinfo->capsCount < max_modes ? devinfo->capsCount : max_modes;
  camcapidev_mode* curmode = modes;
  for (unsigned int i = 0; i < min_count; ++i, ++curmode)
  {
    VIDEO_STREAM_CONFIG_CAPS* caps = devinfo->caps + i;
    curmode->width = caps->InputSize.cx;
    curmode->height= caps->InputSize.cy;
    curmode->max_bps = caps->MaxBitsPerSecond;
    curmode->min_bps = caps->MinBitsPerSecond;
    curmode->max_frame_interval = caps->MaxFrameInterval;
    curmode->min_frame_interval = caps->MinFrameInterval;
  }
  return CC_OK;
}

// //////////////////////////////////////////////////////////////////////////////////
// //////////////////////////////////////////////////////////////////////////////////
HRESULT ccdshow_extract_dev_info(camcap* cc, REFGUID category)
{
  // Create the System Device Enumerator.
  ICreateDevEnum *pDevEnum;
  HRESULT hr = CoCreateInstance(CLSID_SystemDeviceEnum, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&pDevEnum));
  if (FAILED(hr))
    return hr;

  // Create an enumerator for the category.
  IEnumMoniker* pEnum = NULL;
  hr = pDevEnum->CreateClassEnumerator(category, &pEnum, 0);
  SafeRelease(pDevEnum);
  if (FAILED(hr) || hr == S_FALSE || !pEnum) // false is when the category is empty
    return E_FAIL;

  VARIANT varprop;
  IPropertyBag *pPropBag = NULL;
  IMoniker *pMoniker = NULL;
  cc->devinfos_count = 0;
  dshowdevinfo* devinfo = NULL;

  // first loop is to know the no. of devices, second loop to get the info out of them
  const int PASS_COUNT = 0;
  const int PASS_GET = 1;
  const int PASS_MAX = 2;
  for (int i = 0; i < PASS_MAX; ++i)
  {
    // all devices of this enumerator
    while (pEnum->Next(1, &pMoniker, NULL) == S_OK)
    {
      // get properties
      if (SUCCEEDED(pMoniker->BindToStorage(0, 0, IID_IPropertyBag, (void**)&pPropBag)))
      {
        switch (i)
        {
        case PASS_COUNT:
          ++cc->devinfos_count;
          break;
        case PASS_GET:
          {
            VariantInit(&varprop);
            if (SUCCEEDED(pPropBag->Read(L"Description", &varprop, 0)))
            {
              devinfo->desc = BSTR2WChar(varprop.bstrVal);
              VariantClear(&varprop);
            }

            VariantInit(&varprop);
            if (SUCCEEDED(pPropBag->Read(L"FriendlyName", &varprop, 0)))
            {
              devinfo->name = BSTR2WChar(varprop.bstrVal);
              VariantClear(&varprop);
            }

            VariantInit(&varprop);
            if (SUCCEEDED(pPropBag->Read(L"DevicePath", &varprop, 0)))
            {
              devinfo->devpath = BSTR2WChar(varprop.bstrVal);
              VariantClear(&varprop);
            }

            VariantInit(&varprop);
            if (SUCCEEDED(pPropBag->Read(L"WaveInID", &varprop, 0)))
            {
              devinfo->waveInId = varprop.uintVal;
              VariantClear(&varprop);
            }
            devinfo->pMoniker = pMoniker;
            devinfo->pMoniker->AddRef(); // we keep a ref to this

            ++devinfo; // next devinfo struct 
          }break;
        } 
        SafeRelease(pPropBag);
      }
      SafeRelease(pMoniker);
    }

    // allocate array
    if ( i == PASS_COUNT && cc->devinfos_count )
    {
      cc->devinfos = (dshowdevinfo*)calloc(cc->devinfos_count, sizeof(dshowdevinfo));
      cc_checkmem(cc->devinfos);
      devinfo = cc->devinfos;
      pEnum->Reset();
    }
  }
  pEnum->Release();
  return S_OK;
}

// //////////////////////////////////////////////////////////////////////////////////
// Borrowed from VideoInput
// //////////////////////////////////////////////////////////////////////////////////
HRESULT ccdshow_route_crossbar(camcap* cc, ICaptureGraphBuilder2 **ppBuild, IBaseFilter **pVidInFilter, int conType, GUID captureMode)
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
    if ( cc->opts.errcb )
      cc->opts.errcb(CC_FAIL,"You are not a webcam! Setting Crossbar");
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

        switch (conType) {

        case PhysConn_Video_Composite:
          if (cc->opts.errcb)
            cc->opts.errcb(CC_FAIL, " Found Physical Interface - Composite");
          break;
        case PhysConn_Video_SVideo:
          if (cc->opts.errcb)
            cc->opts.errcb(CC_FAIL, " Found Physical Interface - S-Video");
          break;
        case PhysConn_Video_Tuner:
          if (cc->opts.errcb)
            cc->opts.errcb(CC_FAIL, " Found Physical Interface - Tuner");
          break;
        case PhysConn_Video_USB:
          if (cc->opts.errcb)
            cc->opts.errcb(CC_FAIL, " Found Physical Interface - USB");
          break;
        case PhysConn_Video_1394:
          if (cc->opts.errcb)
            cc->opts.errcb(CC_FAIL, " Found Physical Interface - Firewire");
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
      if (cc->opts.errcb)
        cc->opts.errcb(CC_FAIL, "Didn't find specified Physical Connection type. Using Defualt.");
    }

    //we only free the crossbar when we close or restart the device
    //we were getting a crash otherwise
    //if(Crossbar)Crossbar->Release();
    //if(Crossbar)Crossbar = NULL;

    if (pXBar1)pXBar1->Release();
    if (pXBar1)pXBar1 = NULL;

  }
  else {
    if (cc->opts.errcb)
      cc->opts.errcb(CC_FAIL, "You are a webcam or snazzy firewire cam! No Crossbar needed");
    return hr;
  }

  return hr;
}

// //////////////////////////////////////////////////////////////////////////////////
// //////////////////////////////////////////////////////////////////////////////////
HRESULT ccdshow_get_dev_caps(dshowdevinfo* devinfo)
{
  if (!devinfo->pStreamCfg) 
    return E_FAIL;

  HRESULT hr = S_OK;

  // some caps
  int capCount = 0, capSize = 0;
  if (SUCCEEDED(devinfo->pStreamCfg->GetNumberOfCapabilities(&capCount, &capSize)) && capSize == sizeof(VIDEO_STREAM_CONFIG_CAPS) && capCount > 0)
  {
    devinfo->capsCount = (unsigned int)capCount;
    devinfo->caps = (VIDEO_STREAM_CONFIG_CAPS*)realloc(devinfo->caps, sizeof(VIDEO_STREAM_CONFIG_CAPS)*capCount);
    VIDEO_STREAM_CONFIG_CAPS* vscc = devinfo->caps;

    AM_MEDIA_TYPE *pmtCfg = NULL;
    for (int iFormat = 0; iFormat < capCount; ++iFormat, ++vscc)
    {
#error would be nice also to store info from pmtCfg like the format type, total size...
      ZeroMemory(vscc, sizeof(VIDEO_STREAM_CONFIG_CAPS));
      if (FAILED(devinfo->pStreamCfg->GetStreamCaps(iFormat, &pmtCfg, (BYTE*)vscc))) continue;
      ReleaseMediaType(pmtCfg);
    }
  }
  else
  {
    hr = E_FAIL;
  }
  return hr;
}

#elif defined(CC_WMC) /*   WINDOWS MEDIA CAPTURE */

#endif


#endif // CC_IMPLEMENTATION

#pragma warning(default:4127)

#endif // _CC_H_
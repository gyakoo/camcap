// The MIT License (MIT)
// 
// Copyright (c) 2015 Manu Marin / gyakoo@gmail.com
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

// PART OF THIS CODE IS BASED ON https://github.com/ofTheo/videoInput
// videoInput by Theodore Watson theo.watson@gmail.com

/*
DOCUMENTATION - DIRECT SHOW

// setting the media type of the grabber
https://msdn.microsoft.com/en-us/library/dd407288(v=vs.85).aspx#set_the_media_type
// general video input doc
https://msdn.microsoft.com/en-us/library/windows/desktop/dd377566(v=vs.85).aspx
// render stream use
https://msdn.microsoft.com/en-us/library/aa930715.aspx (also samples)
// using sample grabber
https://msdn.microsoft.com/en-us/library/windows/desktop/dd407288(v=vs.85).aspx


DOCUMENTATION - WINDOWS MEDIA FOUNDATION (WIN VISTA AND 7)

https://msdn.microsoft.com/en-us/library/windows/desktop/bb970511(v=vs.85).aspx


*/

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


/*
// frame grabber.
//pGrabberCB = new SampleGrabberCallback;
//pGrabberCB->setupBuffer(pMediaType->lSampleSize);
pFrameBuffer = (char*)malloc(pMediaType->lSampleSize);
pGrabber->SetOneShot(TRUE); // for video, not photo
pGrabber->SetBufferSamples(TRUE);
//hr = pGrabber->SetCallback(pGrabberCB,0);
//CheckErrExit(hr, "Cannot set callback for grabber\n");

*/

#ifndef _CC_H_
#define _CC_H_

#ifndef _MSC_VER
#   error CamCap Only compiles under Windows VisualStudio platform
#endif
#pragma warning(disable:4127)

#if !defined(CC_DSHOW) && !defined(CC_WMC)
#   define CC_DSHOW
#endif

#include <stdint.h>
#ifdef CC_DSHOW
#   include <dshow.h>
#   define __IDxtCompositor_INTERFACE_DEFINED__
#   define __IDxtAlphaSetter_INTERFACE_DEFINED__
#   define __IDxtJpeg_INTERFACE_DEFINED__
#   define __IDxtKey_INTERFACE_DEFINED__
#   include <uuids.h>
#   include <aviriff.h>
#   include <Windows.h>
#   include <comutil.h>
#endif

// /////////////////////////////////////////////////////////////////////////////////////////////////
//                                  PUBLIC API                                   
// /////////////////////////////////////////////////////////////////////////////////////////////////
#define CC_OK (0)
#define CC_FAIL (-1)
#define CC_TRUE (1)
#define CC_FALSE (0)
#define CC_FLAG_VIDEOINPUT (1<<0)

// !!! Add more video format subtypes here and in functions like:
// ccdshow_mediasubtype_to_formattype
// ccdshow_formattype_to_mediasubtype
// cc_get_format_type_name
#define CC_VIDEOFMT_UNKNOWN -1
#define CC_VIDEOFMT_RGB24 0
#define CC_VIDEOFMT_RGB32 1
#define CC_VIDEOFMT_MJPEG 2    // motion jpeg
#define CC_VIDEOFMT_YUV420P 3
#define CC_VIDEOFMT_JPEG 4
#define CC_VIDEOFMT_RGB565 5
#define CC_VIDEOFMT_RGB555 6
#define CC_VIDEOFMT_MAX 7 // <- ADD HERE

#ifdef __cplusplus
extern "C" {
#endif

  struct camcap;
  typedef void(*camcap_err_callabck)(int err, const char* msg);
  typedef struct camcap_opts
  {
    int32_t input_flags;
    camcap_err_callabck errcb; // optional to receive error codes and messages
  }camcap_opts;

  typedef struct camcapidev_mode
  {
    int64_t min_frame_interval;     // supported range of frame intervals
    int64_t max_frame_interval;
    int width;                      // width of the image
    int height;                     // height of the image
    int bitcount;                   // no. of bits per pixel
    int video_format_type;          // CC_VIDEOFMT_* 
    int min_bps;                    // bits per second range 
    int max_bps;
    int64_t avg_time_per_frame;     // in units of 100ns
    int reserved;                   // not used
  }camcapidev_mode;

    // Initializes the camcap library with options
  int cc_init(camcap** cc, camcap_opts* opts);

    // Deinitialize camcap library
  void cc_deinit(camcap** cc);

    // Returns number of input devices connected
  int cc_idev_count(camcap* cc);

    // Initializes the input video device with index idev
  int cc_idev_init(camcap* cc, int idev);
    
    // Deinitializes the input video device with index idev
  int cc_idev_deinit(camcap* cc, int idev);

    // Returns 0 if the input video device idev isn't initialized, or 1 if it is
  int cc_idev_is_initialized(camcap* cc, int idev);

    // Copies up to max_modes into the array modes, with supported input video device modes
    // To return the number of available modes for the idev, pass modes as NULL
  int cc_idev_modes(camcap* cc, int idev, camcapidev_mode* modes, unsigned int max_modes);

    // Get information about the current set mode.
  int cc_idev_get_current_mode(camcap* cc, int idev, camcapidev_mode* out_mode);

    // Set the mode to the input video device.
  int cc_idev_set_mode(camcap* cc, int idev, camcapidev_mode* mode);

    // Start capturing the input video device idev
  int cc_idev_start(camcap* cc, int idev);

    // Pause capturing
  int cc_idev_pause(camcap* cc, int idev);

    // Stop capturing
  int cc_idev_stop(camcap* cc, int idev);

    // Retrieves the frame from the running input video device (synchronous mode)
  int cc_idev_grab(camcap* cc, int idev, int timeout_ms);

    // Returns the last grabbed frame buffer, buffer size depends on the current set mode 
  void* cc_idev_get_buffer(camcap* cc, int idev);

    // Gets the name of the given video format type (CC_VIDEOFMT_*)
  const char* cc_get_format_type_name(int format_type);

#ifdef __cplusplus
};
#endif

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
#define CC_BREAK_ALWAYS { __debugbreak(); }
#define CC_ASSERT_ALWAYS(c) { if (!(c)) { CC_BREAK_ALWAYS;} }

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

#define CC_MODENDX_MAGIC (0xabcd)

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
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////DIRECT SHOW IMPLEMENTATION/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#ifdef CC_DSHOW  /*    DIRECT SHOW      */
#pragma comment(lib, "uuid.lib")
#pragma comment(lib, "strmiids.lib")
#pragma comment(lib, "comsuppw.lib")


// missing I420 from uuids.h
// YUV420p
// 30323449-0000-0010-8000-00AA00389B71  'i420' == MEDIASUBTYPE_I420
const GUID MEDIASUBTYPE_I420 = { 0x30323449, 0x0000, 0x0010, {0x80, 0x00, 0x00, 0xaa, 0x00, 0x38, 0x9b, 0x71} };

// //////////////////////////////////////////////////////////////////////////////////
// Due to a missing qedit.h in recent Platform SDKs
// //////////////////////////////////////////////////////////////////////////////////
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

// //////////////////////////////////////////////////////////////////////////////////
// //////////////////////////////////////////////////////////////////////////////////
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

// //////////////////////////////////////////////////////////////////////////////////
// //////////////////////////////////////////////////////////////////////////////////
void ReleaseMediaType(AM_MEDIA_TYPE*& pMediaType)
{
  if (!pMediaType) return;
  if (pMediaType->cbFormat)
    CoTaskMemFree((void*)pMediaType->pbFormat);
  SafeRelease(pMediaType->pUnk);
  CoTaskMemFree(pMediaType);
  pMediaType = NULL;
}

// //////////////////////////////////////////////////////////////////////////////////
// capabilities of an input video device in Directshow
// //////////////////////////////////////////////////////////////////////////////////
typedef struct dshowdevcap
{
  VIDEO_STREAM_CONFIG_CAPS cfgcaps;
  AM_MEDIA_TYPE* pMediaType;
}dshowdevcap;

// //////////////////////////////////////////////////////////////////////////////////
// information and objects of an input video device in DirectShow
// //////////////////////////////////////////////////////////////////////////////////
typedef struct dshowdevinfo
{
  dshowdevinfo()
    : name(NULL), desc(NULL), devpath(NULL), waveInId(0)
    , pMoniker(NULL), caps(NULL), capsCount(0)
    , pCaptureGraph(NULL), pGraphBuilder(NULL), pVideoCaptureFilter(NULL)
    , pStreamCfg(NULL), pGrabberFilter(NULL), pGrabber(NULL), pNullRenderFilter(NULL)
    , pCurMediaType(NULL), pMediaControl(NULL), pFrameBuffer(NULL)
  {
  }

  ~dshowdevinfo()
  {
    Release();
  }

  void Release()
  {
    // base info for device
    SafeFree(name);
    SafeFree(desc);
    SafeFree(devpath);
    waveInId = 0;
    SafeRelease(pMoniker);        

    // graph builder info for device
    ReleaseGraph();
  }

  void ReleaseGraph()
  {
    SafeFree(pFrameBuffer);
    for (unsigned int i = 0; caps && i < capsCount; ++i)
      ReleaseMediaType(caps[i].pMediaType);
    SafeFree(caps);
    capsCount = 0;
    SafeRelease(pMediaControl);
    ReleaseMediaType(pCurMediaType);
    SafeRelease(pNullRenderFilter);
    SafeRelease(pGrabber);
    SafeRelease(pGrabberFilter);
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
  dshowdevcap* caps;
  unsigned int capsCount;

  ICaptureGraphBuilder2* pCaptureGraph;
  IGraphBuilder* pGraphBuilder;
  IBaseFilter* pVideoCaptureFilter;
  IAMStreamConfig* pStreamCfg;
  IBaseFilter* pGrabberFilter;
  ISampleGrabber* pGrabber;
  IBaseFilter* pNullRenderFilter;
  AM_MEDIA_TYPE* pCurMediaType;
  IMediaControl* pMediaControl;
  unsigned char* pFrameBuffer;
}dshowdevinfo;

// //////////////////////////////////////////////////////////////////////////////////
// camcap structure in DirectShow implementation
// //////////////////////////////////////////////////////////////////////////////////
typedef struct camcap
{
  dshowdevinfo* devinfos;
  int devinfos_count;
  camcap_opts opts;
}cc;


// //////////////////////////////////////////////////////////////////////////////////
// direct show specific helpers
// //////////////////////////////////////////////////////////////////////////////////
#define CC_VIH(pmt) ( (VIDEOINFOHEADER*)(pmt->pbFormat) )

#define cc_checkhr(hr,msg) \
  { if ( FAILED(hr) ) { if ( cc->opts.errcb ) cc->opts.errcb((int)hr,msg); } }

#define cc_checkhr_ret(hr,ec,msg) \
  { if ( FAILED(hr) ) { if ( cc->opts.errcb ) cc->opts.errcb((int)hr,msg); return ec; } }

#define cc_checkmem(ptr) \
  { if (!ptr){ if ( cc->opts.errcb ) cc->opts.errcb((int)CC_FAIL,"Out of mem"); return CC_FAIL; } }

HRESULT ccdshow_extract_dev_info(camcap* cc, REFGUID category);
HRESULT ccdshow_get_dev_caps(dshowdevinfo* devinfo);
HRESULT ccdshow_route_crossbar(camcap* cc, ICaptureGraphBuilder2 **ppBuild, IBaseFilter **pVidInFilter, int conType, GUID captureMode);
int ccdshow_mediasubtype_to_formattype(REFGUID subtype);
void ccdshow_formattype_to_mediasubtype(int format_type, GUID* out_subtype);
dshowdevinfo* ccdshow_get_dev(camcap* cc, int idev);
HRESULT ccdshow_set_framebuffer_size(dshowdevinfo* devinfo, AM_MEDIA_TYPE* pMediaType);
HRESULT ccdshow_wait_graph_state(dshowdevinfo* devinfo, OAFilterState desired_st, int timeout_ms);

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
  
  // get dev info for video input category
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
int cc_idev_count(camcap* cc)
{
  return cc->devinfos_count;
}

// //////////////////////////////////////////////////////////////////////////////////
// //////////////////////////////////////////////////////////////////////////////////
int cc_idev_init(camcap* cc, int idev)
{
  // checking parameters
  if (idev >= cc->devinfos_count)
    cc_checkhr_ret(E_FAIL, CC_FAIL, "Invalid video device index");

  if (cc_idev_is_initialized(cc, idev))
    cc_checkhr_ret(E_FAIL, CC_FAIL, "Device already initialized");

  // initializing base graph objects
  dshowdevinfo* devinfo = cc->devinfos+ idev;
  HRESULT hr;
  hr = CoCreateInstance(CLSID_CaptureGraphBuilder2, NULL, CLSCTX_INPROC_SERVER, IID_ICaptureGraphBuilder2, (void **)&devinfo->pCaptureGraph);
  cc_checkhr_ret(hr, CC_FAIL, "Cannot create Capture Graph Builder");
  hr = CoCreateInstance(CLSID_FilterGraph, 0, CLSCTX_INPROC_SERVER, IID_IGraphBuilder, (void**)&devinfo->pGraphBuilder);
  cc_checkhr_ret(hr, CC_FAIL, "Cannot create Filter Graph");
  hr = devinfo->pCaptureGraph->SetFiltergraph(devinfo->pGraphBuilder);
  cc_checkhr_ret(hr, CC_FAIL, "Cannot set filter graph");

  // initialize and add base video capture filter
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

  // getting caps and default media type
  hr = ccdshow_get_dev_caps(devinfo);
  cc_checkhr_ret(hr, CC_FAIL, "Cannot retrieve device caps");
  hr = devinfo->pStreamCfg->GetFormat(&devinfo->pCurMediaType);
  cc_checkhr_ret(hr, CC_FAIL, "Cannot retrieve device default media type");
  hr = ccdshow_set_framebuffer_size(devinfo, devinfo->pCurMediaType);
  cc_checkhr_ret(hr, CC_FAIL, "Cannot initialize frame buffer with default media type");

  // grabber filter and interface
  hr = CoCreateInstance(CLSID_SampleGrabber, NULL, CLSCTX_INPROC_SERVER, IID_IBaseFilter, (void**)&devinfo->pGrabberFilter);
  cc_checkhr_ret(hr, CC_FAIL, "Cannot create frame grabber filter");
  hr = devinfo->pGraphBuilder->AddFilter(devinfo->pGrabberFilter, L"Sample Grabber");
  cc_checkhr_ret(hr, CC_FAIL, "Cannot add grabber filter to graph");
  hr = devinfo->pGrabberFilter->QueryInterface(IID_ISampleGrabber, (void**)&devinfo->pGrabber);
  cc_checkhr_ret(hr, CC_FAIL, "Cannot create/query grabber object");
  hr = devinfo->pGrabber->SetOneShot(TRUE);
  cc_checkhr_ret(hr, CC_FAIL, "Cannot set one show");
  hr = devinfo->pGrabber->SetBufferSamples(TRUE);
  cc_checkhr_ret(hr, CC_FAIL, "Cannot buffer samples");

  // sets the default media type to the grabber filter (it needs one)
  // it uses a temp struct initialized with pCurMediaType, b/c devinfo->pCurMediaType does not work directly
  AM_MEDIA_TYPE defMt = { 0 };
  defMt.majortype = devinfo->pCurMediaType->majortype;
  defMt.subtype = devinfo->pCurMediaType->subtype;
  defMt.formattype = devinfo->pCurMediaType->formattype;
  hr = devinfo->pGrabber->SetMediaType(&defMt);
  cc_checkhr_ret(hr, CC_FAIL, "Cannot set default media type to grabber filter");

  // null renderer filter because we need to specify a destination filter
  hr = CoCreateInstance(CLSID_NullRenderer, NULL, CLSCTX_INPROC_SERVER, IID_IBaseFilter, (void**)&devinfo->pNullRenderFilter);
  cc_checkhr_ret(hr, CC_FAIL, "Cannot create Null Renderer Filter");
  hr = devinfo->pGraphBuilder->AddFilter(devinfo->pNullRenderFilter, L"NullRenderer");
  cc_checkhr_ret(hr, CC_FAIL, "Cannot add Null Renderer Filter to graph");

  // Connects an output pin on a source filter to a sink filter, optionally through an intermediate filter.
  hr = devinfo->pCaptureGraph->RenderStream(&PIN_CATEGORY_PREVIEW, &MEDIATYPE_Video, devinfo->pVideoCaptureFilter, devinfo->pGrabberFilter, devinfo->pNullRenderFilter);
  cc_checkhr_ret(hr, CC_FAIL, "Cannot connect the Preview PIN (RenderStream)");

  // sets Sync Source of all filters to NULL. Not to syncing the filters will make them run as quickly as possible
  IMediaFilter *pMediaFilter = NULL;
  hr = devinfo->pGraphBuilder->QueryInterface(IID_IMediaFilter, (void**)&pMediaFilter);
  if (SUCCEEDED(hr))
  {
    pMediaFilter->SetSyncSource(NULL);
    SafeRelease(pMediaFilter);
  }

  // creates the media control interface
  hr = devinfo->pGraphBuilder->QueryInterface(IID_IMediaControl, (void **)&devinfo->pMediaControl);
  cc_checkhr_ret(hr, CC_FAIL, "Cannot create media control object");

  return CC_OK;
}

// //////////////////////////////////////////////////////////////////////////////////
// //////////////////////////////////////////////////////////////////////////////////
dshowdevinfo* ccdshow_get_dev(camcap* cc, int idev)
{
  // checking parameters
  if (!cc || !cc->devinfos)
    cc_checkhr_ret(E_FAIL, NULL, "Invalid pointer");

  if (idev >= cc->devinfos_count)
    cc_checkhr_ret(E_FAIL, NULL, "Invalid video device index");

  if (!cc_idev_is_initialized(cc, idev))
    cc_checkhr_ret(E_FAIL, NULL, "Device not initialized");

  return cc->devinfos + idev;
}

// //////////////////////////////////////////////////////////////////////////////////
// //////////////////////////////////////////////////////////////////////////////////
int cc_idev_deinit(camcap* cc, int idev)
{
  dshowdevinfo* devinfo = ccdshow_get_dev(cc, idev);
  if (!devinfo) return CC_FAIL;

  devinfo->ReleaseGraph();
  return CC_OK;
}

// //////////////////////////////////////////////////////////////////////////////////
// //////////////////////////////////////////////////////////////////////////////////
int cc_idev_is_initialized(camcap* cc, int idev)
{
  // checking parameters
  if (idev >= cc->devinfos_count)
    cc_checkhr_ret(E_FAIL, CC_FAIL, "Invalid video device index");

  dshowdevinfo* devinfo = cc->devinfos + idev;
  return (devinfo->pCaptureGraph) ? CC_TRUE : CC_FALSE;
}

// //////////////////////////////////////////////////////////////////////////////////
// //////////////////////////////////////////////////////////////////////////////////
int cc_idev_modes(camcap* cc, int idev, camcapidev_mode* modes, unsigned int max_modes)
{
  dshowdevinfo* devinfo = ccdshow_get_dev(cc, idev);
  if (!devinfo) return CC_FAIL;

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
    dshowdevcap* caps = devinfo->caps + i;
    curmode->width = caps->cfgcaps.InputSize.cx;
    curmode->height= caps->cfgcaps.InputSize.cy;
    curmode->max_bps = caps->cfgcaps.MaxBitsPerSecond;
    curmode->min_bps = caps->cfgcaps.MinBitsPerSecond;
    curmode->max_frame_interval = caps->cfgcaps.MaxFrameInterval;
    curmode->min_frame_interval = caps->cfgcaps.MinFrameInterval;
    if (caps->pMediaType)
    {
      curmode->bitcount = CC_VIH(caps->pMediaType)->bmiHeader.biBitCount;
      curmode->avg_time_per_frame = CC_VIH(caps->pMediaType)->AvgTimePerFrame;
      curmode->video_format_type = ccdshow_mediasubtype_to_formattype(caps->pMediaType->subtype);
    }
    curmode->reserved = (int)MAKELONG(CC_MODENDX_MAGIC, (WORD)i); // pattern to indicate it's an index
  }
  return CC_OK;
}

// //////////////////////////////////////////////////////////////////////////////////
// //////////////////////////////////////////////////////////////////////////////////
int cc_idev_get_current_mode(camcap* cc, int idev, camcapidev_mode* out_mode)
{
  if (!cc || idev < 0 || !out_mode) return CC_FAIL;
  dshowdevinfo* devinfo = ccdshow_get_dev(cc, idev);
  if (!devinfo) return CC_FAIL;
  VIDEOINFOHEADER* vih = CC_VIH(devinfo->pCurMediaType);
  if (!vih) return CC_FAIL;

  out_mode->width = vih->bmiHeader.biWidth;
  out_mode->height = vih->bmiHeader.biHeight;
  out_mode->max_bps = 0;
  out_mode->min_bps = 0;
  out_mode->max_frame_interval = 0;
  out_mode->min_frame_interval = 0;
  out_mode->bitcount = vih->bmiHeader.biBitCount;
  out_mode->avg_time_per_frame = vih->AvgTimePerFrame;
  out_mode->video_format_type = ccdshow_mediasubtype_to_formattype(devinfo->pCurMediaType->subtype);
  out_mode->reserved = 0;
  return CC_OK;
}

// //////////////////////////////////////////////////////////////////////////////////
// //////////////////////////////////////////////////////////////////////////////////
int cc_idev_set_mode(camcap* cc, int idev, camcapidev_mode* mode)
{
  dshowdevinfo* devinfo = ccdshow_get_dev(cc, idev);
  if (!devinfo) return CC_FAIL;

  // saved current format
  AM_MEDIA_TYPE saved_mt = { 0 };
  //VIDEOINFOHEADER saved_vih = { 0 };

  saved_mt = *devinfo->pCurMediaType;
  // setting desired info
  VIDEOINFOHEADER* vih = CC_VIH(devinfo->pCurMediaType);
  if (!vih) return CC_FAIL;
  //saved_vih = *vih;

  vih->AvgTimePerFrame = mode->avg_time_per_frame;
  vih->bmiHeader.biWidth = mode->width;
  vih->bmiHeader.biHeight = mode->height;
  ccdshow_formattype_to_mediasubtype(mode->video_format_type, &devinfo->pCurMediaType->subtype);
  devinfo->pCurMediaType->lSampleSize = mode->width * mode->height * mode->bitcount;
  devinfo->pCurMediaType->lSampleSize >>= 3; // don't like this, because even when the bitcount might be 12, the stride for pixel might be 2 bytes not 1'5
  vih->bmiHeader.biSizeImage = devinfo->pCurMediaType->lSampleSize;

  // try to set the format
  int retval = CC_OK;
  HRESULT hr = devinfo->pStreamCfg->SetFormat(devinfo->pCurMediaType);
  int set_fb_size = CC_FALSE;
  if (hr == S_OK)
  {
    hr = ccdshow_set_framebuffer_size(devinfo, devinfo->pCurMediaType);
    if (hr == S_OK)
    {
      AM_MEDIA_TYPE defMt = { 0 };
      defMt.majortype = devinfo->pCurMediaType->majortype;
      defMt.subtype = devinfo->pCurMediaType->subtype;
      defMt.formattype = devinfo->pCurMediaType->formattype;
      defMt.lSampleSize = devinfo->pCurMediaType->lSampleSize;
      hr = devinfo->pGrabber->SetMediaType(&defMt);
      if (hr != S_OK)
        set_fb_size = CC_TRUE;
    }
    else
    {
      set_fb_size = CC_TRUE; // could set the format OK, but couldn't allocate frame in memory
    }
  }
    
  if (hr!=S_OK)
  {
    // failed. restore saved format info
    *devinfo->pCurMediaType = saved_mt;

    // reallocate frame in memory for the old format only if it changed format but couldn't allocate for new format
    if ( set_fb_size )
      ccdshow_set_framebuffer_size(devinfo, devinfo->pCurMediaType);
    retval = CC_FAIL;
  }
  return retval;
}

// //////////////////////////////////////////////////////////////////////////////////
// //////////////////////////////////////////////////////////////////////////////////
int cc_idev_start(camcap* cc, int idev)
{
  dshowdevinfo* devinfo = ccdshow_get_dev(cc, idev);
  if (!devinfo) return CC_FAIL;

  OAFilterState state;
  if (devinfo->pMediaControl->GetState(100, &state) != S_OK)
    return CC_FAIL;

  if (state != State_Running)
  {
    HRESULT hr = devinfo->pMediaControl->Run();    
    if (hr == S_FALSE)
    {
      hr = ccdshow_wait_graph_state(devinfo, State_Running, 500);
      if (FAILED(hr))
        return CC_FAIL;
    }
  }

  return CC_OK;
}

// //////////////////////////////////////////////////////////////////////////////////
// //////////////////////////////////////////////////////////////////////////////////
int cc_idev_grab(camcap* cc, int idev, int timeout_ms)
{
  dshowdevinfo* devinfo = ccdshow_get_dev(cc, idev);
  if (!devinfo) return CC_FAIL;

  // is running?
  OAFilterState state;
  if (devinfo->pMediaControl->GetState(100, &state) != S_OK || state != State_Running )
    return CC_FAIL;

  // get curren buffer and size
  HRESULT hr = S_FALSE;
  const long buffersize = CC_VIH(devinfo->pCurMediaType)->bmiHeader.biSizeImage;
  long resbuffsize = buffersize;
  while (hr != S_OK && timeout_ms >= 0)
  {
    hr = devinfo->pGrabber->GetCurrentBuffer(&resbuffsize, (long*)devinfo->pFrameBuffer);
    
    // it returns ok, but the buffer wasn't filled with all expected bytes, then loop
    if (hr == S_OK && buffersize != resbuffsize)
    {
      hr = S_FALSE;
      resbuffsize = buffersize;
    }

    // if it's not ok, sleep and make a loop until timeout
    if (hr != S_OK ) 
    {
      Sleep(10);
      timeout_ms -= 10;
    }
  }
  if (hr != S_OK)
    return CC_FAIL;
  return CC_OK;
}

// //////////////////////////////////////////////////////////////////////////////////
// //////////////////////////////////////////////////////////////////////////////////
void* cc_idev_get_buffer(camcap* cc, int idev)
{
  dshowdevinfo* devinfo = ccdshow_get_dev(cc, idev);
  return devinfo ? devinfo->pFrameBuffer : NULL;
}

// //////////////////////////////////////////////////////////////////////////////////
// //////////////////////////////////////////////////////////////////////////////////
int cc_idev_pause(camcap* cc, int idev)
{
  dshowdevinfo* devinfo = ccdshow_get_dev(cc, idev);
  if (!devinfo) return CC_FAIL;
  
  HRESULT hr = devinfo->pMediaControl->Pause();
  if (hr != S_OK)
  {
    hr = ccdshow_wait_graph_state(devinfo, State_Paused, 500);
    if ( FAILED(hr) )
      return CC_FAIL;
  }

  return CC_OK;
}

// //////////////////////////////////////////////////////////////////////////////////
// //////////////////////////////////////////////////////////////////////////////////
int cc_idev_stop(camcap* cc, int idev)
{
  dshowdevinfo* devinfo = ccdshow_get_dev(cc, idev);
  if (!devinfo) return CC_FAIL;

  HRESULT hr = devinfo->pMediaControl->Stop();
  if (hr != S_OK)
  {
    hr = ccdshow_wait_graph_state(devinfo, State_Stopped, 500);
    if (FAILED(hr))
      return CC_FAIL;
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
// https://github.com/ofTheo/videoInput/blob/master/videoInputSrcAndDemos/libs/videoInput/videoInput.cpp
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
    devinfo->caps = (dshowdevcap*)calloc(capCount, sizeof(dshowdevcap));
    if (!devinfo->caps) return E_OUTOFMEMORY;

    dshowdevcap* caps = devinfo->caps;    
    for (int iFormat = 0; iFormat < capCount; ++iFormat, ++caps)
    {
      hr = devinfo->pStreamCfg->GetStreamCaps(iFormat, &caps->pMediaType, (BYTE*)&caps->cfgcaps);
      if (SUCCEEDED(hr) && caps->pMediaType->majortype != MEDIATYPE_Video)
        ReleaseMediaType(caps->pMediaType);
    }
  }
  else
  {
    hr = E_FAIL;
  }
  return hr;
}

// //////////////////////////////////////////////////////////////////////////////////
// //////////////////////////////////////////////////////////////////////////////////
int ccdshow_mediasubtype_to_formattype(REFGUID subtype)
{
  if (subtype == MEDIASUBTYPE_RGB24)      return CC_VIDEOFMT_RGB24;
  else if (subtype == MEDIASUBTYPE_RGB32) return CC_VIDEOFMT_RGB32;
  else if (subtype == MEDIASUBTYPE_MJPG)  return CC_VIDEOFMT_MJPEG;
  else if (subtype == MEDIASUBTYPE_IJPG)  return CC_VIDEOFMT_JPEG;
  else if (subtype == MEDIASUBTYPE_I420)  return CC_VIDEOFMT_YUV420P;
  else if (subtype == MEDIASUBTYPE_RGB565)return CC_VIDEOFMT_RGB565;
  else if (subtype == MEDIASUBTYPE_RGB555)return CC_VIDEOFMT_RGB555;

  return CC_VIDEOFMT_UNKNOWN;
}

// //////////////////////////////////////////////////////////////////////////////////
// //////////////////////////////////////////////////////////////////////////////////
void ccdshow_formattype_to_mediasubtype(int format_type, GUID* out_subtype)
{
  switch (format_type)
  {
  case CC_VIDEOFMT_RGB24: *out_subtype = MEDIASUBTYPE_RGB24; break;
  case CC_VIDEOFMT_RGB32: *out_subtype = MEDIASUBTYPE_RGB32; break;
  case CC_VIDEOFMT_MJPEG: *out_subtype = MEDIASUBTYPE_MJPG; break;
  case CC_VIDEOFMT_JPEG:  *out_subtype = MEDIASUBTYPE_IJPG; break;
  case CC_VIDEOFMT_YUV420P:*out_subtype = MEDIASUBTYPE_I420; break;
  case CC_VIDEOFMT_RGB565:*out_subtype = MEDIASUBTYPE_RGB565; break;
  case CC_VIDEOFMT_RGB555:*out_subtype = MEDIASUBTYPE_RGB555; break;
  default: *out_subtype = MEDIASUBTYPE_RGB24;
  }
}

// //////////////////////////////////////////////////////////////////////////////////
// //////////////////////////////////////////////////////////////////////////////////
HRESULT ccdshow_set_framebuffer_size(dshowdevinfo* devinfo, AM_MEDIA_TYPE* pMediaType)
{
  if (!devinfo || !pMediaType || pMediaType->majortype != MEDIATYPE_Video) 
    return E_FAIL;

  VIDEOINFOHEADER* vih = CC_VIH(pMediaType);
  if (!vih) return E_FAIL;

  unsigned int size_image = vih->bmiHeader.biSizeImage;
  if (!size_image)
  {
    // don't like this, because even when the bitcount might be 12, the stride for pixel might be 2 bytes not 1'5
    size_image = vih->bmiHeader.biWidth * vih->bmiHeader.biHeight * vih->bmiHeader.biBitCount;
    size_image >>= 3;
  }

  
  if (!size_image)
    return E_FAIL;

  devinfo->pFrameBuffer = (unsigned char*)cc_realloc(devinfo->pFrameBuffer, size_image);
  if (!devinfo->pFrameBuffer)
    return E_OUTOFMEMORY;
  return S_OK;
}

// //////////////////////////////////////////////////////////////////////////////////
// //////////////////////////////////////////////////////////////////////////////////
HRESULT ccdshow_wait_graph_state(dshowdevinfo* devinfo, OAFilterState desired_st, int timeout_ms)
{
  OAFilterState fs = -1;
  while (timeout_ms > 0 && fs != desired_st)
  {
    devinfo->pMediaControl->GetState(10, &fs);
    timeout_ms -= 10;
  }
  return fs == desired_st ? S_OK : E_FAIL;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////WINDOWS MEDIA CAPTURE IMPLEMENTATION///////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#elif defined(CC_WMC) /*   WINDOWS MEDIA CAPTURE */

#endif

//* COMMON */
const char* cc_get_format_type_name(int format_type)
{
  static const char* names[CC_VIDEOFMT_MAX] = { "RGB24", "RGB32", "MJPEG", "YUV420P", "JPEG", "RGB565", "RGB555" };
  return format_type >= 0 && format_type < CC_VIDEOFMT_MAX ? names[format_type] : "UNKNOWN";
}


#endif // CC_IMPLEMENTATION

#pragma warning(default:4127)

#endif // _CC_H_
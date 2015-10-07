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

#pragma warning(disable:4996 4456 4457)
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image_write.h>
#pragma warning(default:4996 4456 4457)

#define CC_DSHOW
#define CC_IMPLEMENTATION
#include <camcap.hpp>


void err_callback(int err, const char* msg)
{
  printf("0x%8x: %s\n", err, msg);
}

double get_cur_time()
{
  double f;
  LARGE_INTEGER t, freq;
  QueryPerformanceFrequency(&freq);
  f = 1000.0 / (double)(freq.QuadPart);
  QueryPerformanceCounter(&t);
  return (double)(t.QuadPart)*f;
}

// flips vertical/horizontally and BGR to RGB
// no cache friendly
void flip_buff(void* buff, int width, int height, int bitcount)
{
  const int size = width*height * (bitcount>>3);
  const int mid = size >> 1;
  unsigned char* a = (unsigned char*)buff;    
  unsigned char* b = a + size - 1;
  unsigned char t;
  for (int i = 0; i < mid; ++i)
  {
    t = *b;
    *b = *a;
    *a = t;
    ++a;
    --b;
  }
}

///
/// SIMPLE DEMO TO SHOW HOW TO CAPTURE A QUICK SNAPSHOT
/// THIS SAMPLE DOES A BAD ERR CHECKING AND RESOURCE RELEASE
///
void test_camcap1()
{
  camcap* cc = 0;
  camcap_opts opts = { CC_FLAG_VIDEOINPUT, err_callback };

  cc_init(&cc, &opts);
  if (cc_idev_count(cc) == 0) return;
  if (cc_idev_init(cc, 0) != CC_OK) return;
  if (cc_idev_start(cc, 0) != CC_OK) return;
  camcapidev_mode curmode;
  cc_idev_get_current_mode(cc, 0, &curmode);
  while (cc_idev_grab(cc, 0, 100) != CC_OK) {};
  cc_idev_stop(cc, 0);

  double t0 = get_cur_time();
  flip_buff(cc_idev_get_buffer(cc, 0), curmode.width, curmode.height, curmode.bitcount);
  printf("%lf msecs. flipping\n", (get_cur_time() - t0)/1000.0);
  stbi_write_png("defcap.png", curmode.width, curmode.height, curmode.bitcount>>3, cc_idev_get_buffer(cc, 0), curmode.width * (curmode.bitcount >> 3));

  cc_idev_deinit(cc, 0);
  cc_deinit(&cc);
}

///
/// THIS SAMPLE INITIALIZES A WEBCAM, AND TAKES A SNAPSHOT FOR EVERY SUPPORTED RGB24 MODE
///
void test_camcap2()
{
  camcap* cc = 0;
  camcap_opts opts = { CC_FLAG_VIDEOINPUT, err_callback };
  char fname[256];

  // initializes lib
  cc_init(&cc, &opts);

  // is there any video input?
  if (cc_idev_count(cc) > 0)
  {
    // we use the first available one
    int idev = 0;

    // initialize input device
    if (cc_idev_init(cc, idev) == CC_OK)
    {
      // how many modes?
      int modescount = cc_idev_modes(cc, idev, NULL, 0);
      if (modescount > 0)
      {
        // get the modes
        camcapidev_mode* modes = (camcapidev_mode*)calloc(modescount, sizeof(camcapidev_mode));
        cc_idev_modes(cc, idev, modes, modescount);
        
        // sample: take a snapshot of every rgb24 format found and save it to png
        camcapidev_mode* curmode = NULL;
        int iCurMode = 0;
        for (; iCurMode < modescount; ++iCurMode)
        {
          curmode = modes + iCurMode;
          if (curmode->video_format_type == CC_VIDEOFMT_RGB24 /*&& curmode->bitcount == 24*/)
          {
            if (cc_idev_set_mode(cc, idev, curmode) == CC_OK)
            {
              // start capture
              if (cc_idev_start(cc, idev) == CC_OK)
              {
                // grab frame to save into a png
                while (cc_idev_grab(cc, idev, 100) != CC_OK); // risky, infinite loop if can't get the buffer, add
                cc_idev_stop(cc, idev);
                flip_buff(cc_idev_get_buffer(cc, 0), curmode->width, curmode->height, curmode->bitcount);
                sprintf_s(fname, "capture_%dx%d.png", curmode->width, curmode->height);
                stbi_write_png(fname, curmode->width, curmode->height, 3, cc_idev_get_buffer(cc, idev), curmode->width * 3);
              }
            }
          }
        }
        SafeFree(modes);
      }
    }

    // deinitializes input device
    cc_idev_deinit(cc, idev);
  }

  // deinitializes lib
  cc_deinit(&cc);
}

//===-----------------------------------------------------------===//
//===-----------------------------------------------------------===//
int main(int argc, const char** argv)
{
  UNREFERENCED_PARAMETER(argc);
  UNREFERENCED_PARAMETER(argv);

  //test_camcap1();
  test_camcap2();
  return S_OK;
}
# camcap
C++ header file for (web) camera capture.
Windows-only so far.

# desc
This header-only code allows you to grab frame information from a connected video input device (webcam for example).

The idea is to provide a generic API and specific implementations for DirectShow, Windows Media Capture (Win10) and/or Windows Media Foundation (WinVista/7).

# usage

```c++
///
/// how to capture a snapshot from a connected webcam
/// disclaimer: this sample does a bad error checking and resource mgmt.
///
void test_camcap1()
{
  camcap* cc = 0;
  camcap_opts opts = { CC_FLAG_VIDEOINPUT, err_callback };

  // initiliazes lib
  cc_init(&cc, &opts);
  
  // no devices?
  if (cc_idev_count(cc) == 0) 
    return; 
    
  // can't init device 0?
  if (cc_idev_init(cc, 0) != CC_OK) 
    return; 
    
  // can't start device 0?
  if (cc_idev_start(cc, 0) != CC_OK) 
    return; 
    
  // grabs a frame (blocking and storing in the default frame buffer)
  while (cc_idev_grab(cc, 0, 100) != CC_OK) {};
  cc_idev_stop(cc, 0);
  
  // gets the current mode
  camcapidev_mode curmode;
  cc_idev_get_current_mode(cc, 0, &curmode);
  
  // flips and saves the frame in png using stbi
  double t0 = get_cur_time();
  flip_buff(cc_idev_get_buffer(cc, 0), curmode.width, curmode.height, curmode.bitcount);
  printf("%lf msecs. flipping\n", (get_cur_time() - t0)/1000.0);
  stbi_write_png("defcap.png", curmode.width, curmode.height, curmode.bitcount>>3, cc_idev_get_buffer(cc, 0), curmode.width * (curmode.bitcount >> 3));

  // deinit device and lib
  cc_idev_deinit(cc, 0);
  cc_deinit(&cc);
}
```

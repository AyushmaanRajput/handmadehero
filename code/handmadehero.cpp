#include <cstdint>
#include <stdint.h>
#include <windows.h>

#define internal_function static
#define local_persist static
#define global_variable static

typedef uint8_t uint8;
typedef uint16_t uint16;
typedef uint32_t uint32;
typedef uint64_t uint64;

typedef int8_t int8;
typedef int16_t int16;
typedef int32_t int32;
typedef int64_t int64;

global_variable bool Running; // This is a global boolean which tracts whether
global_variable BITMAPINFO BitmapInfo; // TODO: make them unglobal later
global_variable void *BitMapMemory;
global_variable int BitmapWidth;
global_variable int BitmapHeight;

global_variable int XOffset = 0;
global_variable int YOffset = 0;
internal_function void
Win32RenderWeirdGradient(int Width, int Height, int XOffset, int YOffset) {

  // NOTE:THIS IS WHERE WE ACTUALLY START DRAWING
  // NOTE: Once we have the memory we can draw in int
  // case the void* as unit8
  int BytesPerPixel = 4; // because each pixel has 4bytes of info defined above
  int BitmapMemorySize = BytesPerPixel * Width * Height;

  uint8 *Row = (uint8 *)BitMapMemory;
  int Pitch = Width * BytesPerPixel; // Basically how much memory one row of
                                     // pixels on the my game window have
  for (int Y = 0; Y < BitmapHeight; Y++) {
    uint32 *Pixel = (uint32 *)Row;
    for (int X = 0; X < BitmapWidth; X++) {
      /*
       * Pixel in Memory 00 00 00 00
       * Common Sense:   RR GG BB XX(Some padding)
       * */
      /**Pixel = (uint8)(X + XOffset); // NOTE: Writing to First Byte pixel;*/
      /*++Pixel;*/
      /**Pixel = (uint8)(Y + YOffset); // NOTE: Writing to pixel*/
      /*++Pixel;*/
      /**/
      /**Pixel = 0; // NOTE: Writing to pixel*/
      /*++Pixel;*/
      /**/
      /**Pixel = 0; // NOTE: Writing to pixel*/
      /*++Pixel;*/

      // NOTE: For 32 bits calculations
      /*
       * Memory:    BB GG RR XX
       * Register:  XX RR GG BB
       * so to write to the greet bits we have to shift by 8 and for red 16
       *
       * */
      uint8 Blue = (X + XOffset);
      uint8 Green = (Y + YOffset);
      uint8 Red = ((X + Y) + (XOffset + YOffset));
      *Pixel++ = ((Green << 8) | Blue | (Red << 16));
    }
    Row += Pitch;
  }
}

internal_function void Win32ResizeDIBSection(int Width, int Height) {
  // TODO: For now we are
  if (BitMapMemory) {
    VirtualFree(BitMapMemory, NULL, MEM_RELEASE);
  }
  // fill bitmapinfo
  // typedef struct tagBITMAPINFO {
  // BITMAPINFOHEADER bmiHeader;
  // RGBQUAD          bmiColors[1];
  //} BITMAPINFO, *LPBITMAPINFO, *PBITMAPINFO;
  BitmapWidth = Width;
  BitmapHeight = Height;
  BitmapInfo.bmiHeader.biSize = sizeof(BitmapInfo.bmiHeader);
  BitmapInfo.bmiHeader.biWidth = Width;
  BitmapInfo.bmiHeader.biHeight = -Height; // Top down frame rows
  BitmapInfo.bmiHeader.biPlanes = 1;
  BitmapInfo.bmiHeader.biBitCount = 32; // NOTE: Will dicuss later
  BitmapInfo.bmiHeader.biCompression = BI_RGB;
  BitmapInfo.bmiHeader.biSizeImage = 0;
  BitmapInfo.bmiHeader.biXPelsPerMeter = 0;
  BitmapInfo.bmiHeader.biClrImportant = 0;
  BitmapInfo.bmiHeader.biClrUsed = 0;

  /*    typedef struct tagBITMAPINFOHEADER {
    DWORD biSize;
    LONG  biWidth;
    LONG  biHeight;
    WORD  biPlanes;
    WORD  biBitCount;
    DWORD biCompression;
    DWORD biSizeImage;
    LONG  biXPelsPerMeter;
    LONG  biYPelsPerMeter;
    DWORD biClrUsed;
    DWORD biClrImportant;
  } BITMAPINFOHEADER, *LPBITMAPINFOHEADER, *PBITMAPINFOHEADER;
  */

  int BytesPerPixel = 4; // because each pixel has 4bytes of info defined above
  int BitmapMemorySize = BytesPerPixel * Width * Height;
  // NOTE: We have to ask windows for memory so we can use either virtual alloc
  // or heap alloc
  BitMapMemory = VirtualAlloc( // these allocates memory in terms of pages
      0,                       // we don't really care where the memory is
      BitmapMemorySize,
      MEM_COMMIT,
      PAGE_READWRITE);

  /*Win32RenderWeirdGradient(Width, Height, ++XOffset, ++YOffset);*/
}

internal_function void Win32UpdateWindow(
    HDC DeviceContext,
    RECT *WindowRect,
    int X,
    int Y,
    int Width,
    int Height) {
  int WindowWidth = WindowRect->right - WindowRect->left;
  int WindowHeight = WindowRect->bottom - WindowRect->top;

  StretchDIBits(
      DeviceContext,
      0,
      0,
      BitmapWidth,
      BitmapHeight,
      0,
      0,
      WindowWidth,
      WindowHeight,
      BitMapMemory,
      &BitmapInfo,
      DIB_RGB_COLORS,
      SRCCOPY); // we will be doing direct rgb writing into our buffer
}

LRESULT CALLBACK Win32MainWindowCallback(
    HWND windowHandle,
    UINT message,
    WPARAM WParam,
    LPARAM LParam) {
  LRESULT Result = 0;
  switch (message) {
  case WM_PAINT: {
    PAINTSTRUCT PaintStruct;
    HDC DeviceContext = BeginPaint(windowHandle, &PaintStruct);

    int X = PaintStruct.rcPaint.left;
    int Y = PaintStruct.rcPaint.top;
    LONG width = PaintStruct.rcPaint.right - PaintStruct.rcPaint.left;
    LONG height = PaintStruct.rcPaint.bottom - PaintStruct.rcPaint.top;
    RECT ClientRect;
    GetClientRect(
        windowHandle,
        &ClientRect); // size of the acutal screen rectangle/window we
                      // can draw into

    Win32UpdateWindow(DeviceContext, &ClientRect, X, Y, width, height);
    /*local_persist DWORD Operation =*/
    /*    WHITENESS; // NOTE: Don't use static villy nilly*/
    /*PatBlt(DeviceContext, X, Y, width, height, Operation);*/
    EndPaint(windowHandle, &PaintStruct);
  } break;
  case WM_SIZE: {
    RECT ClientRect;
    GetClientRect(
        windowHandle,
        &ClientRect); // size of the acutal screen rectangle/window we
                      // can draw into
    int X = ClientRect.left;
    int Y = ClientRect.top;
    LONG width = ClientRect.right - X;
    LONG height = ClientRect.bottom - Y;
    Win32ResizeDIBSection(
        width,
        height); // our function that draws to a fixed buffer

    OutputDebugStringA("change size\n ");
  } break;
  case WM_DESTROY: {
    Running = false;
    OutputDebugStringA("destroy\n");
  } break;
  case WM_CLOSE: {
    // PostQuitMessage(0);
    Running = false;
    OutputDebugStringA("Close The Window\n");
  } break;
  case WM_ACTIVATEAPP: {
    OutputDebugStringA("Activate App\n");
  } break;
  default: {
    OutputDebugStringA("Default\n");
    Result = DefWindowProcA(windowHandle, message, WParam, LParam);
  } break;
  }
  return (Result);
}
int CALLBACK WinMain(
    HINSTANCE Instance,
    HINSTANCE PrevInstance,
    LPSTR CommandLine,
    int ShowCode) {
  WNDCLASS WindowClass =
      {}; // sets all properties of Window class like uint style etc to 0.

  WindowClass.style =
      CS_OWNDC | CS_HREDRAW |
      CS_VREDRAW; // UINT style: binary flags for how the window will look ;
  WindowClass.lpfnWndProc =
      Win32MainWindowCallback;      // main window prodecure: here we define a
                                    // function that will answer to windows
                                    // events are us
  WindowClass.hInstance = Instance; // HINSTANCE hInstance;
  WindowClass.lpszClassName =
      "HandmadeHeroWindowClass"; // LPCSTR    lpszClassName;
  // We won't be setting more props for now

  // NOTE: After creating the window we have to register it
  if (RegisterClassA(&WindowClass)) {
    // registering success
    HWND WindowHandle = CreateWindowExA(
        0,
        WindowClass.lpszClassName,
        "Handmade Hero | Ayushmaan",
        WS_OVERLAPPEDWINDOW | WS_VISIBLE,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        0,
        0,
        Instance,
        0);
    if (WindowHandle) {
      //
      MSG Message;
      Running = true;
      while (Running) {
        while (PeekMessageA(
            &Message,
            0,
            0,
            0,
            PM_REMOVE)) { // PeekMessage only processes the message when there
                          // is something there and is non-blocking unlike
                          // GetMessagge
          if (Message.message == WM_QUIT) {
            Running = false;
          }
          TranslateMessage(&Message);
          DispatchMessageA(&Message);
        }
        RECT ClientRect;
        GetClientRect(
            WindowHandle,
            &ClientRect); // size of the acutal screen rectangle/window we
                          // can draw into
        int X = ClientRect.left;
        int Y = ClientRect.top;
        LONG width = ClientRect.right - X;
        LONG height = ClientRect.bottom - Y;

        Win32RenderWeirdGradient(width, height, ++XOffset, ++YOffset);
        HDC DeviceContext = GetDC(WindowHandle);
        Win32UpdateWindow(DeviceContext, &ClientRect, X, Y, width, height);
        ReleaseDC(WindowHandle, DeviceContext);
        /*BOOL messageResult = GetMessage(&Message, 0, 0, 0);*/
        /*if (messageResult > 0) {*/
        /*  // its not WM_QUIT ==0*/
        /*  // Translating and dispatching is absolute windows minutia*/
        /*  TranslateMessage(&Message);*/
        /*  DispatchMessageA(&Message);*/
        /*} else {*/
        /*  // break: User closed the window*/
        /*  break;*/
        /*}*/
      }

    } else {
      // handle error
    }
  } else {
    // rare case: registering fails
  }
  return (0);
}

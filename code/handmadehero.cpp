#include <windows.h>

#define internal_function static
#define local_persist static
#define global_variable static

global_variable bool Running; // This is a global boolean which tracts whether
                              // our program/window is running or not
global_variable BITMAPINFO BitmapInfo;// TODO: make them unglobal later
global_variable void *BitMapMemory;
global_variable HBITMAP BitmapHandle;
global_variable HDC BitmapDeviceContext;

internal_function void Win32ResizeDIBSection(int Width, int Height) {

  // TODO: For now we are
  if(BitmapInfo.bmiHeader.biSize){
    // already allocated so free it using DeleteObject
    DeleteObject(BitmapHandle);
  }else{
  //NOTE: We create a global  the device context for bitmap here 
    BitmapDeviceContext= CreateCompatibleDC(0);
  }
  // fill bitmapinfo
  // typedef struct tagBITMAPINFO {
  // BITMAPINFOHEADER bmiHeader;
  // RGBQUAD          bmiColors[1];
  //} BITMAPINFO, *LPBITMAPINFO, *PBITMAPINFO;
  BitmapInfo.bmiHeader.biSize = sizeof(BitmapInfo.bmiHeader);
  BitmapInfo.bmiHeader.biWidth = Width;
  BitmapInfo.bmiHeader.biHeight = Height;
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

  BitmapHandle =
      CreateDIBSection(BitmapDeviceContext, &BitmapInfo, DIB_RGB_COLORS,
                       &BitMapMemory,
                       0, 0);// The bit map memory is what we will get
                                      // from window to draw to
}

internal_function void Win32UpdateWindow(HDC DeviceContext, int X, int Y,
                                         int Width, int Height) {
  int StretchDIBits(
      DeviceContext, X, Y, Width, Height, X, Y, Width,
      Height, [in] const VOID *lpBits, [in] const BITMAPINFO *lpbmi,
      DIB_RGB_COLORS,
      SRCCOPY); // we will be doing direct rgb writing into our buffer
}

LRESULT CALLBACK Win32MainWindowCallback(HWND windowHandle, UINT message,
                                         WPARAM WParam, LPARAM LParam) {
  LRESULT Result = 0;
  switch (message) {
  case WM_PAINT: {
    PAINTSTRUCT PaintStruct;
    HDC DeviceContext = BeginPaint(windowHandle, &PaintStruct);

    int X = PaintStruct.rcPaint.left;
    int Y = PaintStruct.rcPaint.top;
    LONG width = PaintStruct.rcPaint.right - PaintStruct.rcPaint.left;
    LONG height = PaintStruct.rcPaint.bottom - PaintStruct.rcPaint.top;
    Win32UpdateWindow(DeviceContext, X, Y, width, height);
    local_persist DWORD Operation =
        WHITENESS; // NOTE: Don't use static villy nilly
    PatBlt(DeviceContext, X, Y, width, height, Operation);
    if (Operation == WHITENESS) {
      Operation = BLACKNESS;
    } else {
      Operation = WHITENESS;
    }
    EndPaint(windowHandle, &PaintStruct);
  } break;
  case WM_SIZE: {
    RECT ClientRect;
    GetClientRect(windowHandle,
                  &ClientRect); // size of the acutal screen rectangle/window we
                                // can draw into
    int X = ClientRect.left;
    int Y = ClientRect.top;
    LONG width = ClientRect.right - X;
    LONG height = ClientRect.bottom - Y;
    Win32ResizeDIBSection(DeviceContext, width,
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
int CALLBACK WinMain(HINSTANCE Instance, HINSTANCE PrevInstance,
                     LPSTR CommandLine, int ShowCode) {

  WNDCLASS WindowClass =
      {}; // sets all properties of Window class like uint style etc to 0.

  WindowClass.style =
      CS_OWNDC | CS_HREDRAW |
      CS_VREDRAW; // UINT style: binary flags for how the window will look ;
  WindowClass.lpfnWndProc =
      Win32MainWindowCallback; // main window prodecure: here we define a
                               // function that will answer to windows events
                               // are us
  WindowClass.hInstance = Instance; // HINSTANCE hInstance;
  WindowClass.lpszClassName =
      "HandmadeHeroWindowClass"; // LPCSTR    lpszClassName;
  // We won't be setting more props for now

  // NOTE: After creating the window we have to register it
  if (RegisterClassA(&WindowClass)) {
    // registering success
    HWND WindowHandle = CreateWindowExA(
        0, WindowClass.lpszClassName, "Handmade Hero | Ayushmaan",
        WS_OVERLAPPEDWINDOW | WS_VISIBLE, CW_USEDEFAULT, CW_USEDEFAULT,
        CW_USEDEFAULT, CW_USEDEFAULT, 0, 0, Instance, 0);
    if (WindowHandle) {
      //
      MSG Message;
      Running = true;
      while (Running) {
        BOOL messageResult = GetMessage(&Message, 0, 0, 0);
        if (messageResult > 0) {
          // its not WM_QUIT ==0
          // Translating and dispatching is absolute windows minutia
          TranslateMessage(&Message);
          DispatchMessageA(&Message);
        } else {
          // break: User closed the window
          break;
        }
      }

    } else {
      // handle error
    }
  } else {
    // rare case: registering fails
  }
  return (0);
}

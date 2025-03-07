#include <windows.h>
/*typedef struct tagWNDCLASSA {
  UINT      style;
  WNDPROC   lpfnWndProc;
  int       cbClsExtra;
  int       cbWndExtra;
  HINSTANCE hInstance;
  HICON     hIcon;
  HCURSOR   hCursor;
  HBRUSH    hbrBackground;
  LPCSTR    lpszMenuName;
  LPCSTR    lpszClassName;
} WNDCLASSA, *PWNDCLASSA, *NPWNDCLASSA, *LPWNDCLASSA;
*/

LRESULT CALLBACK MainWindowCallback(HWND windowHandle, UINT message,
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
    static DWORD Operation = WHITENESS; // NOTE: Don't use static villy nilly
    PatBlt(DeviceContext, X, Y, width, height, Operation);
    if (Operation == WHITENESS) {
      Operation = BLACKNESS;
    } else {
      Operation = WHITENESS;
    }
    EndPaint(windowHandle, &PaintStruct);
  } break;
  case WM_SIZE: {
    OutputDebugStringA("change size\n ");
  } break;
  case WM_DESTROY: {
    OutputDebugStringA("destroy\n");
  } break;
  case WM_CLOSE: {
    OutputDebugStringA("Close\n");
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
      MainWindowCallback; // main window prodecure: here we define a function
                          // that will answer to windows events are us
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
      for (;;) {
        BOOL messageResult = GetMessage(&Message, 0, 0, 0);
        if (messageResult > 0) {
          // its not WM_QUIT ==0
          // Translating and dispatching is absolute windows minutia
          TranslateMessage(&Message);
          DispatchMessageA(&Message);
        } else {
          // break: User closed the window
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

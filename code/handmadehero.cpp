#include <math.h>
#include <stdint.h>
#include <windows.h>
#include <xaudio2.h>
#include <xinput.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#pragma comment(lib, "xaudio2.lib")
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

typedef float real32;
typedef double real64;
// ========================
// Audio System Declarations
// ========================

global_variable IXAudio2 *XAudio2Engine; // NOTE: This is like the thing that
                                         // manages all the audio processing
global_variable IXAudio2MasteringVoice
    *MasteringVoice; // NOTE: This is the actaul speaker output, handles
                     // converting digitial sound to actual audio waves
global_variable IXAudio2SourceVoice
    *SourceVoice; // NOTE: This is like a audio cd(track ) that we will later
                  // feed the audio data into
global_variable WAVEFORMATEX WaveFormat;
global_variable XAUDIO2_BUFFER AudioBuffer;
global_variable int16 *WaveBuffer;
global_variable uint32 WaveBufferSize;

internal_function bool InitAudioSystem() {
  HRESULT hr;

  // Initialize COM for XAudio2
  hr = CoInitializeEx(nullptr, COINIT_MULTITHREADED);
  if (FAILED(hr))
    return false;

  // Create XAudio2 engine
  hr = XAudio2Create(&XAudio2Engine, 0, XAUDIO2_DEFAULT_PROCESSOR);
  if (FAILED(hr))
    return false;

  // Create mastering voice
  hr = XAudio2Engine->CreateMasteringVoice(&MasteringVoice);
  if (FAILED(hr))
    return false;

  // Setup wave format (PCM, 44.1kHz, 16-bit, mono)
  WaveFormat.wFormatTag = WAVE_FORMAT_PCM;
  WaveFormat.nChannels = 1;
  WaveFormat.nSamplesPerSec = 44100;
  WaveFormat.nAvgBytesPerSec = 44100 * sizeof(int16);
  WaveFormat.nBlockAlign = sizeof(int16);
  WaveFormat.wBitsPerSample = 16;
  WaveFormat.cbSize = 0;

  // Create source voice
  hr = XAudio2Engine->CreateSourceVoice(&SourceVoice, &WaveFormat);
  return SUCCEEDED(hr);
}

internal_function void ShutdownAudioSystem() {
  if (SourceVoice) {
    SourceVoice->Stop(0);
    SourceVoice->DestroyVoice();
    SourceVoice = nullptr;
  }
  if (MasteringVoice) {
    MasteringVoice->DestroyVoice();
    MasteringVoice = nullptr;
  }
  if (XAudio2Engine) {
    XAudio2Engine->Release();
    XAudio2Engine = nullptr;
  }
  if (WaveBuffer) {
    VirtualFree(WaveBuffer, 0, MEM_RELEASE);
    WaveBuffer = nullptr;
  }
  CoUninitialize();
}

internal_function bool
CreateSineWaveBuffer(uint32 FrequencyHz, real32 DurationSeconds) {
  // Calculate buffer parameters
  const uint32 samplesPerSecond = 44100;
  const uint32 totalSamples = (uint32)(samplesPerSecond * DurationSeconds);
  const uint32 bufferSize = totalSamples * sizeof(int16);

  // Allocate buffer memory
  WaveBuffer = (int16 *)VirtualAlloc(0, bufferSize, MEM_COMMIT, PAGE_READWRITE);
  if (!WaveBuffer)
    return false;

  // Define wave amplitude (max 16-bit signed value, e.g., 10000)
  const real32 Amplitude = 10000.0f;

  // Generate sine wave pattern
  for (uint32 i = 0; i < totalSamples; ++i) {
    // Calculate the time for the current sample (i / sampleRate)
    real32 time = (real32)i / (real32)samplesPerSecond;
    // Compute the sine wave sample
    real32 sample = Amplitude * sinf(2.0f * (real32)M_PI * FrequencyHz * time);
    // Cast to int16 and store
    WaveBuffer[i] = (int16)sample;
  }

  // Configure audio buffer
  ZeroMemory(&AudioBuffer, sizeof(XAUDIO2_BUFFER));
  AudioBuffer.AudioBytes = bufferSize;
  AudioBuffer.pAudioData = (BYTE *)WaveBuffer;
  AudioBuffer.Flags = XAUDIO2_END_OF_STREAM;
  AudioBuffer.LoopCount = XAUDIO2_LOOP_INFINITE;

  return true;
}

internal_function void PlayWave() {
  if (!SourceVoice)
    return;

  // Stop and flush any existing buffers
  SourceVoice->Stop(0);
  SourceVoice->FlushSourceBuffers();

  // Submit new buffer and start playing
  HRESULT hr = SourceVoice->SubmitSourceBuffer(&AudioBuffer);
  if (SUCCEEDED(hr)) {
    SourceVoice->Start(0);
  }
}

// Define function signatures for XInput functions
#define X_INPUT_GET_STATE(name)                                                \
  DWORD WINAPI name(DWORD dwUserIndex, XINPUT_STATE *pState)

#define X_INPUT_SET_STATE(name)                                                \
  DWORD WINAPI name(DWORD dwUserIndex, XINPUT_VIBRATION *pVibration)

typedef X_INPUT_GET_STATE(x_input_get_state);
typedef X_INPUT_SET_STATE(x_input_set_state);

// Stub functions for when XInput is unavailable
X_INPUT_GET_STATE(XInputGetStateStub) { return ERROR_DEVICE_NOT_CONNECTED; }

X_INPUT_SET_STATE(XInputSetStateStub) { return ERROR_DEVICE_NOT_CONNECTED; }

// Global function pointers initialized to stubs
global_variable x_input_get_state *XInputGetState_ = XInputGetStateStub;
global_variable x_input_set_state *XInputSetState_ = XInputSetStateStub;
// Macros to use function pointers
#define XInputGetState XInputGetState_
#define XInputSetState XInputSetState_
// Function to dynamically load XInput functions
internal_function void LoadXInput() {
  HMODULE XInputLibrary = LoadLibraryA("xinput1_4.dll"); // Windows 8+

  if (!XInputLibrary) {
    XInputLibrary = LoadLibraryA("xinput1_3.dll"); // Windows 7
  }
  if (!XInputLibrary) {
    XInputLibrary = LoadLibraryA("xinput9_1_0.dll"); // Fallback
  }

  if (XInputLibrary) {
    XInputGetState_ =
        (x_input_get_state *)GetProcAddress(XInputLibrary, "XInputGetState");
    XInputSetState_ =
        (x_input_set_state *)GetProcAddress(XInputLibrary, "XInputSetState");

    if (!XInputGetState_)
      XInputGetState_ = XInputGetStateStub;
    if (!XInputSetState_)
      XInputSetState_ = XInputSetStateStub;
  }
}

struct win32_offscreen_buffer {
  /* NOTE: Pixels are always 32bits wide (4 bytes) in Memeory Order BB GG RR XX
   */
  BITMAPINFO Info; // TODO: make them unglobal later
  void *Memory;
  int Width;
  int Height;
  int Pitch;
};

struct win32_window_dimension {
  int Width;
  int Height;
};
/*
 * This is where we define our Global Variables
 */

global_variable bool Running; // This is a global boolean which tracts whether
global_variable win32_offscreen_buffer GlobalBackBuffer;

/*
 *
 */

internal_function win32_window_dimension Win32GetWindowDimension(HWND Window) {
  win32_window_dimension WindowDimension;
  RECT ClientRect;
  GetClientRect(Window, &ClientRect);
  WindowDimension.Width = ClientRect.right - ClientRect.left;
  WindowDimension.Height = ClientRect.bottom - ClientRect.top;

  return (WindowDimension);
}
internal_function void Win32RenderWeirdGradient(
    win32_offscreen_buffer *Buffer,
    int XOffset,
    int YOffset) {

  // NOTE:THIS IS WHERE WE ACTUALLY START DRAWING
  // NOTE: Once we have the memory we can draw in int
  // case the void* as unit8

  uint8 *Row = (uint8 *)Buffer->Memory;
  // pixels on the my game window have
  for (int Y = 0; Y < Buffer->Height; Y++) {
    uint32 *Pixel = (uint32 *)Row;
    for (int X = 0; X < Buffer->Width; X++) {
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
    Row += Buffer->Pitch;
  }
}
internal_function void Win32DisplayBufferInWindow(
    HDC DeviceContext,
    win32_offscreen_buffer *Buffer,
    int Width,
    int Height) {
  int WindowWidth = Width;
  int WindowHeight = Height;

  // TODO: Aspect Ratio Correction to be done
  StretchDIBits(
      DeviceContext,
      0,
      0,
      WindowWidth,
      WindowHeight,
      0,
      0,
      Buffer->Width,
      Buffer->Height,
      Buffer->Memory,
      &Buffer->Info,
      DIB_RGB_COLORS,
      SRCCOPY); // we will be doing direct rgb writing into our buffer
}

internal_function void
Win32ResizeDIBSection(win32_offscreen_buffer *Buffer, int Width, int Height) {
  // TODO: For now we are
  if (Buffer->Memory) {
    VirtualFree(Buffer->Memory, NULL, MEM_RELEASE);
  }

  Buffer->Width = Width;
  Buffer->Height = Height;
  int BytesPerPixel = 4;

  Buffer->Info.bmiHeader.biSize = sizeof(Buffer->Info.bmiHeader);
  Buffer->Info.bmiHeader.biWidth = Width;
  Buffer->Info.bmiHeader.biHeight = -Height; // Top down frame rows
  Buffer->Info.bmiHeader.biPlanes = 1;
  Buffer->Info.bmiHeader.biBitCount = 32; // NOTE: Will dicuss later
  Buffer->Info.bmiHeader.biCompression = BI_RGB;
  Buffer->Info.bmiHeader.biSizeImage = 0;
  Buffer->Info.bmiHeader.biXPelsPerMeter = 0;
  Buffer->Info.bmiHeader.biClrImportant = 0;
  Buffer->Info.bmiHeader.biClrUsed = 0;

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

  int BitmapMemorySize = (BytesPerPixel)*Width * Height;
  // NOTE: We have to ask windows for memory so we can use either virtual alloc
  // or heap alloc
  Buffer->Memory = VirtualAlloc( // these allocates memory in terms of pages
      0,                         // we don't really care where the memory is
      BitmapMemorySize,
      MEM_COMMIT,
      PAGE_READWRITE);

  Buffer->Pitch = Width * BytesPerPixel;
  /*Win32RenderWeirdGradient(GlobalBackBuffer, ++XOffset, ++YOffset);*/
}

LRESULT CALLBACK Win32MainWindowCallback(
    HWND windowHandle,
    UINT message,
    WPARAM WParam,
    LPARAM LParam) {
  LRESULT Result = 0;
  switch (message) {
  case WM_KEYDOWN:
  case WM_SYSKEYDOWN:
  case WM_KEYUP:
  case WM_SYSKEYUP: {
    uint32 vkCode = WParam;
    bool wasDown = ((LParam & (1 << 30)) != 0);
    bool isDown = ((LParam & (1 << 31)) == 0);
    if (isDown != wasDown) {
      if (vkCode == 'W') {
        /*OutputDebugStringA("W\n");*/
        /*if (isDown) {*/
        /*  OutputDebugStringA("Is down");*/
        /*}*/
        /*if (wasDown) {*/
        /*  OutputDebugStringA("was down");*/
        /*}*/
        /*OutputDebugStringA("\n");*/
      } else if (vkCode == 'A') {
      } else if (vkCode == 'S') {
      } else if (vkCode == 'D') {
      } else if (vkCode == VK_SPACE) {
      } else if (vkCode == VK_LEFT) {
      } else if (vkCode == VK_RIGHT) {
      } else if (vkCode == VK_UP) {
      } else if (vkCode == VK_DOWN) {
      } else if (vkCode == 'Q') {
      } else if (vkCode == 'E') {
      } else if (vkCode == VK_ESCAPE) {
      }
    }
  } break;
  case WM_PAINT: {
    PAINTSTRUCT PaintStruct;
    HDC DeviceContext = BeginPaint(windowHandle, &PaintStruct);
    win32_window_dimension Dimension = Win32GetWindowDimension(windowHandle);
    Win32DisplayBufferInWindow(
        DeviceContext, &GlobalBackBuffer, Dimension.Width, Dimension.Height);
    EndPaint(windowHandle, &PaintStruct);
  } break;
  case WM_SIZE: {
  } break;
  case WM_DESTROY: {
    Running = false;
  } break;
  case WM_CLOSE: {
    // PostQuitMessage(0);
    Running = false;
  } break;
  case WM_ACTIVATEAPP: {
  } break;
  default: {
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
  // Get Performance frequency of the processor??
  LARGE_INTEGER PerfCountFrequency;
  QueryPerformanceFrequency(&PerfCountFrequency);
  int64 PerformanceFrequency = PerfCountFrequency.QuadPart;

  LoadXInput();

  if (!InitAudioSystem()) {
    // Handle audio initialization failure
    return 1;
  }

  // Create 440Hz square wave (A4 note) that lasts 1 second (but will loop)
  if (!CreateSineWaveBuffer(440, 1.0f)) {
    // Handle buffer creation failure
    ShutdownAudioSystem();
    return 1;
  }

  // Start playing the square wave
  PlayWave();
  WNDCLASS WindowClass =
      {}; // sets all properties of Window class like uint style etc to 0.
  // Make the back buffer first
  Win32ResizeDIBSection(
      &GlobalBackBuffer,
      1280,
      720); // our function that draws to a fixed buffer

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
      /*
       * NOTE: Since we have specific OWN_DC, we can just get one device context
       * and use it forever because we are not sharing it with anyone
       */
      HDC DeviceContext = GetDC(WindowHandle);
      MSG Message;
      Running = true;
      int XOffset = 0;
      int YOffset = 0;
      LARGE_INTEGER LastCounter;
      QueryPerformanceCounter(&LastCounter);
      while (Running) {
        // Step1: Read the message of the queue
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
        /*
         * NOTE: Xinput is a polling based API so we will only get the
         * controller input when we ask for it.
         * TODO: Figure out if we have to poll it more frequently than each
         * frame
         */
        for (DWORD ControllerIndex = 0; ControllerIndex < XUSER_MAX_COUNT;
             ++ControllerIndex) {
          XINPUT_STATE ControllerState;
          if (XInputGetState_(ControllerIndex, &ControllerState) ==
              ERROR_SUCCESS) {
            // Controller is connected

            XINPUT_GAMEPAD *GamePad = &ControllerState.Gamepad;

            bool Up = (GamePad->wButtons & XINPUT_GAMEPAD_DPAD_UP);
            bool Down = (GamePad->wButtons & XINPUT_GAMEPAD_DPAD_DOWN);
            bool Left = (GamePad->wButtons & XINPUT_GAMEPAD_DPAD_LEFT);
            bool Right = (GamePad->wButtons & XINPUT_GAMEPAD_DPAD_RIGHT);
            bool Start = (GamePad->wButtons & XINPUT_GAMEPAD_START);
            bool Back = (GamePad->wButtons & XINPUT_GAMEPAD_BACK);
            bool LeftShoulder =
                (GamePad->wButtons & XINPUT_GAMEPAD_LEFT_SHOULDER);
            bool RightShoulder =
                (GamePad->wButtons & XINPUT_GAMEPAD_RIGHT_SHOULDER);
            bool AButton = (GamePad->wButtons & XINPUT_GAMEPAD_A);
            bool BButton = (GamePad->wButtons & XINPUT_GAMEPAD_B);
            bool XButton = (GamePad->wButtons & XINPUT_GAMEPAD_X);
            bool YButton = (GamePad->wButtons & XINPUT_GAMEPAD_Y);

            int16 StickX = GamePad->sThumbLX;
            int16 StickY = GamePad->sThumbLY;
            if (AButton) {
              ++YOffset;
            }

            if (BButton) {
              ++XOffset;
            }
          } else {
            // Controller is not connected
            // This is important as we might want to show that a particular user
            // has disconnected etc.
          }
        }

        // XINPUT_VIBRATION Vibration;
        // Vibration.wLeftMotorSpeed = 60000;
        // Vibration.wRightMotorSpeed = 60000;
        // XInputSetState(0, &Vibration);

        // Step2: Render(calcualte) the gradient in background inside of our
        // backbuffer
        Win32RenderWeirdGradient(&GlobalBackBuffer, XOffset, YOffset);
        win32_window_dimension Dimension =
            Win32GetWindowDimension(WindowHandle);
        // Step3: Display our back buffer ( Acutal Rendering to the screen )
        Win32DisplayBufferInWindow(
            DeviceContext,
            &GlobalBackBuffer,
            Dimension.Width,
            Dimension.Height);

        LARGE_INTEGER EndCounter;
        QueryPerformanceCounter(&EndCounter);
        int64 CounterElapsed = EndCounter.QuadPart - LastCounter.QuadPart;
        int64 TimeElapsed =
            (CounterElapsed * 1000) /
            PerformanceFrequency; // NOTE: this will tend to 0 so we multiply
                                  // the counterElpased by 1000 to give value in
                                  // milliseconds/Frame
        int64 FPS= 1000/TimeElapsed;

        char Buffer[256];
        wsprintfA(Buffer, "Milliseconds/frame: %dms FPS: %d\n", TimeElapsed, FPS);
        OutputDebugStringA(Buffer);

        LastCounter = EndCounter;
      }

    } else {
      // handle error
    }
  } else {
    // rare case: registering fails
  }
  // Cleanup at end of program
  ShutdownAudioSystem();
  return (0);
}

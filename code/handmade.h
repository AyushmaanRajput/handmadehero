#if !defined(HANDMADE_H)
#define HANDMADE_H

#include <stdint.h>
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

#define Pi32 3.14159265358979323846f
/*
 * NOTE: Ayushmaan: Services that the game provides to the platform (Windows,
 * Linux etc) */
#define internal_function static
#define local_persist static
#define global_variable static

global_variable uint64 RunningSampleIndex = 0;
struct game_offscreen_buffer {
  /* NOTE: Pixels are always 32bits wide (4 bytes) in Memeory Order BB GG RR XX
   */
  // BITMAPINFO Info; // TODO: make them unglobal later
  void *Memory;
  int Width;
  int Height;
  int Pitch;
};

internal_function void GameUpdateAndRender(
    game_offscreen_buffer *Buffer,
    int GreenOffset,
    int BlueOffset);

struct game_sound_output_buffer {
  int16 *Samples;
  uint32 SampleCount;
  uint32 SamplesPerSecond;
  uint32 RunningSampleIndex;  // For continuous waveform generation
  uint64 TotalSamplesElapsed; // persistent across buffers
  int ToneHz;
};
internal_function void
GameGenerateSoundSamples(game_sound_output_buffer *SoundBuffer);
#endif

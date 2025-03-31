/*
 * NOTE: Ayushmaan: This is platform independent code
 */

#include "handmade.h"
#include <math.h>

internal_function void
RenderWeirdGradient(game_offscreen_buffer *Buffer, int XOffset, int YOffset) {

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
       * so to write to the green bits we have to shift by 8 and for red 16
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

internal_function void GameGenerateSoundSamples(
    game_sound_output_buffer *SoundBuffer,
    uint64 StartSample) {

  real32 ToneVolume = 10000.0f;
  uint32 WavePeriod = SoundBuffer->SamplesPerSecond / SoundBuffer->ToneHz;

  for (uint32 SampleIndex = 0; SampleIndex < SoundBuffer->SampleCount;
       SampleIndex += 2) {
    uint64 TotalSample = StartSample + (SampleIndex / 2);
    real32 SineValue =
        sinf(2.0f * Pi32 * (real32)TotalSample / (real32)WavePeriod);

    SoundBuffer->Samples[SampleIndex] = (int16)(SineValue * ToneVolume);
    SoundBuffer->Samples[SampleIndex + 1] = (int16)(SineValue * ToneVolume);
  }

  // Update global sample counter
  RunningSampleIndex += SoundBuffer->SampleCount / 2;
}

void GameUpdateAndRender(
    game_offscreen_buffer *Buffer,
    int GreenOffset,
    int BlueOffset) {
  RenderWeirdGradient(Buffer, BlueOffset, GreenOffset);
};

@echo off
mkdir C:\Users\hp\Desktop\handmadehero\build
pushd C:\Users\hp\Desktop\handmadehero\build
cl -Zi C:\Users\hp\Desktop\handmadehero\code\win32_handmadehero.cpp /link user32.lib gdi32.lib ole32.lib xaudio2.lib
popd


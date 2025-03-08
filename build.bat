@echo off
mkdir C:\Users\PC\Desktop\handmadehero\build
pushd C:\Users\PC\Desktop\handmadehero\build
cl -Zi C:\Users\PC\Desktop\handmadehero\code\handmadehero.cpp /link user32.lib gdi32.lib
popd


FOR /L %%i IN (1,1,15) DO (
    call "%~dp0\unsharp_mask.exe" lena.ppm lena-processed-%%i.ppm %%i cpu
)
pause
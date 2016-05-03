FOR /L %%i IN (1,1,15) DO (
    call "%~dp0\unsharp_mask.exe" ghost-town-8k.ppm ghost-town-8k-processed-%%i.ppm %%i cpu
)
pause
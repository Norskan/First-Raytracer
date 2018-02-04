@echo off
pushd .\run_tree
RayTracer.exe

start "" result.bmp
popd
@echo on
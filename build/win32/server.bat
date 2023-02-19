@echo off
setlocal

pushd ..\..\binary\web
start python -m http.server
popd

endlocal

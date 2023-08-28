/*////////////////////////////////////////////////////////////////////////////*/

// :WindowsOpenGL
//
// We are just always using OpenGL even though we have the Direct3D backend, as that's what we originally shipped with.
//
// #if defined(NK_OS_WIN32)
// #include "renderer_direct3d.cpp"
// #else
#include "renderer_opengl.cpp"
// #endif

/*////////////////////////////////////////////////////////////////////////////*/

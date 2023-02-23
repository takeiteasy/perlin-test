//
//  platform.h
//  sokol
//
//  Created by George Watson on 23/02/2023.
//

#ifndef platform_h
#define platform_h

#if defined(__EMSCRIPTEN__) || defined(EMSCRIPTEN)
#define WEB_BUILD 1
#else
#define WEB_BUILD 0
#endif

#if defined(macintosh) || defined(Macintosh) || (defined(__APPLE__) && defined(__MACH__))
#define PLATFORM_MAC
#elif defined(_WIN32) || defined(_WIN64) || defined(__WIN32__) || defined(__WINDOWS__)
#define PLATFORM_WINDOWS
#elif defined(__gnu_linux__) || defined(__linux__) || defined(__unix__)
#define PLATFORM_LINUX
#endif

#endif /* platform_h */

#pragma once

#ifdef PLANET_LIB_EXPORTS
#	define PLANET_LIB_API _declspec(dllexport)
#else
#	define PLANET_LIB_API _declspec(dllimport)
#endif


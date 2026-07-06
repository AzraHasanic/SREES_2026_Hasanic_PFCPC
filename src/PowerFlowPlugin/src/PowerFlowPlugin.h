#pragma once
#include <compiler/Definitions.h>
#include <sc/IPlugin.h>

#ifdef MU_WINDOWS
    #ifdef PLUGIN_EXPORTS
        #define PLUGIN_API __declspec(dllexport)
    #else
        #define PLUGIN_API __declspec(dllimport)
    #endif
#else
    #ifdef PLUGIN_EXPORTS
        #define PLUGIN_API __attribute__((visibility("default")))
    #else
        #define PLUGIN_API
    #endif
#endif

void onClosedPluginWindow();

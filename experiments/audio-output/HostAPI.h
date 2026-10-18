#pragma once
#include "tier1/interface.h"
#include "vgui/VGUI.h"
// Experimental host ABI. Not an exported retail GameUI interface.
typedef bool (*HostInitFn)(CreateInterfaceFn);
typedef vgui::VPANEL (*HostCreateFn)(vgui::VPANEL);
typedef bool (*HostBindFn)(vgui::VPANEL);
typedef void (*HostVoidFn)();
struct AudioHostAPI { HostInitFn Init; HostCreateFn Create; HostBindFn Bind; HostVoidFn Unbind; HostVoidFn Destroy; HostVoidFn Inspect; HostCreateFn RenderProbe; HostVoidFn AlphaOnce; void (*Update)(float); void (*SetPolish)(bool); };

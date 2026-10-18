# Architecture and source inventory

Client.cpp:
- Adds a client-owned Button inside a resource-created GameUI Panel mount. Bounded public VPANEL tree discovery runs every 0.5 seconds and verifies expected Audio controls. Own action target invokes OpenAudioHost directly. No console required.
- Does not modify GameUI-owned C++ objects or cast their pointers. The resource is a mount, not a resource-only command-routing solution. Original captions and shared buttons unchanged.
- Owns ListPanel, hidden ComboBox adapter, bilingual text, selection helper and independent MMDevice watcher. Full IDs remain internal. Refresh retains staged ID and original helper transaction baseline, including unavailable selections; Apply revalidates.
- Polls separate UI notification instance on game thread, coalescing events for 350ms. Does not consume recovery watcher's events or write audio preferences on refresh.
- Loads host from the mod game directory; owns receiver destruction; updates host animation controller with engine->Time() in a game-system update.
Host.cpp:
- Owns actual public PropertyDialog, PropertySheet, PropertyPage and buttons. A plain host-owned Panel mount prevents public PropertySheet hotkey traversal from dereferencing foreign Panel pointers.
- Own page layout avoids EditablePanel legacy pin-offset auto-positioning. Native skin and animation remain.
- Sends synchronous public IPanel::RequestInfo messages named AudioHostTransactionV1: reset=1, apply=2, discard=3. Receiver returns explicit result. Host gates native Apply/OK before base handling; rejection keeps dialog open. Cancel/unbind explicitly discard, not inferred from destruction.
HostAPI.h:
- Experimental function table from CreateInterface AudioTransactionHost001; own modules only. Init/Create/Bind/Unbind/Destroy/Inspect/RenderProbe/AlphaOnce/Update/SetPolish. Not an interface exported by retail GameUI. Table has no size negotiation; both modules must match.
- Receiver unbound before deletion, then host destroyed. Host module pinned until process exit; physical unload is not validated.

Inherited dependencies (unchanged):
src/common/GameUI/AudioOutputSelection.{h,cpp}
src/common/GameUI/AudioOutputDevicesWin.{h,cpp}
src/common/GameUI/AudioOutputWatchWin.{h,cpp}
plus production recovery state machine/client game-system and VPC entries from the two parent commits. See git diff --name-only be57e3d..7478290 for the complete inherited contribution list.
Public SDK vgui_controls, tier libraries, interface factories, cvar/engine interfaces; Windows MMDevice/COM; existing SDK third-party libraries.

Selection invokes existing sound_device_override SetValue through the helper, retaining engine callback and NOT_CONNECTED checks. Explicit changes need no redundant snd_restart. Original recovery independently uses the validated restart path on availability transitions. System Default clears override but may require audio restart; no live-default-follow guarantee. Startup sounds may stop on engine reinitialization.

The installed standard GameUI Audio page hosts the entry, while the new dialog owns its own transactions. This does NOT add our receiver to the original options sheet's Apply transaction. This architecture was validated in the SDK mod with installed GameUI, not as an official retail TF2 client replacement.

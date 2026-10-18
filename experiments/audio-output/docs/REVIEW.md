# Validation, provenance and submission assessment

User-confirmed Windows x64 real hardware:
- Original Audio-tab button opens dialog without console; physical output switching; native keyboard/volume settings still work.
- Apply, OK, Cancel, Reset, rejection/result=6 and retry, Apply/change/Cancel, rebind, explicit destruction, shutdown with staged choice.
- Manual refresh and automatic headphone appearance on hotplug; stable reopening/console/lifecycle; final layout and Polish/English paths tested.
- Original production recovery separately passed in-server recovery, five disconnect/reconnect cycles, no repeat loop and normal shutdown. This does not make the new UI a separate exhaustive recovery retest.

Automated/tool verification:
- Experimental host/client compilation and linking.
- New parameterized packaging recipe successfully compiled/linked using existing SDK objects.
- Frozen source copies hash-equal to validated sources before commits; manifests contain no local paths.
- No new automated transaction assertion count claimed. Hardware reports are distinct from code review and compile checks.
Untested: clean checkout package build, Linux, other architectures, Counter-Strike: Source, official retail client integration, physical DLL unload, all future GameUI/resource versions.

Known limitations:
- Earlier unexplained intermittent termination in recovery development remains documented, despite later successful cycles. Separate host crash was diagnosed in public PropertySheet cross-module hotkey traversal and corrected with a same-module mount; repeated user tests passed afterward.
- GetUILanguage sampled on initialization; no dynamic language switch. Hardcoded Polish/English strings, English fallback, fixed Audio tab label; developer diagnostics English.
- Duplicate friendly names can look identical though IDs remain exact.
- DLL pinned, diagnostics/test commands retained, ABI table lacks negotiation, periodic discovery relies on known Audio control names/resource mount.
- Entry discovery and separate dialog are validated public-interface composition, not a stable Valve-supported extension registration contract.
- Helper uses hidden ComboBox adapter for ListPanel; review readability/maintenance before upstream.
- System Default limitations and interrupted startup sounds remain; no extra restart introduced by UI.
- No changes made to production code, existing PR branch or published PR.

Provenance:
This package's three implementation files were written for the public SDK experiment. It links public SDK VGUI implementations; it does not copy the separate Alien Swarm options-page reference files, and has no Swarm include/link dependency. Earlier Swarm source was consulted for lifecycle comparison and retained separately; no claim is made that its license allows redistribution.
The full installed Valve OptionsSubAudio.res is intentionally excluded. Only our mount fragment and local assembler are included. Valve can apply it to its own resource; standalone redistribution of extracted content is not assumed authorized.
SDK code and bundled libraries remain subject to repository LICENSE and thirdpartylegalnotices.txt; not all engine/runtime dependencies are open-source. Windows SDK/Steam components are prerequisites, not repackaged.
Official sources:
https://github.com/ValveSoftware/source-sdk-2013/blob/master/LICENSE
https://github.com/ValveSoftware/source-sdk-2013/blob/master/CONTRIBUTING
CONTRIBUTING requires ownership/authorization and identifying third-party material. The original integration is distinguished from Valve dependencies; no blanket licensing clearance is asserted. Keep upstream notices; do not represent SDK library implementations as original contribution.

Recommendation:
Propose the minimal SDK/GameUI extension contract first, with this runnable reference as evidence, in a separate discussion/proposal after authorization. Preserve PR #2158. If Valve accepts standalone-window architecture, prepare a separate dependent PR with host VPC project and resource integration; do not silently expand #2158.
Official integration needs Valve decisions on module ownership/loading, resource mount/button, localization resources, stable ABI/lifecycle contract, packaging and shared engine/SoundSystem reuse. Public sources suffice to build this prototype with SDK-provided binary dependencies; they do not provide a self-contained rebuild of private GameUI/engine, or automatic official TF2 integration.
Submission blockers: clean build-system integration and clean-checkout run; dependency/resource provenance boundary accepted; debug/test surface cleanup and stable ABI policy; Valve architectural agreement. Functional runtime success is established, upstream readiness is not.

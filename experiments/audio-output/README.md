# Audio options integration reference

Frozen Windows x64 SDK-mod implementation. Read docs/ARCHITECTURE.md, docs/BUILD.md and docs/REVIEW.md before building or proposing upstream integration.

Source files: Client.cpp, Host.cpp, HostAPI.h. Resource addition: resource/AudioOutputMount.res.fragment; resource/Add-Mount.ps1 assembles a locally supplied Valve Audio resource. Build.ps1 and four response templates in build/ reproduce the experimental compile/link recipe. Source hashes are in docs/validated-source-hashes.json.

This branch depends on the unchanged audio helper/provider/recovery contributions through 7478290ca1fb1ef355ecbc09efec6501417ffb11 (preceded by 593ff1d7014a957aa0ff58d02eca7531efc108d1). Upstream base of that chain is be57e3d01ac7d7aa6a2cacbfef187c0b46616db0; this package is NOT claimed rebased onto current master.

No installed engine/GameUI binaries, generated files, crash dumps or hardware logs are included. Diagnostic and rejection-test commands remain in frozen source deliberately; removal is an explicit pre-submission cleanup task, not silently performed on hardware-validated code.

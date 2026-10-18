# Audio options integration reference

Frozen Windows x64 SDK-mod implementation. Read [architecture](docs/ARCHITECTURE.md), [build instructions](docs/BUILD.md) and [validation/limitations](docs/REVIEW.md) before building or proposing upstream integration.

Source files: Client.cpp, Host.cpp, HostAPI.h. Resource addition: resource/AudioOutputMount.res.fragment; resource/Add-Mount.ps1 assembles a locally supplied Valve Audio resource. Build.ps1 and four response templates in build/ reproduce the experimental compile/link recipe. Source hashes are in docs/validated-source-hashes.json.

This branch depends on the unchanged audio helper/provider/recovery contributions through 7478290ca1fb1ef355ecbc09efec6501417ffb11 (preceded by 593ff1d7014a957aa0ff58d02eca7531efc108d1). Upstream base of that chain is be57e3d01ac7d7aa6a2cacbfef187c0b46616db0; this package is NOT claimed rebased onto current master.

No installed engine/GameUI binaries, generated files, crash dumps or hardware logs are included. Diagnostic and rejection-test commands remain in frozen source deliberately; removal is an explicit pre-submission cleanup task, not silently performed on hardware-validated code.

Clean Windows x64 Release reproduction passed for the SDK libraries, base client/server and integration modules. Fresh mount-only resource runtime tests also passed, but one unexplained headphone-removal termination remains disclosed; later successful cycles do not establish its cause or a fix.

The parent commits above are already included in this branch (no separate cherry-pick required) and come from [PR #2158](https://github.com/ValveSoftware/source-sdk-2013/pull/2158). This reference does not modify that PR. It is an isolated SDK-mod prototype, not an upstream-ready replacement for private GameUI or a validated official retail TF2 integration.

Repository [LICENSE](../../LICENSE), [third-party notices](../../thirdpartylegalnotices.txt) and [CONTRIBUTING](../../CONTRIBUTING) remain applicable. The new integration contains no copied Alien Swarm implementation or extracted full GameUI resource.

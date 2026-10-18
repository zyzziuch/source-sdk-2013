# Build and reproduction

Prerequisites: Windows x64, Visual Studio 2022 C++ toolset/Windows SDK, Source SDK public source and bundled libraries, Steam-installed Source SDK Base 2013 Multiplayer and TF2 for runtime content. No Alien Swarm source is needed.

1. Check out this branch including inherited audio commits.
2. From src in a normal developer command prompt:
   devtools\bin\vpc.exe /hl2mp /tf /define:SOURCESDK +everything /mksln everything.sln

   The generated solution target spelling must be checked against that VPC output; building src/game/client/client_win64_tf.vcxproj directly with Release/x64 avoids solution-name variation.
   msbuild game\client\client_win64_tf.vcxproj /p:Configuration=Release /p:Platform=x64 /m
   Build matching server_win64_tf.vcxproj under game/server similarly.
3. Generated protobuf includes normally live under src/game/client/generated_proto_tf (client_base.vpc). Use the directory containing tf_gcmessages.pb.h and related generated headers.
4. In an x64 VS developer PowerShell, from repository root:
   & .\experiments\audio-output\build\Build.ps1 -SdkRoot (Get-Location).Path -GeneratedHeaders .\src\game\client\generated_proto_tf -OutputDirectory .\experiments\audio-output\out

Recipe reuses base Release_mod_tf object files, adds Client.obj and links the full client. Host links Host.obj, base memoverride.obj/vgui_controls.obj, public vgui_controls and SDK libraries. Link templates explicitly list objects from the frozen SDK revision; they are source build recipes, not generated files committed from an arbitrary machine. Paths are parameterized. Do NOT use them for another revision/configuration without checking objects and CRT compatibility.

Validation: this parameterized script compiled and linked both modules successfully using the existing fully built SDK object's directory and generated headers. A brand-new clean checkout regeneration/build has NOT been performed for this package. This is a remaining reproducibility gate. Output files were not redeployed; hardware-tested binaries remain unchanged. Compiler legacy-header encoding warnings and missing library PDB warnings remain.

Runtime layout in a separate mod:
- gameinfo.txt: use SDK game/mod_tf template, point content mounts to installed SDK Base/TF2 and put the isolated mod first in game+vgui search paths.
- bin/x64/client.dll (new), audio_transaction_host.dll (new), server.dll (matching Release SDK).
- resource/OptionsSubAudio.res: obtain your installed matching Valve resource locally; run resource/Add-Mount.ps1 -OriginalResource <local-original> -Destination <isolated-mod>/resource/OptionsSubAudio.res. Script inserts only our Panel mount before the root closing brace. Review the result; no original controls should change.
- Original SDK mod resources/localization assets from parent audio contribution must remain accessible.
- PDBs optional for debugging, never source-controlled.
From the SDK Base installation directory, use PowerShell:
  & .\hl2_win64.exe -game $isolatedMod -dev -console -condebug -windowed -w 1280 -h 720 -insecure
Set $isolatedMod to the absolute path of your isolated mod. Keep Steam running. Never replace retail DLLs or GameUI.
The local resource assembler is a reproduction aid, not a general KeyValues parser or updater for arbitrary future resource revisions. Tested mount geometry assumes the matching original Audio layout.

Needed for normal upstream packaging: dedicated host VPC project, supported client registration/loading, resource integration owned by Valve, and clean-build validation. This package is a review reference, not a finished SDK build-system integration.

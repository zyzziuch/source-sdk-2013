param(
 [Parameter(Mandatory=$true)][string]$SdkRoot,
 [Parameter(Mandatory=$true)][string]$GeneratedHeaders,
 [Parameter(Mandatory=$true)][string]$OutputDirectory
)
# Run in VS 2022 x64 Native Tools PowerShell. No runtime deployment.
$ErrorActionPreference='Stop'
$package=Split-Path $PSScriptRoot -Parent
$sdk=(Resolve-Path $SdkRoot).Path
$generated=(Resolve-Path $GeneratedHeaders).Path
New-Item -ItemType Directory $OutputDirectory -Force | Out-Null
$out=(Resolve-Path $OutputDirectory).Path
foreach($name in @('compile-host','compile-client','link-host','link-client')){
 $text=Get-Content (Join-Path $PSScriptRoot "$name.rsp.in") -Raw
 $text=$text.Replace('@SDK@',$sdk).Replace('@PACKAGE@',$package).Replace('@GENERATED@',$generated).Replace('@OUT@',$out)
 $rsp=Join-Path $out "$name.rsp"
 Set-Content $rsp $text
 Push-Location (Join-Path $sdk 'src\game\client')
 try {
  if($name.StartsWith('compile')){& cl.exe "@$rsp"}else{& link.exe "@$rsp"}
  if($LASTEXITCODE -ne 0){throw "$name failed: $LASTEXITCODE"}
 }finally{Pop-Location}
}

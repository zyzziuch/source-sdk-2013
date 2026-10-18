param([Parameter(Mandatory=$true)][string]$OriginalResource,
      [Parameter(Mandatory=$true)][string]$Destination)
$ErrorActionPreference='Stop'
$text=Get-Content -LiteralPath $OriginalResource -Raw
if($text.Contains('SDKAudioOutputMount')){throw 'Already patched'}
$last=$text.LastIndexOf('}')
if($last -lt 0){throw 'Missing root closing brace'}
$fragment=Get-Content (Join-Path $PSScriptRoot 'AudioOutputMount.res.fragment') -Raw
Set-Content -LiteralPath $Destination ($text.Substring(0,$last)+$fragment+[Environment]::NewLine+$text.Substring($last))

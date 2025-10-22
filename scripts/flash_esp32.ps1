
param(
  [string]$Port = ""
)
if (-not (Get-Command py -ErrorAction SilentlyContinue)) {
  Write-Host "Python launcher 'py' not found. Install Python 3 first." -ForegroundColor Yellow
  exit 1
}
py -m pip install esptool | Out-Null
$cmd = @("py","-m","esptool","--chip","esp32","--baud","921600","--before","default_reset","--after","hard_reset","write_flash","-z")
if ($Port -ne "") { $cmd += @("--port",$Port) }
$cmd += @(
  "0x1000","../firmware/bootloader.bin",
  "0x8000","../firmware/partitions.bin",
  "0xE000","../firmware/boot_app0.bin",
  "0x10000","../firmware/app.bin"
)
& $cmd
Write-Host "Done."

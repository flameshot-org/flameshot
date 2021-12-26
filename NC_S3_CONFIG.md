# S3 bucket configuration

_Note: Some aspects in this section is Namecheap related only._

S3 bucket credentials are stored on the server which is available in the VPN network only.
Path to this remote file is stored in the local file `config.ini` and cannot be configured with UI.
This file is not included into installation and should be placed manually for non-default teams,
location for the `config.ini` file depends on OS:

- Linux:
    - `/etc/flameshot/config.ini`
    - `/home/<user>/.config/flameshot/config.ini`
- MacOS:
    - `/Users/<user>/.config/flameshot/config.ini`
    - `/Users/<user>/Library/Preferences/config.ini`
- Windows:
    - At the same folder where `flameshot.exe` is placed

Local configuration file `config.ini` example:
```
[General]
; Path with s3 creds for upload and other settings
STORAGE_CONFIG_URL="https://git.examle.com/projects/repos/flameshot_config/raw/config.ini"
```
Where:

- `STORAGE_CONFIG_URL` - url with the remote configuration file

_Note: local configuration file is optional and default value is set to the non-CS Namecheap teams._

Remote (external) configuration file example:
```
; Namecheap CS Team configuration file
[General]
;HTTP_PROXY_HOST=10.0.0.1
;HTTP_PROXY_PORT=3128

; No authentication USER and PASSWORD should be empty
;HTTP_PROXY_USER=
;HTTP_PROXY_PASSWORD=

; Proxy Types (3 is default):
; 0	Proxy is determined based on the application proxy set using setApplicationProxy()
; 1	Socks5 proxying is used
; 3	HTTP transparent proxying is used
; 4	Proxying for HTTP requests only
; 5	Proxying for FTP requests only
;HTTP_PROXY_TYPE=3

; fix settings for the end user (enterprise needs)
checkForUpdates=false

[S3]
S3_URL=https://img.example.com/
S3_CREDS_URL=https://api.img.example.com/
S3_X_API_KEY=<your creds>
```
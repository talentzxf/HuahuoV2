Works on any Windows platform as long as ENABLE_ETW is defined

Workflow:
- Build Unity editor
- Run "build\WindowsEditor\Data\UnityETWProvider\InstallProvider.ps1" as Admin to register Unity profile on your machine
- Use Unity*.wprp profiles to record data you want
- Run Editor or player on local machine or a device
- Examine traces. 

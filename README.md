## Capletomer
Capletomer ~~(remoteLPAC)~~ is a http api for https://github.com/estkme-group/lpac  
Managed by ChatGPT.   
~~Multi devices (may, perhaps, possibly, or probably) be supported in the far future~~  

### Usage  
You need to compile LPAC first (https://github.com/estkme-group/lpac/blob/main/docs/DEVELOPERS.md). 
For Wifi dongles with Qualcomm 410 built in, perhaps the legacy version of README of https://github.com/damonto/telegram-sms may help.  
  
Currently this project is extremely unstable and is limited to simulation under Windows.

GET /execute/aaa/bbb will run $lpac$ aaa bbb. Example: GET /execute/profile/list will return the output of lpac profile list .  
GET /getprofile returns profiles;  
GET /chipinfo returns chipinfo;  
GET /enableprofile/{iccid} enables {iccid}'s profile;  
GET /disableprofile/{iccid} disables ^^^^^^;  
POST /nickname/{iccid} body:{base64-encoded-nickname} changes the profile's nickname  

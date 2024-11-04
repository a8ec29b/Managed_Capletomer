## Capletomer
Capletomer ~~(remoteLPAC)~~ is a http api for https://github.com/estkme-group/lpac  
Managed by ChatGPT.   
~~Multi devices (may, perhaps, possibly, or probably) be supported in the far future~~  
**Currently this project is extremely unstable.**  

### Usage  
You need to compile LPAC first (https://github.com/estkme-group/lpac/blob/main/docs/DEVELOPERS.md). 
For Wifi dongles with Qualcomm 410 built in, perhaps the legacy version of README of https://github.com/damonto/telegram-sms may help.  

First you need a config.txt(just modify according to the config.txt in repo). To specify a config, run $(compiled application) -c /path/to/the/specific/config.txt$. If you are using Qualcomm 410 wifi dongle with debian installed, you may enable 410WifiDongle in the config.json.  

GET /execute/aaa/bbb will run $lpac aaa bbb$. Example: GET /execute/profile/list will return the output of $lpac profile list$. Need to enable execute in the config.txt first.  
GET /getprofile returns profiles;  
GET /chipinfo returns chipinfo;  
GET /enableprofile/{iccid} enables {iccid}'s profile;  
GET /disableprofile/{iccid} disables ^^^^^^;  
POST /nickname/{iccid} body:{base64-encoded-nickname} changes the profile's nickname  
POST /download/ body:{base64-encodd-LPA_AC} download a new profile. **Passed test only on Linux. Confirmation code currently not supported.**. 
GET /delete/{iccid} delete the profile.  
GET /getnotification gets the notification.  
GET /processnotification/{sequenceID} processes the sequence id of the notification.  
GET /removenotification/{sequenceID} removes the sequence id of the notification.  

1.D05平台进入shell环境。

2.D05自带一个存储空间fs0,如下：
FS0:\> map -r                                                                   
Mapping table                                                                   
      FS0: Alias(s):F0:;BLK1:                                                   
          VenMsg(06ED4DD0-FF78-11D3-BDC4-00A0C94053D1,0000000000000000)         
     BLK0: Alias(s):                                                            
          VenHw(CC2CBF29-1498-4CDD-8171-F8B6B41D0909)                           
FS0:\>   

3.在shell下使用ifconfig命令配置网络，如下：
FS0:\> ifconfig -s eth2 static 192.168.2.102 255.255.255.0 192.168.2.1

4.检查是否ping通我的工作机，如下：
FS0:\> ping 192.168.2.112

5.使用TFTP工具，搭建一个TFTP的server，我个人用的是工具：tftpd32

6.在Shell下使用tftp命令从server上获取bios的fd文件和falsh工具，如下：
FS0:\> tftp 192.168.2.112 myflashtool.efi flashtool.efi 
FS0:\> tftp 192.168.2.112 D05.fd bios.fd 
FS0:\> ls                                                                       
Directory of: FS0:\                                                             
08/03/2017  04:09              65,536  flashtool.efi                            
08/03/2017  04:10           3,145,728  bios.fd                                  
          2 File(s)   3,211,264 bytes                                           
          0 Dir(s) 

7.运行flashtool.efi工具，帮助如下：
FS0:\> flashtool.efi
FlashTool:  usage                                                               
  FlashTool -f <BiosFile.fd>                                                    
  FlashTool -m <BiosFile.fd>                                                    
Parameter:                                                                      
  -f:  Update all BIOS Rom.                                                     
  -m:  Update all BIOS Rom except NVRAM.

如果刷整个BIOS Rom，命令如下：
FS0:\> flashtool.efi -f bios.fd
Bios File Name: bios.fd
Bios Size: 0x300000
Erase Flash Start                                                               
Erase Flash Status: Success                                                     
Write Flash Start                                                               
Write Flash Status: Success                                                     
Update BIOS success! Please reboot your system!

如果刷整个BIOS Rom，但是不想刷NVRAM，则命令如下：
FS0:\> flashtool.efi -m bios.fd
1.D05ƽ̨����shell������

2.D05�Դ�һ���洢�ռ�fs0,���£�
FS0:\> map -r                                                                   
Mapping table                                                                   
      FS0: Alias(s):F0:;BLK1:                                                   
          VenMsg(06ED4DD0-FF78-11D3-BDC4-00A0C94053D1,0000000000000000)         
     BLK0: Alias(s):                                                            
          VenHw(CC2CBF29-1498-4CDD-8171-F8B6B41D0909)                           
FS0:\>   

3.��shell��ʹ��ifconfig�����������磬���£�
FS0:\> ifconfig -s eth2 static 192.168.2.102 255.255.255.0 192.168.2.1

4.����Ƿ�pingͨ�ҵĹ����������£�
FS0:\> ping 192.168.2.112

5.ʹ��TFTP���ߣ��һ��TFTP��server���Ҹ����õ��ǹ��ߣ�tftpd32

6.��Shell��ʹ��tftp�����server�ϻ�ȡbios��fd�ļ���falsh���ߣ����£�
FS0:\> tftp 192.168.2.112 myflashtool.efi flashtool.efi 
FS0:\> tftp 192.168.2.112 D05.fd bios.fd 
FS0:\> ls                                                                       
Directory of: FS0:\                                                             
08/03/2017  04:09              65,536  flashtool.efi                            
08/03/2017  04:10           3,145,728  bios.fd                                  
          2 File(s)   3,211,264 bytes                                           
          0 Dir(s) 

7.����flashtool.efi���ߣ��������£�
FS0:\> flashtool.efi
FlashTool:  usage                                                               
  FlashTool -f <BiosFile.fd>                                                    
  FlashTool -m <BiosFile.fd>                                                    
Parameter:                                                                      
  -f:  Update all BIOS Rom.                                                     
  -m:  Update all BIOS Rom except NVRAM.

���ˢ����BIOS Rom���������£�
FS0:\> flashtool.efi -f bios.fd
Bios File Name: bios.fd
Bios Size: 0x300000
Erase Flash Start                                                               
Erase Flash Status: Success                                                     
Write Flash Start                                                               
Write Flash Status: Success                                                     
Update BIOS success! Please reboot your system!

���ˢ����BIOS Rom�����ǲ���ˢNVRAM�����������£�
FS0:\> flashtool.efi -m bios.fd
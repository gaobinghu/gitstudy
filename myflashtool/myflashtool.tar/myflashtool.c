
#include <Library/BaseLib.h>
#include <Library/DebugLib.h>
#include <Library/UefiLib.h>
#include <Library/PrintLib.h>
#include <Library/UefiDriverEntryPoint.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiRuntimeServicesTableLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/MemoryAllocationLib.h>
//#include <Library/DevicePathLib.h>
//#include <Library/DxeServicesTableLib.h>
//#include <Library/HiiLib.h>
//#include <Library/HobLib.h>
//#include <IndustryStandard/Pci22.h>
#include <Library/PcdLib.h>

#include <Protocol/SimpleFileSystem.h>
#include <Protocol/DevicePath.h>
#include <Library/DevicePathLib.h>
#include <Guid/FileInfo.h>
#include <Protocol/HisiSpiFlashProtocol.h>
#include <Library/ShellLib.h>

//#include <SetupVariableSample.h>
//#include <Guid/ByoSetupFormsetGuid.h>
//#include <Protocol/LogSetupVariableProtocol.h>
//#include <Protocol/ByoFormSetManager.h>
//#include "FormsetConfiguration.h"
//#include "SetupFormInit.h"
//kavi- #define BIOS_FILE_NAME   L"\\D05.fd" 

SHELL_PARAM_ITEM    mflashbiosCheckList[] = {
  {
    L"-f",
    TypeValue
  },
  {
    L"-m",
    TypeValue
  },
  {
    NULL,
    TypeMax
  },
};

VOID
PrintUsage (
  VOID
  )
{
  Print(L"FlashTool:  usage\n");
  Print(L"  FlashTool -f <BiosFile.fd>\n");
  Print(L"  FlashTool -m <BiosFile.fd>\n");
  Print(L"Parameter:\n");
  Print(L"  -f:  Update all BIOS Rom.\n");
  Print(L"  -m:  Update all BIOS Rom except NVRAM.\n");
}



EFI_STATUS
OpenFileFromDevicePath (
  IN  EFI_DEVICE_PATH_PROTOCOL   *DevicePath,
  IN  UINT64                     OpenMode,
  IN  UINT64                     Attributes,
  OUT EFI_FILE_HANDLE            *NewHandle
  )
{
  EFI_STATUS                        Status;
  EFI_HANDLE                        DeviceHandle;
  FILEPATH_DEVICE_PATH              *FilePathNode;
  EFI_SIMPLE_FILE_SYSTEM_PROTOCOL   *Volume;
  EFI_FILE_HANDLE                   FileHandle;
  EFI_FILE_HANDLE                   LastHandle;
  FILEPATH_DEVICE_PATH              *OriginalFilePathNode;

  FilePathNode = (FILEPATH_DEVICE_PATH *) DevicePath;
  Volume       = NULL;
  *NewHandle   = NULL;

  Status = gBS->LocateDevicePath (
                  &gEfiSimpleFileSystemProtocolGuid,
                  (EFI_DEVICE_PATH_PROTOCOL **) &FilePathNode,
                  &DeviceHandle
                  );
  if (!EFI_ERROR (Status)) {
    Status = gBS->HandleProtocol (DeviceHandle, &gEfiSimpleFileSystemProtocolGuid, (VOID **) &Volume);
  }

  if (EFI_ERROR (Status) || Volume == NULL) {
    return EFI_NOT_FOUND;
  }

  Status = Volume->OpenVolume (Volume, &FileHandle);
  if (!EFI_ERROR (Status)) {
    //
    // Duplicate the device path to avoid the access to unaligned device path node.
    // Because the device path consists of one or more FILE PATH MEDIA DEVICE PATH
    // nodes, It assures the fields in device path nodes are 2 byte aligned.
    //
    FilePathNode = (FILEPATH_DEVICE_PATH *) DuplicateDevicePath (
                                              (EFI_DEVICE_PATH_PROTOCOL *)(UINTN)FilePathNode
                                              );
    if (FilePathNode == NULL) {
      FileHandle->Close (FileHandle);
      Status = EFI_OUT_OF_RESOURCES;
    } else {
      OriginalFilePathNode = FilePathNode;
      //
      // Parse each MEDIA_FILEPATH_DP node. There may be more than one, since the
      //  directory information and filename can be seperate. The goal is to inch
      //  our way down each device path node and close the previous node
      //
      while (!IsDevicePathEnd (&FilePathNode->Header)) {
        if (DevicePathType (&FilePathNode->Header) != MEDIA_DEVICE_PATH ||
            DevicePathSubType (&FilePathNode->Header) != MEDIA_FILEPATH_DP) {
          Status = EFI_UNSUPPORTED;
        }

        if (EFI_ERROR (Status)) {
          //
          // Exit loop on Error
          //
          break;
        }

        LastHandle = FileHandle;
        FileHandle = NULL;
        Status = LastHandle->Open (
                               LastHandle,
                               &FileHandle,
                               FilePathNode->PathName,
                               OpenMode,
                               Attributes
                               );

        //
        // Close the previous node
        //
        LastHandle->Close (LastHandle);
        FilePathNode = (FILEPATH_DEVICE_PATH *) NextDevicePathNode (&FilePathNode->Header);
      }
      //
      // Free the allocated memory pool
      //
      gBS->FreePool (OriginalFilePathNode);
    }
  }

  if (Status == EFI_SUCCESS) {
    *NewHandle = FileHandle;
  }

  return Status;
}


EFI_STATUS
EFIAPI
UpdateBiosRom (
    IN UINT8               *BiosData,
    IN UINT32              BiosSize,
    IN BOOLEAN             updateNV
  )
{
	HISI_SPI_FLASH_PROTOCOL         *mFlash;
	UINTN					        RomAddress;
    EFI_STATUS                      Status;
    UINT8                           *NVbackup;
	UINT32					        NVoffset;
	UINT32                          NVsize;
	
    Status = gBS->LocateProtocol (&gHisiSpiFlashProtocolGuid, NULL, (VOID*) &mFlash);

    DEBUG ((DEBUG_ERROR, "kavi updateNV = 0x%x \n", updateNV)); 

    if(updateNV == FALSE){
      NVoffset =(UINT32) (PcdGet32 (PcdFlashNvStorageVariableBase) - (UINT32)PcdGet64(PcdFdBaseAddress));
      NVsize = PcdGet32 (PcdFlashNvStorageVariableSize);
	  DEBUG ((DEBUG_ERROR, "kavi NVoffset = 0x%x, NVsize = 0x%x\n", NVoffset, NVsize)); 
	  NVbackup = AllocateZeroPool (NVsize);
	  Status = mFlash->Read(mFlash, NVoffset, NVbackup, NVsize);
      DEBUG ((DEBUG_ERROR, "kavi Read nvram Status : %r\n", Status));
	  if(Status != 0){	  	
        return Status;
	  }
    }

    RomAddress = SIZE_8MB;

	Print(L"Erase Flash Start\n");
    Status = mFlash->Erase(mFlash, (UINT32)RomAddress, BiosSize);
	//DEBUG ((DEBUG_ERROR, "kavi Erase Status : %r\n", Status));
	Print(L"Erase Flash Status: %r \n", Status);
	
    if(Status != 0) return Status;

	Print(L"Write Flash Start\n");
    Status = mFlash->Write(mFlash, (UINT32)RomAddress, (UINT8*)BiosData, BiosSize);
    Print(L"Write Flash Status: %r \n", Status);
    if(Status != 0) return Status;


    if(updateNV == FALSE){
	  Status = mFlash->Write(mFlash, (UINT32)NVoffset, (UINT8*)NVbackup, NVsize);
      DEBUG ((DEBUG_ERROR, "kavi write nvram Status : %r\n", Status));
	  if(NULL != NVbackup){
        FreePool (NVbackup);
      }
	  if(Status != 0){	  	
        return Status;
	  }
    }


    return Status;
}


EFI_STATUS
EFIAPI
ReadBIOSFile (
    IN  EFI_HANDLE          FileSystemHandle,
    IN  CHAR16				*BiosFileName,
    OUT UINT8               **BiosData,
    OUT UINT32              *BiosSize
  )
{
    EFI_STATUS                  Status;
    EFI_DEVICE_PATH_PROTOCOL    *StatusFilePath = NULL;
    EFI_FILE_HANDLE             BIOSFileHandle;
    UINTN                       FileBufferSize;
    EFI_FILE_INFO               *Info = NULL;
    UINT8                       *FileData = NULL;
    UINT32                      k = 0;//kavi+

    StatusFilePath = FileDevicePath (FileSystemHandle, BiosFileName);
    if(NULL == StatusFilePath)
    {
        return EFI_NOT_FOUND;
    }
    
    Status = OpenFileFromDevicePath (
                                    StatusFilePath,
                                    EFI_FILE_MODE_READ,
                                    0,
                                    &BIOSFileHandle
                                    );
    if ((Status == EFI_SUCCESS) && (BIOSFileHandle != NULL)) 
    {
        //
        // Get file size
        //
        FileBufferSize  = SIZE_OF_EFI_FILE_INFO + 200;
        do {
            Info = NULL;
            Status = gBS->AllocatePool (EfiBootServicesData, FileBufferSize, (VOID **) &Info);
            if (EFI_ERROR (Status)) 
            {
                break;
            }
            Status = BIOSFileHandle->GetInfo (
                                                BIOSFileHandle,
                                                &gEfiFileInfoGuid,
                                                &FileBufferSize,
                                                Info
                                                );
            if (!EFI_ERROR (Status)) 
            {
                break;
            }
            if (Status != EFI_BUFFER_TOO_SMALL) 
            {
                FreePool (Info);
                continue;
            }
            FreePool (Info);
        } while (TRUE);

        if(NULL == Info)
        {
            return EFI_NOT_FOUND;
        }


        FileBufferSize =  Info->FileSize;
		DEBUG ((DEBUG_ERROR, "kavi file size = 0x%x \n", FileBufferSize));
		
//kavi-        FileData = AllocateZeroPool (FileBufferSize);
		*BiosData = AllocateZeroPool (FileBufferSize);
        FileData = *BiosData; //kavi+
        if (NULL == FileData) 
        {
             return EFI_OUT_OF_RESOURCES;;
        }

        Status =BIOSFileHandle->Read (BIOSFileHandle, &FileBufferSize, FileData);
        if (EFI_ERROR (Status)) 
        {
             return Status;;
        }
        Status = BIOSFileHandle->Close (BIOSFileHandle);

        DEBUG ((DEBUG_ERROR, "kavi the first 160 : \n"));
		for(k = 0; k < 160; k++){
		  if(k%16 == 0)	DEBUG ((DEBUG_ERROR, "\n"));
          DEBUG ((DEBUG_ERROR, "0x%x ", FileData[k]));
		}
        

        DEBUG ((DEBUG_ERROR, "\n kavi the last 160 : \n"));
		for(k = FileBufferSize - 160; k < FileBufferSize; k++){
		  if(k%16 == 0)	DEBUG ((DEBUG_ERROR, "\n"));
          DEBUG ((DEBUG_ERROR, "0x%x ", FileData[k]));
		}

        if(NULL != Info)
        {
            FreePool (Info);
        }

        //if(NULL != FileData)
        //{
        //    FreePool (FileData);
        //}
    }
	
	if(NULL != FileData){
	//  BiosData = FileData;
	  *BiosSize = FileBufferSize;
	}
	
    DEBUG ((DEBUG_ERROR, "Read BIOS file return Status = %r \n", Status));
    return Status;
}


EFI_STATUS
EFIAPI
flashtoolentry (
 IN EFI_HANDLE        ImageHandle,
 IN EFI_SYSTEM_TABLE  *SystemTable
)
{

    UINTN                           Index;
    UINTN                           NumberFileSystemHandles;
    EFI_HANDLE                      *FileSystemHandles;
    EFI_STATUS                      Status;
    VOID                            *Interface;
    UINT8                           *BiosData = NULL;
    UINT32                          BiosSize = 0;
    UINT32                          k = 0;
	LIST_ENTRY					  *ParamPackage;
	CONST CHAR16				  *FileName;
	CHAR16				           *BiosFileName;
	CHAR16						  *ProblemParam;
    BOOLEAN                         updateNV = FALSE;
	
	Status = ShellCommandLineParseEx (mflashbiosCheckList, &ParamPackage, &ProblemParam, TRUE, FALSE);
	DEBUG ((DEBUG_ERROR, "kavi ShellCommandLineParseEx status = %r \n", Status));


//	PcdGet32 (PcdFlashNvStorageVariableBase) + PcdGet32 (PcdFlashNvStorageVariableSize)
	
	if (EFI_ERROR (Status)) {
	  
      return Status;
	}
	
	if (ShellCommandLineGetFlag (ParamPackage, L"-f")) {

	  FileName = ShellCommandLineGetValue (ParamPackage, L"-f");
	  if (FileName != NULL) {
	    BiosFileName             = AllocateCopyPool (StrSize (FileName), FileName);
		updateNV = TRUE;
        //DEBUG ((DEBUG_ERROR, "kavi BiosFileName = %s \n", BiosFileName));
		//Print(L"Bios File Name: %s\n", BiosFileName);
	  }else{
	    PrintUsage();
        return Status;
	  }
	}else if(ShellCommandLineGetFlag (ParamPackage, L"-m")){
	  FileName = ShellCommandLineGetValue (ParamPackage, L"-m");
	  if (FileName != NULL) {
	    BiosFileName             = AllocateCopyPool (StrSize (FileName), FileName);
        //DEBUG ((DEBUG_ERROR, "kavi BiosFileName = %s \n", BiosFileName));
		updateNV = FALSE;
	  }else{
	    PrintUsage();
        return Status;
	  }
	}else{
	  PrintUsage();
      return Status;
	}

	Print(L"Bios File Name: %s\n", BiosFileName);

    Status = gBS->LocateProtocol (&gEfiSimpleFileSystemProtocolGuid, NULL, (VOID **) &Interface);
    DEBUG ((DEBUG_ERROR, "kavi gEfiSimpleFileSystemProtocolGuid status = %r \n", Status));   
    if (EFI_ERROR(Status)) {
      return EFI_SUCCESS;
    }
				   
    gBS->LocateHandleBuffer (
							ByProtocol,
							&gEfiSimpleFileSystemProtocolGuid,
							NULL,
							&NumberFileSystemHandles,
							&FileSystemHandles
							);
	
    DEBUG ((DEBUG_ERROR, "NumberFileSystemHandles = 0x%x \n", NumberFileSystemHandles));
    for (Index = 0; Index < NumberFileSystemHandles; Index++){
	  Status = ReadBIOSFile(FileSystemHandles[Index], BiosFileName, &BiosData, &BiosSize);
      if(Status == 0) break;	  
    }

	if(Index == NumberFileSystemHandles){
	  //DEBUG ((DEBUG_ERROR, "kavi can not find the BIOS file: %s \n", BiosFileName));
	  Print(L"Can not find the Bios File: %s\n", BiosFileName);
      return Status;
	}
	//DEBUG ((DEBUG_ERROR, "kavi BiosSize = 0x%x \n", BiosSize));
	Print(L"Bios Size: 0x%x \n", BiosSize);

	DEBUG ((DEBUG_ERROR, "kavi the first 160 : \n"));
	for(k = 0; k < 160; k++){
	  if(k%16 == 0) DEBUG ((DEBUG_ERROR, "\n"));
	  DEBUG ((DEBUG_ERROR, "0x%x ", BiosData[k]));
	}
	
	
	DEBUG ((DEBUG_ERROR, "\n kavi the last 160 : \n"));
	for(k = BiosSize - 160; k < BiosSize; k++){
	  if(k%16 == 0) DEBUG ((DEBUG_ERROR, "\n"));
	  DEBUG ((DEBUG_ERROR, "0x%x ", BiosData[k]));
	}


    Status = UpdateBiosRom(BiosData, BiosSize, updateNV);
	
    if(Status == 0){
      Print(L"Update BIOS success! Please reboot your system!\n");
    }else{
      Print(L"Update BIOS Fail!\n");
	}
/*
	EFI_STATUS					  Status;
	LIST_ENTRY					  *ParamPackage;
	CONST CHAR16				  *ValueStr;
	CHAR16						  *ProblemParam;


	Status = ShellCommandLineParseEx (mflashbiosCheckList, &ParamPackage, &ProblemParam, TRUE, FALSE);
	DEBUG ((DEBUG_ERROR, "kavi ShellCommandLineParseEx status = %r \n", Status));

	
	if (EFI_ERROR (Status)) {
	  
      return Status;
	}
	
	if (ShellCommandLineGetFlag (ParamPackage, L"-f")) {

	  ValueStr = ShellCommandLineGetValue (ParamPackage, L"-f");
	  if (ValueStr != NULL) {
        DEBUG ((DEBUG_ERROR, "kavi ValueStr = %s \n", ValueStr));
	  }
	}
*/

    return Status;
}

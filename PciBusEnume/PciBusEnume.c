/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023. All rights reserved.
 * Description  :Component description file about IO Initialization.
 * Author     : gwx1264738
 * Create     : 2023/08/23
 */
#include "PciBusEnume.h"


UINT8      *mBusPool;
// BOOLEAN
// IsPciBridge (
//   IN  UINT8                               Bus,
//   IN  UINT8                               Device,
//   IN  UINT8                               Func
//   )
// {
//   UINT64      Address;
//   EFI_STATUS  Status;
//   UINT8       HeaderType = 0xFF;
//   //
//   // Create PCI address map in terms of Bus, Device and Func
//   //
//   Address = EFI_PCI_ADDRESS (Bus, Device, Func, 0x0E);

//   //
//   // Read the Vendor ID register
//   //
//   HeaderType = PciRead8 (Address);

// if ((HeaderType & HEADER_LAYOUT_CODE) == (HEADER_TYPE_PCI_TO_PCI_BRIDGE)) {
//     return TRUE;
// }
//   return FALSE;
// }

// EFI_STATUS
// PciDevicePresent (
//   IN  UINT8                               Bus,
//   IN  UINT8                               Device,
//   IN  UINT8                               Func
//   )
// {
//   UINT64      Address;
//   EFI_STATUS  Status;
//   UINT16       VendorId = 0xFFFF;
//   //
//   // Create PCI address map in terms of Bus, Device and Func
//   //
//   Address = EFI_PCI_ADDRESS (Bus, Device, Func, 0);

//   //
//   // Read the Vendor ID register
//   //
//   VendorId = PciRead16 (Address);
//   if (VendorId != 0xffff) {
//     return EFI_SUCCESS;
//   }

//   return EFI_NOT_FOUND;
// }

// VOID
// ResetAllPpbBusNumber (
//   IN UINT8                              StartBusNumber
//   )
// {
//   EFI_STATUS                      Status;
//   PCI_TYPE00                      Pci;
//   UINT8                           Device;
//   UINT32                          Register;
//   UINT8                           Func;
//   UINT64                          Address;
//   UINT8                           SecondaryBus;
//   EFI_PCI_ROOT_BRIDGE_IO_PROTOCOL *PciRootBridgeIo;



//   for (Device = 0; Device <= PCI_MAX_DEVICE; Device++) {
//     for (Func = 0; Func <= PCI_MAX_FUNC; Func++) {

//       //
//       // Check to see whether a pci device is present
//       //
//       Status = PciDevicePresent (StartBusNumber, Device, Func);
//       if (EFI_ERROR (Status) && Func == 0) {
//         //
//         // go to next device if there is no Function 0
//         //
//         break;
//       }

//       if (!EFI_ERROR (Status) && (IsPciBridge (StartBusNumber, Device, Func))) {

//         Register  = 0;
//         Address   = EFI_PCI_ADDRESS (StartBusNumber, Device, Func, 0x18);
//         Register = PciRead32 (Address);
//         SecondaryBus = (UINT8)(Register >> 8);

//         if (SecondaryBus != 0) {
//           ResetAllPpbBusNumber (SecondaryBus);
//         }

//         //
//         // Reset register 18h, 19h, 1Ah on PCI Bridge
//         //
//         Register &= 0xFF000000;
//         PciWrite32 (Address, Register);
//       }

//       if (Func == 0 && !IsPciMultiFunc (&Pci)) {
//         //
//         // Skip sub functions, this is not a multi function device
//         //
//         Func = PCI_MAX_FUNC;
//       }
//     }
//   }
// }

// EFI_STATUS
// PciScanBus (
//   IN PCI_IO_DEVICE                      *Bridge,
//   IN UINT8                              StartBusNumber,
//   OUT UINT8                             *SubBusNumber,
//   OUT UINT8                             *PaddedBusRange
//   )
// {
//   EFI_STATUS                        Status;
//   PCI_TYPE00                        Pci;
//   UINT8                             Device;
//   UINT8                             Func;
//   UINT64                            Address;
//   UINT8                             SecondBus;
//   UINT8                             PaddedSubBus;
//   UINT16                            Register;
//   UINTN                             HpIndex;
//   PCI_IO_DEVICE                     *PciDevice;
//   EFI_EVENT                         Event;
//   EFI_HPC_STATE                     State;
//   UINT64                            PciAddress;
//   EFI_HPC_PADDING_ATTRIBUTES        Attributes;
//   EFI_ACPI_ADDRESS_SPACE_DESCRIPTOR *Descriptors;
//   EFI_ACPI_ADDRESS_SPACE_DESCRIPTOR *NextDescriptors;
//   UINT16                            BusRange;
//   EFI_PCI_ROOT_BRIDGE_IO_PROTOCOL   *PciRootBridgeIo;
//   BOOLEAN                           BusPadding;
//   UINT32                            TempReservedBusNum;

//   PciRootBridgeIo = Bridge->PciRootBridgeIo;
//   SecondBus       = 0;
//   Register        = 0;
//   State           = 0;
//   Attributes      = (EFI_HPC_PADDING_ATTRIBUTES) 0;
//   BusRange        = 0;
//   BusPadding      = FALSE;
//   PciDevice       = NULL;
//   PciAddress      = 0;

//   for (Device = 0; Device <= PCI_MAX_DEVICE; Device++) {
//     TempReservedBusNum = 0;
//     for (Func = 0; Func <= PCI_MAX_FUNC; Func++) {

//       //
//       // Check to see whether a pci device is present
//       //
//       Status = PciDevicePresent (
//                 PciRootBridgeIo,
//                 &Pci,
//                 StartBusNumber,
//                 Device,
//                 Func
//                 );

//       if (EFI_ERROR (Status) && Func == 0) {
//         //
//         // go to next device if there is no Function 0
//         //
//         break;
//       }

//       if (EFI_ERROR (Status)) {
//         continue;
//       }

//       //
//       // Get the PCI device information
//       //
//       Status = PciSearchDevice (
//                 Bridge,
//                 &Pci,
//                 StartBusNumber,
//                 Device,
//                 Func,
//                 &PciDevice
//                 );

//       if (EFI_ERROR (Status)) {
//         continue;
//       }

//       PciAddress = EFI_PCI_ADDRESS (StartBusNumber, Device, Func, 0);

//       if (!IS_PCI_BRIDGE (&Pci)) {
//         //
//         // PCI bridges will be called later
//         // Here just need for PCI device or PCI to cardbus controller
//         // EfiPciBeforeChildBusEnumeration for PCI Device Node
//         //
//         PreprocessController (
//             PciDevice,
//             PciDevice->BusNumber,
//             PciDevice->DeviceNumber,
//             PciDevice->FunctionNumber,
//             EfiPciBeforeChildBusEnumeration
//             );
//       }



//       if (IS_PCI_BRIDGE (&Pci) || IS_CARDBUS_BRIDGE (&Pci)) {


//         Status = PciAllocateBusNumber (Bridge, *SubBusNumber, 1, SubBusNumber);
//         if (EFI_ERROR (Status)) {
//           return Status;
//         }
//         SecondBus = *SubBusNumber;

//         Register  = (UINT16) ((SecondBus << 8) | (UINT16) StartBusNumber);
//         Address   = EFI_PCI_ADDRESS (StartBusNumber, Device, Func, PCI_BRIDGE_PRIMARY_BUS_REGISTER_OFFSET);

//         Status = PciRootBridgeIo->Pci.Write (
//                                         PciRootBridgeIo,
//                                         EfiPciWidthUint16,
//                                         Address,
//                                         1,
//                                         &Register
//                                         );


//         //
//         // If it is PPB, resursively search down this bridge
//         //
//         if (IS_PCI_BRIDGE (&Pci)) {

//           //
//           // Temporarily initialize SubBusNumber to maximum bus number to ensure the
//           // PCI configuration transaction to go through any PPB
//           //
//           Register  = PciGetMaxBusNumber (Bridge);
//           Address   = EFI_PCI_ADDRESS (StartBusNumber, Device, Func, PCI_BRIDGE_SUBORDINATE_BUS_REGISTER_OFFSET);
//           Status = PciRootBridgeIo->Pci.Write (
//                                           PciRootBridgeIo,
//                                           EfiPciWidthUint8,
//                                           Address,
//                                           1,
//                                           &Register
//                                           );

//           //
//           // Nofify EfiPciBeforeChildBusEnumeration for PCI Brige
//           //
//           PreprocessController (
//             PciDevice,
//             PciDevice->BusNumber,
//             PciDevice->DeviceNumber,
//             PciDevice->FunctionNumber,
//             EfiPciBeforeChildBusEnumeration
//             );

//           Status = PciScanBus (
//                     PciDevice,
//                     SecondBus,
//                     SubBusNumber,
//                     PaddedBusRange
//                     );
//           if (EFI_ERROR (Status)) {
//             return Status;
//           }
//         }



//         //
//         // Set the current maximum bus number under the PPB
//         //
//         Address = EFI_PCI_ADDRESS (StartBusNumber, Device, Func, PCI_BRIDGE_SUBORDINATE_BUS_REGISTER_OFFSET);

//         Status = PciRootBridgeIo->Pci.Write (
//                                         PciRootBridgeIo,
//                                         EfiPciWidthUint8,
//                                         Address,
//                                         1,
//                                         SubBusNumber
//                                         );
//       } else  {
//         //
//         // It is device. Check PCI IOV for Bus reservation
//         // Go through each function, just reserve the MAX ReservedBusNum for one device
//         //
//         if (PcdGetBool (PcdSrIovSupport) && PciDevice->SrIovCapabilityOffset != 0) {
//           if (TempReservedBusNum < PciDevice->ReservedBusNum) {

//             Status = PciAllocateBusNumber (PciDevice, *SubBusNumber, (UINT8) (PciDevice->ReservedBusNum - TempReservedBusNum), SubBusNumber);
//             if (EFI_ERROR (Status)) {
//               return Status;
//             }
//             TempReservedBusNum = PciDevice->ReservedBusNum;

//             if (Func == 0) {
//               DEBUG ((EFI_D_INFO, "PCI-IOV ScanBus - SubBusNumber - 0x%x\n", *SubBusNumber));
//             } else {
//               DEBUG ((EFI_D_INFO, "PCI-IOV ScanBus - SubBusNumber - 0x%x (Update)\n", *SubBusNumber));
//             }
//           }
//         }
//       }

//       if (Func == 0 && !IS_PCI_MULTI_FUNC (&Pci)) {

//         //
//         // Skip sub functions, this is not a multi function device
//         //

//         Func = PCI_MAX_FUNC;
//       }
//     }
//   }

//   return EFI_SUCCESS;
// }
BOOLEAN
IsPciMultiFunc (
  IN  UINT16                               Bus,
  IN  UINT8                               Device,
  IN  UINT8                               Func
  )
{
  UINT64      Address;
  UINT8       HeaderType = 0xFF;
  //
  // Create PCI address map in terms of Bus, Device and Func
  //
  Address = PCI_LIB_ADDRESS (Bus, Device, Func, 0x0E);

  //
  // Read the Vendor ID register
  //
  HeaderType = PciRead8 (Address);

if (HeaderType & HEADER_TYPE_MULTI_FUNCTION) {
    return TRUE;
}
  return FALSE;
}

BOOLEAN
IsPciBridge (
  IN  UINT16                              Bus,
  IN  UINT8                               Device,
  IN  UINT8                               Func
  )
{
  UINT64      Address;
  UINT8       HeaderType = 0xFF;
  //
  // Create PCI address map in terms of Bus, Device and Func
  //
  Address = PCI_LIB_ADDRESS (Bus, Device, Func, 0x0E);

  //
  // Read the Vendor ID register
  //
  HeaderType = PciRead8 (Address);

if ((HeaderType & HEADER_LAYOUT_CODE) == (HEADER_TYPE_PCI_TO_PCI_BRIDGE)) {
    return TRUE;
}
  return FALSE;
}


UINT32
GetDidVid (
  IN  UINT16                              Bus,
  IN  UINT8                               Device,
  IN  UINT8                               Func
  )
{
    UINT64      Address;
    UINT32       DidVid = 0xFFFFFFFF;
    //
    // Create PCI address map in terms of Bus, Device and Func
    //
    Address = PCI_LIB_ADDRESS (Bus, Device, Func, 0);

    //
    // Read the Vendor ID register
    //
    DidVid = PciRead32 (Address);
    if ((DidVid != 0) && (DidVid != 0xFFFFFFFF)) {
        return DidVid;
    }

    return 0xFFFFFFFF;
}


VOID
ResetAllPpbBusNumber (
  IN UINT8                              StartBusNumber
  )
{
  UINT8                           Device;
  UINT32                          Register;
  UINT8                           Func;
  UINT64                          Address;
  UINT8                           SecondaryBus;
  UINT32                            DidVid;



  for (Device = 0; Device <= PCI_MAX_DEVICE; Device++) {
    for (Func = 0; Func <= PCI_MAX_FUNC; Func++) {

        DidVid = GetDidVid (StartBusNumber, Device, Func);
        if ((Func == 0) && (DidVid == 0xFFFFFFFF)) {
            break;
        }

      if (IsPciBridge (StartBusNumber, Device, Func)) {

        Register  = 0;
        Address   = PCI_LIB_ADDRESS (StartBusNumber, Device, Func, 0x18);
        Register = PciRead32 (Address);
        SecondaryBus = (UINT8)(Register >> 8);

        if (SecondaryBus != 0) {
          ResetAllPpbBusNumber (SecondaryBus);
        }

        //
        // Reset register 18h, 19h, 1Ah on PCI Bridge
        //
        Register &= 0xFF000000;
        PciWrite32 (Address, Register);
      }

      if (Func == 0 && !IsPciMultiFunc (StartBusNumber, Device, Func)) {
        //
        // Skip sub functions, this is not a multi function device
        //
        Func = PCI_MAX_FUNC;
      }
    }
  }
}

// VOID
// GetTheEndPointOnBridge (
//     UINT16                              Bus,
//     UINT8                               Device,
//     UINT8                               Func,
//     UINT16                              EndBus
//     )
// {
//     UINT8           PrimaryBus;
//     UINT8           SecondaryBus;
//     UINT8           SubordinateBus;
//     UINT16                              TempBus;
//     UINT8                               TempDevice;
//     UINT8                               TempFunc;
//     UINT64                              Address;
//     UINT32                              DidVid;

//     PrimaryBus = Bus;
//     Address = PCI_LIB_ADDRESS (Bus, Device, Func, 0x18);
//     PciWrite8 (Address, PrimaryBus);

//     SecondaryBus =  Bus + 1;
//     Address = PCI_LIB_ADDRESS (Bus, Device, Func, 0x19);
//     PciWrite8 (Address, SecondaryBus);

//     SubordinateBus = EndBus - 1;
//     Address = PCI_LIB_ADDRESS (Bus, Device, Func, 0x1A);
//     PciWrite8 (Address, EndBus - 1);


//     for (TempBus = SecondaryBus; TempBus < EndBus; TempBus++) {
//         for (TempDevice = 0; TempDevice < PCI_MAX_DEVICE; TempDevice++) {
//             for (TempFunc = 0; TempFunc < PCI_MAX_FUNC; TempFunc++) {
//                 DidVid = GetDidVid (TempBus, TempDevice, TempFunc);
//                 if ((TempFunc == 0) && (DidVid == 0xFFFFFFFF)) {
//                     break;
//                 }
//                 if (DidVid == 0xFFFFFFFF) {
//                     break;
//                 }
//                 DEBUG ((EFI_D_ERROR, "TempBus 0x%x, TempDevice 0x%x, TempFunc 0x%x \n", TempBus, TempDevice, TempFunc));
//                 DEBUG ((EFI_D_ERROR, "DidVid = 0x%x \n", DidVid));
//                 if (IsPciBridge (TempBus, TempDevice, TempFunc)) {
//                     DEBUG ((EFI_D_ERROR, "This is Bridge \n"));
//                     GetTheEndPointOnBridge (TempBus, TempDevice, TempFunc, EndBus);
//                 }
//                 DEBUG ((EFI_D_ERROR, "This is endpoint \n"));
//                 //return;
//             }
//         }
//     }
// }

EFI_STATUS
GetBusIndex (
    IN OUT UINT64      *BusIndex,
    IN     UINT64      AddrLen
    )
{
    UINT64              Index;

    for (Index = 0; Index < AddrLen; Index++) {
        if (mBusPool[Index] == 0x01) {
            mBusPool[Index] = 0;
            *BusIndex = Index;
            return EFI_SUCCESS;
        }
    }
    return EFI_OUT_OF_RESOURCES;
}

VOID
PciScanBus (
    UINT64     PrimaryBus,
    UINT64     AddrLen
    )
{
    UINT8           SecondaryBus;
    UINT8           SubordinateBus;
    UINT8                               TempDevice;
    UINT8                               TempFunc;
    UINT32                              DidVid;
    UINT64                              Address;
    UINT64                             BusIndex;
    EFI_STATUS                          Status;

    DEBUG ((EFI_D_ERROR, "kavi A \n"));
    for (TempDevice = 0; TempDevice < PCI_MAX_DEVICE; TempDevice++) {
        for (TempFunc = 0; TempFunc < PCI_MAX_FUNC; TempFunc++) {
            
            DidVid = GetDidVid (PrimaryBus, TempDevice, TempFunc);
            if ((TempFunc == 0) && (DidVid == 0xFFFFFFFF)) {
                break;
            }
            if (DidVid == 0xFFFFFFFF) {
                break;
            }
            DEBUG ((EFI_D_ERROR, "PrimaryBus 0x%x, TempDevice 0x%x, TempFunc 0x%x \n", PrimaryBus, TempDevice, TempFunc));
            DEBUG ((EFI_D_ERROR, "DidVid = 0x%x \n", DidVid));
            if (IsPciBridge (PrimaryBus, TempDevice, TempFunc)) {
                DEBUG ((EFI_D_ERROR, "This is Bridge\n"));
                Status = GetBusIndex (&BusIndex, AddrLen);
                if (Status != EFI_SUCCESS) {
                    DEBUG ((EFI_D_ERROR, "There are not enough Bus resources !\n"));
                    return;
                }
                SecondaryBus = PrimaryBus + BusIndex;
                SubordinateBus = AddrLen;

                Address = PCI_LIB_ADDRESS (PrimaryBus, TempDevice, TempFunc, 0x18);
                PciWrite8 (Address, PrimaryBus);

                Address = PCI_LIB_ADDRESS (PrimaryBus, TempDevice, TempFunc, 0x19);
                PciWrite8 (Address, SecondaryBus);

                Address = PCI_LIB_ADDRESS (PrimaryBus, TempDevice, TempFunc, 0x1A);
                PciWrite8 (Address, AddrLen - 1);
                DEBUG ((EFI_D_ERROR, "kavi B \n"));
                PciScanBus (SecondaryBus, AddrLen);
                DEBUG ((EFI_D_ERROR, "kavi c \n"));
            }
            DEBUG ((EFI_D_ERROR, "kavi d \n"));
            DEBUG ((EFI_D_ERROR, "PrimaryBus 0x%x, TempDevice 0x%x, TempFunc 0x%x \n", PrimaryBus, TempDevice, TempFunc));
            DEBUG ((EFI_D_ERROR, "DidVid = 0x%x \n", DidVid));
            if (TempFunc == 0 && !IsPciMultiFunc (PrimaryBus, TempDevice, TempFunc)) {
                //
                // Skip sub functions, this is not a multi function device
                //
                TempFunc = PCI_MAX_FUNC;
            }
        }
    }
}

EFI_STATUS
PciHostBridgeEnume (
    IN EFI_HANDLE                                           RootBridgeHandle,
    IN EFI_PCI_HOST_BRIDGE_RESOURCE_ALLOCATION_PROTOCOL     *HostResAlloc
    )
{
    EFI_ACPI_ADDRESS_SPACE_DESCRIPTOR   *Configuration;
    UINT64                              Bus;
    EFI_STATUS                          Status;
    UINT64                             Index;
    UINT64                             BusIndex;

    //
    // Get the Bus information
    //
    Status = HostResAlloc->StartBusEnumeration (
                            HostResAlloc,
                            RootBridgeHandle,
                            (VOID **) &Configuration
                            );
    if (Configuration == NULL || Configuration->Desc == ACPI_END_TAG_DESCRIPTOR) {
        return EFI_INVALID_PARAMETER;
    }

    DEBUG ((EFI_D_ERROR, "AddrRangeMin - 0x%lx\n", Configuration->AddrRangeMin));
    DEBUG ((EFI_D_ERROR, "AddrRangeMax - 0x%lx\n", Configuration->AddrRangeMax));
    DEBUG ((EFI_D_ERROR, "AddrLen - 0x%lx\n", Configuration->AddrLen));

    mBusPool = AllocateZeroPool(Configuration->AddrLen);
    for (Index = 0; Index < Configuration->AddrLen; Index++) {
        mBusPool[Index] = 0x01;
    }

    
    Status = GetBusIndex (&BusIndex, Configuration->AddrLen);
    if (Status != EFI_SUCCESS) {
        DEBUG ((EFI_D_ERROR, "There are not enough Bus resources !\n"));
        return Status;
    }
    Bus = Configuration->AddrRangeMin + BusIndex;

    PciScanBus (Bus, Configuration->AddrLen);
    ResetAllPpbBusNumber (Bus);

    if (mBusPool != NULL) {
        FreePool(mBusPool);
        mBusPool = NULL;
    }

    return EFI_SUCCESS;
}

VOID EFIAPI Bf2OnEndOfDxeCallback (
    IN EFI_EVENT    Event,
    IN VOID         *Context)
{
    EFI_PCI_ROOT_BRIDGE_IO_PROTOCOL                     *PciRootBridgeIo;
    EFI_PCI_HOST_BRIDGE_RESOURCE_ALLOCATION_PROTOCOL    *HostResAlloc;
    EFI_STATUS                                          Status;
    UINTN                                               RootBridgeHandleCount;
    EFI_HANDLE                                          *RootBridgeHandleBuffer;
    UINTN                                               Index;

    Status = gBS->LocateHandleBuffer (
                                    ByProtocol,
                                    &gEfiPciRootBridgeIoProtocolGuid,
                                    NULL,
                                    &RootBridgeHandleCount,
                                    &RootBridgeHandleBuffer
                                    );
    if (EFI_ERROR (Status)) {
        return;
    }

    for (Index = 0; Index < RootBridgeHandleCount; Index++) {
        Status = gBS->HandleProtocol(
                                    RootBridgeHandleBuffer[Index],
                                    &gEfiPciRootBridgeIoProtocolGuid,
                                    (VOID **)&PciRootBridgeIo
                                    );
        if (EFI_ERROR (Status)) {
            continue;
        }

        Status = gBS->HandleProtocol(
                                    PciRootBridgeIo->ParentHandle,
                                    &gEfiPciHostBridgeResourceAllocationProtocolGuid,
                                    (VOID **)&HostResAlloc
                                    );
        if (EFI_ERROR (Status)) {
            continue;
        }
        PciHostBridgeEnume (RootBridgeHandleBuffer[Index], HostResAlloc);
    }
}

EFI_STATUS EFIAPI PciBusEnumeDxeEntry (
    IN EFI_HANDLE         ImageHandle,
    IN EFI_SYSTEM_TABLE  *SystemTable)
{
  EFI_STATUS                            Status;
  EFI_EVENT                             EndOfDxeEvent;

  Status = gBS->CreateEventEx (
                  EVT_NOTIFY_SIGNAL,
                  TPL_CALLBACK,
                  Bf2OnEndOfDxeCallback,
                  NULL,
                  &gEfiEndOfDxeEventGroupGuid,
                  &EndOfDxeEvent
                  );

    return EFI_SUCCESS;
}


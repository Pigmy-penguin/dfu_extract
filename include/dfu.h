#ifndef DFU_H
#define DFU_H
#include <stdint.h>

#define DFU_SIGNATURE "DFU"
#define DFU_SIGNATURE_LENGTH 3

typedef struct {
    char szSignature[5];    // Five-byte coded field
    uint8_t bVersion;       // One-byte coded field
    uint32_t DFUImageSize; // Four-byte coded field
    uint8_t bTargets;       // One-byte coded field
} __attribute__((__packed__)) DFUPrefix;

typedef struct {
    uint8_t bcdDeviceLo;         // Firmware version or 0xFFFF if ignored
    uint8_t bcdDeviceHi;
    uint8_t idProductLo;         // Product ID or 0xFFFF if ignored
    uint8_t idProductHi;
    uint8_t idVendorLo;          // Vendor ID or 0xFFFF if ignored
    uint8_t idVendorHi; 
    uint8_t bcdDFULo;            // Fixed value 0x011A for DFU specification number
    uint8_t bcdDFUHi;
    char ucSignature[3];        // Fixed string "DFU" in reverse order
    uint8_t bLength;            // Fixed value 16 for the length of DFU Suffix
    uint32_t dwCRC;             // CRC calculated over the whole file except for dwCRC itself
} __attribute__((__packed__)) DFUSuffix;

typedef struct {
    char szSignature[6];        // Six-byte coded field, fixed to "Target"
    uint8_t bAlternateSetting;  // One-byte field for device alternate setting
    uint32_t bTargetNamed;     // Four-byte boolean field for target naming
    char szTargetName[255];    // 255-byte field for target name
    uint32_t dwTargetSize;     // Four-byte field for image length excluding prefix
    uint32_t dwNbElements;   // Three-byte field for the number of elements in the image
} __attribute__((__packed__)) TargetPrefix;

typedef struct {
    uint32_t dwElementAddress;  // 4-byte starting address of the data
    uint32_t dwElementSize;     // Size of the contained data
    // Add a placeholder for the Data field, which can be an array of bytes.
    // uint8_t Data[...];  // Replace '...' with the appropriate size for your data.
} __attribute__((__packed__)) ImageElement;

int extractDFU(const char* inputFile, const char* outputFile);

#endif

#include <ide.h>

struct IDEChannelRegisters
{
    uint16_t base;  // I/O Base.
    uint16_t ctrl;  // Control Base
    uint16_t bmide; // Bus Master IDE
    uint8_t nIEN;   // nIEN (No Interrupt);
} channels[2];

struct ide_device
{
    uint8_t Reserved;      // 0 (Empty) or 1 (This Drive really exists).
    uint8_t Channel;       // 0 (Primary Channel) or 1 (Secondary Channel).
    uint8_t Drive;         // 0 (Master Drive) or 1 (Slave Drive).
    uint16_t Type;         // 0: ATA, 1:ATAPI.
    uint16_t Signature;    // Drive Signature
    uint16_t Capabilities; // Features.
    uint32_t CommandSets;  // Command Sets Supported.
    uint32_t Size;         // Size in Sectors.
    uint8_t Model[41];     // Model in string.
} ide_devices[4];

uint8_t ide_buf[2048] = {0};
uint8_t ide_irq_invoked = 0;
uint8_t atapi_packet[12] = {0xA8, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

void ide_install()
{
}
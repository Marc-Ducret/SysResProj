#include "disk.h"

bus primary_bus;
bus secondary_bus;

bus *pbus = &primary_bus;
bus *sbus = &secondary_bus;

// TODO Clear control register ?

void init_bus(bus *bus, u16 base_port) {
    bus->data_port = base_port;
    bus->error_port = base_port + 1;
    bus->sector_count = base_port + 2;
    bus->lba_lo = base_port + 3;
    bus->lba_mid = base_port + 4;
    bus->lba_hi = base_port + 5;
    bus->drive_select = base_port + 6;
    bus->command_port = base_port + 7;
    bus->control_register = base_port + 0x206;
}

void create_bus() {
    init_bus(pbus, PRIMARY_BUS);
    init_bus(sbus, SECONDARY_BUS);
}

int disk_identify() {
    u8 status;
    
    // Master drive
    outportb(pbus->drive_select, 0xA0);
    
    outportb(pbus->sector_count, 0x0);
    outportb(pbus->lba_lo, 0x0);
    outportb(pbus->lba_mid, 0x0);
    outportb(pbus->lba_hi, 0x0);
    
    // Identify command
    outportb(pbus->command_port, 0xEC);
    
    status = inportb(pbus->command_port);
    
    if (!status) {
        // No drive..
        kprintf("The drive doesn't exist..\n");
        return 0;
    }
    else {
        kprintf("Found a drive !\n");
        while (inportb(pbus->command_port) & 0x80) {}
        
        //Checks the LBAmid and LBAhi
        if (inportb(pbus->lba_mid) || inportb(pbus->lba_hi)) {
            kprintf("Not ATA drive.\n");
            return 1;
        }  
        
        while (!(inportb(pbus->command_port) & 0x09)) {}
        
        //Checks error bit is clear
        if (inportb(pbus->command_port) & 0x1) {
            kprintf("Error bit was set.\n");
            return 3;
        }
        u16 table[256];
        for (int i = 0; i < 256; i++) {
            table[i] = inportw(pbus->data_port);
        }
        kprintf("Driver properties :\n");
        
        if ((table[88] >> 10) & 1) {
            kprintf(" - This drive supports LBA48 mode.\n");
        }
        else {
            kprintf(" - This drive does not support LBA48 mode.\n");
        }
        
        u32 lba28_sectors = table[60] | (table[61] << 16);
        kprintf(" - There are %d 28-bit LBA addressable sectors.\n\n", lba28_sectors);
        return 2;
    }
}

status_byte get_status(bus *bus) {
    status_byte_bis status;
    status.content = inportb(bus->command_port);
    return status.bits;
}

int is_busy(status_byte status) {
    return !(!status.busy || status.err || status.df);
}

int is_ready(status_byte status) {
    return !(status.err || status.df);
}

int poll(bus *bus) {
    // Just polls until drive is ready.
    while(is_busy(get_status(bus))) {}
    //do {
        //inportb(bus->command_port);
        //inportb(bus->command_port);
        //inportb(bus->command_port);
        //inportb(bus->command_port);
    //} while (is_busy(get_status(bus)));
    return is_ready(get_status(bus));
}

void software_reset(bus *bus) {
    // Sets the SRST bit of control register.
    outportb(bus->control_register, 0x4);
    //Clears the control register
    outportb(bus->control_register, 0x0);
}

void read_sectors(u32 sector, u8 sector_count, void *buffer) {
    read_address(sector * 512, sector_count * 512, buffer);
}

void read_address(u32 address, u32 length, void *buffer) {
    bus *bus = pbus;

    u32 start_sector = address / 512;
    u32 end_sector = (address + length - 1) / 512;
    u32 sector_count = end_sector - start_sector + 1;
    u32 start_offset = address % 512;
    //kprintf("Start sector %d, End_sector %d, Sector_count %d, Start_offset %d\n",
    //        start_sector, end_sector, sector_count, start_offset);
    assert(sector_count < 256);

    // Sends the read command.
    outportb(bus->drive_select, 0xE0 | ((start_sector >> 24) & 0x0F));
    //outportb(bus->error_port, 0x0);
    outportb(bus->sector_count, (u8) sector_count);
    outportb(bus->lba_lo, (u8) start_sector);
    outportb(bus->lba_mid, (u8) (start_sector >> 8));
    outportb(bus->lba_hi, (u8) (start_sector >> 16));
    outportb(bus->command_port, 0x20);
    
    // Waits the disk
    assert(poll(bus));
    
    // Throws the data until the wanted block starts.
    u32 index = 0;
    while (start_offset > 1) {
        inportw(bus->data_port);
        start_offset -= 2;
        index ++;
    }
    if (start_offset) {
        // Throws the last byte and reads the first we want.
        u16 data = inportw(bus->data_port);
        *((u8*) buffer) = data >> 8;
        length --;
        index ++;
        buffer = ((u8*) buffer) + 1;
    }
    
    // Read until one or zero byte lefts.
    while (length > 1) {
        // At each new sector, waiting for the disk.
        if (index % 256 == 0)
            assert(poll(bus));
        
        *((u16*) buffer) = inportw(bus->data_port);
        buffer = ((u16*) buffer) + 1; // Read two new bytes.
        length -= 2;
        index ++;
    }
    
    // If one byte left, read it.
    if (length) {
        u16 data = inportw(bus->data_port);
        *((u8*) buffer) = (u8) data;
        length --;
        index ++;
        buffer = ((u8*) buffer) + 1;
    }
    
    // Finish reading the full sector.
    while (index % 256) {
        inportw(bus->data_port);
        index ++;
    }
    
    // Checking the results.
    assert(length == 0);
    assert(index == 256 * sector_count);
}

void write_sectors(u32 sector, u8 sector_count, void *buffer) {
    bus *bus = pbus;

    // sector_count = 0 means 256
    assert(sector_count);
    
    outportb(bus->drive_select, 0xE0 | ((sector >> 24) & 0x0F));
    //outportb(bus->error_port, 0x0);
    outportb(bus->sector_count, sector_count);
    outportb(bus->lba_lo, (u8) sector);
    outportb(bus->lba_mid, (u8) (sector >> 8));
    outportb(bus->lba_hi, (u8) (sector >> 16));
    outportb(bus->command_port, 0x30);
    
    for (int i = 0; i < sector_count; i++) {
        // Waiting for an IRQ, or polling. (Here polling)
        assert(poll(bus));
        
        // Transfer the data from port to buffer.
        for (int j = 0; j < 256; j++) {
            outportw(bus->data_port, ((u16*) buffer)[j]);
        }
        
        buffer = ((u16*) buffer) + 256;
    }
    outportb(bus->command_port, 0xE7);
    poll(bus);
}

void write_address(u32 address, u32 length, void *buffer) {
    u32 start_sector = address / 512;
    u32 end_sector = (address + length - 1) / 512;
    u32 sector_count = end_sector - start_sector + 1;
    u32 start_offset = address % 512;
    //kprintf("Start sector %d, End_sector %d, Sector_count %d, Start_offset %d\n",
    //        start_sector, end_sector, sector_count, start_offset);
    assert(sector_count < 256);

    u8 tmp_buffer[512];
    
    if (start_offset) {
        // Reads the sector to complete the writing data.
        u32 to_copy = min(512 - start_offset, length);
        read_sectors(start_sector, 1, tmp_buffer);
        memcpy((void*) tmp_buffer + start_offset, buffer, to_copy);
        write_sectors(start_sector, 1, tmp_buffer);
        length -= to_copy;
        buffer = ((u8*) buffer) + to_copy;
        start_sector++;
    }

    // Writes the following sectors.
    u32 plenty_sectors = length / 512;
    if (plenty_sectors) {
        write_sectors(start_sector, plenty_sectors, buffer);
        length -= 512 * plenty_sectors;
        buffer = ((u8*) buffer) + 512 * plenty_sectors;
    }

    // Reads and the writes the last sector.
    if (length) {
        read_sectors(end_sector, 1, tmp_buffer);
        memcpy(tmp_buffer, buffer, length);
        write_sectors(end_sector, 1, tmp_buffer);
    }
}

void init_disk(int test) {
    kprintf("Initialising Disk \n");
    create_bus();
    software_reset(pbus);
    assert(disk_identify() == 2); // Asserts that it is an ATA drive.
    outportb(pbus->control_register, 0x0);  // Sets control register to 0.
    
    
    if (test) {
        u16 buffer[256];
        memset(buffer, 0x0, 512);
        read_address(0x130, 5, buffer);
        kprintf("Status after first read : %x\n", inportb(pbus->command_port));
        
        for (int i = 0; i < 20; i++) {
            kprintf("%h ", buffer[i]);
        }
        kprintf("...\n");
        
        memset((void*)buffer, 0x55, 512);
        write_address(0x13000, 0x200, buffer);
        kprintf("Status after writing : %x\n", inportb(pbus->control_register));
    }
    
    kprintf("Disk initialised.\n");
}
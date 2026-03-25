 /*
 ============================================================================
 Name        : Lab6.c
 Author      : Austin Tian
 Revised by  :
 Version     :
 Copyright   : Copyright 2023
 Description : Lab 6 in C, ANSI-C Style
 ============================================================================
 */

#include "header.h"
#include <time.h>

// Menu for the testing.
char *menu =    "\n" \
                " ***********Please select the following options**********************\n" \
                " *    This is the memory operation menu (Lab 6)                     *\n" \
                " ********************************************************************\n" \
                " *    1. Write a double-word (32-bit) to the memory                 *\n"  \
                " ********************************************************************\n" \
                " *    2. Read a byte (8-bit) data from the memory                   *\n" \
                " *    3. Read a double-word (32-bit) data from the memory           *\n" \
                " ********************************************************************\n" \
                " *    4. Generate a memory dump from any memory location            *\n" \
                " ********************************************************************\n" \
                " *    e. To Exit, Type 'e'  or 'E'                                  *\n" \
                " ********************************************************************\n";

//---------------------------------------------------------------
// Generate a random number between 0x00 and 0xFF.
unsigned char rand_generator()
{
    return rand()%256;  // generate a random number between 0 and 255.
}
//---------------------------------------------------------------
void free_memory(char *base_address)
{
    free(base_address);  // free memory after use to avoid memory leakage.
    return;
}
//---------------------------------------------------------------
char *init_memory()
{
    char *mem = malloc(MEM_SIZE);  // allocate the memory
    //fills in the memory with random numbers 
    //we loop through every byte in our 1MB memory and put a random number in each spot
    int i;
    if (mem ==NULL){
        puts("Memory alloc failed");
        exit(EXIT_FAILURE);
    }
    srand((unsigned int)time(NULL));
    for (i=0; i < MEM_SIZE; i++)
    {
        mem[i]= (char)rand_generator();//puts a random byte from 0 to 255 into every memory location
    }

    return mem;
}

//---------------------------------------------------------------
//Writes a double word 32 bits=4 bytes to memory
//address is defined by base_address + offset
//uses memcpy to copy all 4 bytes into memory
void write_dword(const char *base_address, const int offset, const unsigned int dword_data){
    if (offset <0 || offset +(int)sizeof(unsigned int) > MEM_SIZE)
    {
        puts("write failed cause of invalid offset");
        return;
    }
    memcpy((void *)(base_address+offset), &dword_data, sizeof(unsigned int));
    printf("Double word 0x%08X written to offset 0x%X\n", dword_data, offset);
}
//---------------------------------------------------------------
//Reads a single byte from address defined by base_address + offset
//casting to unsigned char so it shows 0x00 to 0xFF not negative
unsigned char read_byte(const char *base_address, const int offset){
    unsigned char byte_data;
    if (offset <0 || offset >=MEM_SIZE)
    {
        puts("Read byte failed cause of invalid offset");
        return 0;
    }
    byte_data= *(unsigned char *)(base_address + offset);
    printf("Byte at 0x%X = 0x%02X \n", offset, byte_data);
    return byte_data;
}
//---------------------------------------------------------------
//Reads a double word, 4 bytes, from memory based on address(base_address + offset)
//uses memcpy to copy all 4 bytes into an unsigned int
unsigned int read_dword(const char *base_address, const int offset){
    unsigned int dword_data;

    if (offset <0 || offset + (int)sizeof(unsigned int) > MEM_SIZE)
    {
        puts("Reading double word failed cause of invalid offset");
        return 0;
    }
    memcpy(&dword_data, base_address +offset, sizeof(unsigned int));
    printf("Double word at 0x%X = 0x%08X \n", offset, dword_data);
    return dword_data;
}
//---------------------------------------------------------------
//Generates a memory dump display from base_address + offset 
// and shows the contents in HEX and displays its ASCII code
// if the address content is not between the range 0x20 and 0x7E it displays the character "."
//Address is displayed on the left, HEX in the middle and ASCII on the right
void memory_dump(const char *base_address, const int offset, unsigned int dumpsize){
    int i, j;
    if (offset < 0 || offset >= MEM_SIZE)
    {
        puts ("Memory dump failed");
        return;
    }
    if (dumpsize < MIN_DUMP_SIZE || dumpsize > MEM_SIZE)
        dumpsize = MIN_DUMP_SIZE;  // min dumpsize is 256
    if ((unsigned int)offset+dumpsize > MEM_SIZE)
        dumpsize =MEM_SIZE- (unsigned int)offset;    
        //loop to go through mem in chunks of 16bytes
        for (i=0;i<(int)dumpsize; i+=DUMP_LINE)
        {
            //addiing base address +offset+i to get the actual address
            printf("%p: ", (void *)(base_address + offset + i));

            //prints 16 hex bytes for the row
            for (j=0; j<DUMP_LINE;j++){
                if (i +j < (int)dumpsize){
                //reads each byte and prints it as 2 digit hex number
                unsigned char byte = *(unsigned char *)(base_address+offset+i+j);
                printf("%02X ", byte);
                } else {
                    printf("   ");
                }
            }

            printf("  ");

            //prints the ascii chaaracter
            //if byte is between 0x20(space) and 0x7E, ~, it prints the character
            //if not it prints a dot
            for(j=0; j<DUMP_LINE;j++){
                if (i + j<(int)dumpsize){
                unsigned char byte = *(unsigned char *)(base_address+offset+i+j);
                if (byte >=0x20 && byte <= 0x7E)
                {
                    printf("%c", byte);
                } else {
                    printf(".");
                }
            }
            }
            printf("\n");
        }
        
    return;

}

//---------------------------------------------------------------
//
void setup_memory()
{
    // Now we need to setup the memory controller for the computer system we
    // will build. Basic requirements:
    // 1. Memory size needs to be 1M Bytes
    // 2. Memory is readable/writable with Byte and Double-Word Operations.
    // 3. Memory can be dumped and shown on screen.
    // 4. Memory needs to be freed (released) at the end of the code.
    // 6. For lab 6, we need to have a user interface to fill in memory,
    //                                      read memory and do memory dump.
    char *mem = init_memory();  // initialize the memory.
    char options =0;
    unsigned int offset, dumpsize;
    char tempchar;
    unsigned int dword_data;      // 32-bit operation.
    do{
        if (options != 0x0a)  // if options has a return key input, skip it.
        {
            puts(menu); /* prints Memory Simulation */
            printf ("\nThe base address of your memory is: %I64Xh (HEX)\n", (long long unsigned int)(mem));  // output base memory first.
            puts("Please make a selection:");  // output base memory first.
        }
            options = getchar();

            switch (options)
            {
                case '1':  // writes double word.
                    puts("Please input your memory's offset address (in HEX):");
                    scanf("%x", (unsigned int*)&offset);    // input an offset address (in HEX) to write.
                    puts("Please input your DOUBLE WORD data to be written (in HEX):");
                    scanf("%x", (unsigned int*)&dword_data);    // input data
                    write_dword (mem, offset, dword_data);  // write a double word to memory.
                    continue;
                case '2':  // read byte.
                    puts("Please input your memory's offset address (in HEX):");
                    scanf("%x", &offset);    // input an offset address (in HEX) to write.
                    read_byte(mem, offset);
                    continue;
                case '3':  // read double word.
                    puts("Please input your memory's offset address (in HEX):");
                    scanf("%x", &offset);    // input an offset address (in HEX) to write.
                    read_dword(mem, offset);
                    continue;
                case '4':  // generate memory dump starting at offset address (in HEX).
                    puts("Please input your memory's offset address (in HEX, should be a multiple of 0x10h):");
                    scanf("%x", &offset);    // input an offset address (in HEX) to start.
                    puts("Please input the size of the memory to be dumped (a number between 256 and 1024 ):");
                    scanf("%d", &dumpsize);    // The size of the memory dump
                    memory_dump(mem, offset, dumpsize);  // generate a memory dump display of dumpsize bytes.
                    continue;
                case 'e':
                case 'E':
                    puts("Code finished, press any key to exit");
                    free_memory(mem);
                    while ((tempchar = getchar()) != '\n' && tempchar != EOF);  // wait for the enter.
                    tempchar = getchar();
                    return;  // return to main program.
                default:
                    // puts("Not a valid entry, please try again");
                    continue;
            }
    }while (1);  // make sure the only exit is from 'e'.
    return;
}

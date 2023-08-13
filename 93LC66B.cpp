#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <cstdint>
#include <vector>
#include <string>
#include <csignal>
#include <map>

#include <sys/stat.h>
#include <fcntl.h>

#define CS 6
#define CLK 13
#define DI 19
#define DO 26

#define IN 0
#define OUT 1

int period_clk = 1000 ; 
std::map<int,int> gpio ; 

void set_pin (int gp) {write(gpio[gp], "1", 1) ; }
void unset_pin(int gp){write(gpio[gp], "0", 1) ; }
uint16_t read_pin (int gp) {
  uint8_t c[2] ;
  lseek(gpio[gp], 0, SEEK_SET) ;  
  int n = read (gpio[gp], c, 2) ; 
  if (c[0]=='1') return 1 ; 
  else return 0 ;  
}


void clk () 
{
  usleep(period_clk) ;
  set_pin(CLK) ; 
  usleep(period_clk) ; 
  unset_pin(CLK) ; 
  usleep(period_clk) ;   
}

//================================================================
int open_gpio (int gpio_number, int direction)
{
int gpio ; char path[500] ;  
int init, dir, res ; 
sprintf(path, "/sys/class/gpio/gpio%d/value", gpio_number) ;
gpio=open(path, O_WRONLY) ;
if (gpio <0)
{
 init=open("/sys/class/gpio/export", O_WRONLY) ;
 sprintf(path, "%d", gpio_number) ;
 write(init, path, strlen(path)) ;
 printf("{--%d--}", init) ; fflush(stdout) ;
 close(init) ;

 sprintf(path, "/sys/class/gpio/gpio%d/value", gpio_number) ;
 gpio=open(path, O_WRONLY) ;
}
if (gpio)
{
 sprintf(path, "/sys/class/gpio/gpio%d/direction", gpio_number) ;
 dir = open(path, O_WRONLY) ;
 if (direction==IN)
 {
 	res=write(dir,"in", 2) ;
	close(gpio) ; 
        sprintf(path, "/sys/class/gpio/gpio%d/value", gpio_number) ;
	gpio=open(path, O_RDONLY) ; 
 }
 else
	res=write(dir,"out",3) ; 
 printf("%d ", res) ; fflush(stdout) ;
 close(dir) ;
}
if (gpio < 0) printf("The GPIO port %d could not be opened\n", gpio_number) ; 
return gpio ; 
}

//--------------------------
void EWEN () // Write enable command
{
  usleep(1000) ; 
  set_pin(CS) ; 
  set_pin(DI) ; 
  clk() ; 
  unset_pin(DI) ; 
  clk() ; 
  clk() ; 
  set_pin(DI) ; 
  for (int i=0 ; i<8 ; i++) clk() ; 
  unset_pin(CS) ; 
  usleep(1000) ; 
}
//------------------------
void ERAL ()
{
  usleep(1000) ; 
  set_pin(CS) ;
  set_pin(DI) ; 
  clk() ; 
  unset_pin(DI) ; 
  clk() ; 
  clk() ; 
  set_pin(DI) ; 
  clk() ; 
  unset_pin(DI) ; 
  clk() ; 
  for (int i=0 ; i<7 ; i++) clk() ; 
  unset_pin(CS) ; 
  usleep(1000) ; 
}
//------------------------
void write_at (uint8_t address, uint16_t value)
{
  usleep(1000) ;  
  set_pin(CS) ;  
  set_pin(DI) ; 
  clk() ; 
  unset_pin(DI) ; 
  clk() ; 
  set_pin(DI) ; 
  clk() ; 
  for (int i=7; i>=0 ; i--)
  {
    if ((address>>i) & 1) set_pin(DI) ; 
    else unset_pin(DI) ; 
    clk() ; 
  }  
  for (int i=15 ; i>=0 ; i--)
  {
    if ((value >> i) & 1) set_pin(DI) ; 
    else unset_pin(DI) ; 
    clk() ; 
  } 
  unset_pin(CS) ;
  //usleep(1) ; 
  //set_pin(CS) ; 
  //usleep(1000) ;
  //set_pin(DI) ;  
  //usleep(10) ; 
  //unset_pin(CS) ;
  //usleep(10) ; 
  //unset_pin(DI) ;  
  usleep(10000) ; 
}
//-------------------------
void write_hexfile (char path[], int offset, int length)
{
  FILE * in = fopen(path, "rb") ; 
  if (in==NULL) {printf("File cannot be opened.\n") ; fflush(stdout) ; return ; }

  fseek(in, offset*2, SEEK_SET) ; 
  uint16_t value ; 
  for (int i=0 ; i<length ; i++)
  {
    fread(&value, 2, 1, in) ; 
    if ((offset+i)%16==0) printf("\nw-%03X: ", offset+i) ;     
    printf("%04X ", value) ; fflush(stdout) ; 
    write_at (offset+i, value) ;
    if (feof(in)) 
      break ;  
  }
  fclose(in) ; 
}


/*=============================*/
int main (int argc, char *argv[])
{
gpio[CLK]=open_gpio(CLK,OUT) ; 
gpio[CS]=open_gpio(CS,OUT) ;
gpio[DI]=open_gpio(DI,OUT) ; 
gpio[DO]=open_gpio(DO,IN) ; 

if (argc>1)
{
  unset_pin(CS) ; 
  unset_pin(CLK) ; 
  unset_pin(DI) ;
  exit(0) ;  
}


unset_pin(CS) ; 
unset_pin(CLK) ; 
unset_pin(DI) ; 
usleep(1000000) ;


// Doing a write
if (0)
{
  EWEN() ; 
  //ERAL() ;
  char path[] ="ManufacturedHex.bin" ; 
  write_hexfile(path, 0, 256) ; 
}


printf("\nStarting read\n"); fflush(stdout) ; 
FILE * dmp = fopen("Dump.hex", "wb") ; 

set_pin(CS) ; 
set_pin(DI) ; 
clk() ; 
clk() ; 
unset_pin(DI) ; 
clk() ; 
for (int i=0 ; i<8 ; i++) clk() ; 
    
for (int i=0 ; i<4096/16 ; i++)
{
  uint16_t value[1]={0} ;
  for (int j=0 ; j<16 ; j++)
  {
    int bit ; 
    clk() ; 
    bit = read_pin(DO) ; 
    value[0] |= (bit<<(15-j)) ;   
  }
  fwrite(value, 2, 1, dmp) ;
  if (i%16==0) printf("\nr-%03X: ", i) ;   
  printf("%04X ", value[0]) ; fflush(stdout) ;  
}
fclose(dmp) ; 

return 0 ; 
}

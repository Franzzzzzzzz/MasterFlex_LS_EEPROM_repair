#include <cstdlib>
#include <cstdio>
#include <cstdint>

int main (int argc, char * argv [])
{
FILE * out = fopen("ManufacturedHex.bin", "wb") ; 

FILE *in1 = fopen("Dump_afterfactreset.hex", "rb") ; 
FILE *in2 = fopen("Dump_MSpump_original.hex", "rb") ; 

if (out == NULL || in1 == NULL || in2 == NULL)
{
  printf("ERROR opening files.\n") ; 
  exit(0) ; 
}

uint16_t value ; 
for (int i=0 ; i<0x7E ; i++)
{
  fread(&value, 2,1, in1) ; 
  fwrite(&value,2,1,out) ; 
}

fseek(in2, 0x7E*2, SEEK_SET) ; 
for (int i=0x7E ; i<256 ; i++)
{
  fread(&value, 2,1, in2) ; 
  fwrite(&value,2,1,out) ; 
}

fclose(in1) ; 
fclose(in2) ; 
fclose(out) ; 
}


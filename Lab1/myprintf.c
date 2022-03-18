#include <stdio.h>
#include <stdlib.h>

typedef unsigned int u32;

char *ctable = "0123456789ABCDEF";
int  BASE = 10;
int *FU;

int rpu(u32 x)
{
  char c;
  if (x){
    c = ctable[x % BASE];
    rpu(x / BASE);
    putchar(c);
  }
}

int printu(u32 x)
{
  //printf("In printu");
  (x==0)? putchar('0') : rpu(x);
  putchar(' ');
  //printf("Exit printu");
}


//prints s as a string
void prints(char *s)
{
  //printf("In prints");
  while(*s)
    {
      putchar(*s);
      s++;
    }
  //printf("Exit prints");
}

// Prints x as an int
int printd(int x)
{
  //printf("In printd");
  char int_to_char;
  if (x < 0)
    {
      putchar('-');
      x = x * (-1);
    }

  printu(x);
  /*
  if(x >= 0)
    {
      if (x == 0)
	{
	  int_to_char = '0';
	}
      
      if(x != 0)
	{
	  int_to_char = ctable[x % 10];
	  printd(int_to_char / 10);
	}
      putchar(int_to_char);
    }
  */
  //printf("Exit printd");
}

//prints x in HEX
int printx_helper(u32 x)
{
  //printf("In printx");
  char hex_to_char;
  if(x)
    {
      hex_to_char = ctable[x % 16];
      printx_helper(x / 16);
      putchar(hex_to_char);
    }

  //printf("Exit printx");
}

int printx(u32 x)
{
  putchar('0');
  putchar('x');
  printx_helper(x);
}

//prints x in octal
int printo_helper(u32 x)
{
  //printf("In printo");
  char oct_to_char;
  if(x)
    {
      oct_to_char = ctable[x % 8];
      printo_helper(x / 8);
      putchar(oct_to_char);
    }
  //printf("Exit printo");
}

int printo(u32 x)
{
  putchar('0');
  printo_helper(x);
}

int myprintf(char *fmt, ...)
{
  //printf("In myprintf");
  char *chpt;
  int *intpt;
  asm("movl %ebp, FU");
  intpt = FU + 3;

  for (chpt = fmt; *chpt != '\0'; chpt++)
    {
      if (*chpt != '%')
	{
	  putchar(*chpt);
	  if (*chpt == '\n')
	    {
	      putchar('\r');
	    }
	  if (*chpt == '\t')
	    {
	      putchar('\t');
	    }
	}
      if (*chpt == '%')
	{
	  chpt++;
	  switch(*chpt)
	    {
	    case 'd':
	      printd(*intpt);
	      break;
	    case 'c':
	      putchar((char)(*intpt));
	      break;
	    case 's':
	      prints((char *)(*intpt));
	      break;
	    case 'o':
	      printo(*intpt);
	      break;
	    case 'x':
	      printx(*intpt);
	      break;
	    default:
	      myprintf("Invalid input");
	      break;
	    }
	  intpt++;
	}
    }
  //printf("Exit myprintf");
}

int main(int argc, char *argv[], char *env[])
{
  myprintf("ch=%c string=%s      dec=%d hex=%x oct=%o neg=%d\n", 'A', "this is a test", 100,    100,   100,  -100);

}
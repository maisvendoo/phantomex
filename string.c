/*------------------------------------------------------------------------------
//
//		Work with strings
//		(c) maisvendoo, 31.08.2013
//
//----------------------------------------------------------------------------*/
#include	"string.h"

/* HEX-digits table */
char	hex_table[16] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A',
			             'B', 'C', 'D', 'E', 'F'};

/*------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------*/
u32int strcpy(char* dest, char* src)
{
    int i = 0;

    while (src[i] != '\0')
    {
        dest[i] = src[i];
        i++;
    }

    dest[i] = '\0';

    return i;
}

/*------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------*/
u32int strlen(char* str)
{
    u32int i = 0;

    while (str[i] != 0)
    {
        i++;
    }

    return i;
}

/*------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------*/
int  strcmp(char* str1, char* str2)
{
     int i = 0;
     int failed = 0;
     while(str1[i] != '\0' && str2[i] != '\0')
     {
         if(str1[i] != str2[i])
         {
             failed = 1;
             break;
         }
         i++;
     }
     // why did the loop exit?
     if( (str1[i] == '\0' && str2[i] != '\0') ||
         (str1[i] != '\0' && str2[i] == '\0') )

         failed = 1;

     return failed;
}

/*------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------*/
void dec2dec_str(u32int value, char* dec_str)
{
  u32int mod = 0;
  u32int res = value;
  char tmp_str[256];

  int i = 0;
  int j = 0;

  do
  {
    mod = res % 10;
    res = res / 10;

    tmp_str[i] = hex_table[mod];
    i++;

  } while (res >= 10);

  if (res != 0 )
  {
    tmp_str[i] = hex_table[res];
    tmp_str[++i] = '\0';
  }
  else
    tmp_str[i] = '\0';

  for (j = 0; j < i; j++)
    dec_str[j] = tmp_str[i-j-1];

  dec_str[i] = '\0';
}

/*------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------*/
void dec2hex_str(u64int value, char* hex_str)
{
  u64int mod = 0;
  u64int res = value;

  int n = 8;
  int i = n;

  int j = 0;

  do
  {
    mod = res % 16;
    res = res / 16;

    hex_str[i] = hex_table[mod];
    i--;

  } while (res >= 16);

  hex_str[i] = hex_table[res];

  for (j = 0; j < i ; j++)
    hex_str[j] = '0';

  hex_str[n+1] = '\0';
}

/*------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------*/
void byte2hex_str(u8int value, char* hex_str)
{
  u32int mod = 0;
  u32int res = value;

  int i = 1;

  do
  {
    mod = res % 16;
    res = res / 16;

    hex_str[i] = hex_table[mod];
    i--;

  } while (res >= 16);

  hex_str[i] = hex_table[res];
  hex_str[2] = '\0';
}



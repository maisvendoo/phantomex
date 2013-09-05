/*------------------------------------------------------------------------------
//
//	PhantomEx, working with text framebuffer
//	(c) maisvendoo, 04.07.2013
//
//----------------------------------------------------------------------------*/
#include	"text_framebuffer.h"

/************************************************************************/
/*	Global framebuffer variables					*/
/************************************************************************/

/* Pointer to text framebuffer */
u16int* video_memory = (u16int*) VIDEO_MEMORY;

/* Background color		*/
u8int	background_color = BLACK;
u8int	default_background_color = BLACK;
/* Text color			*/
u8int	text_color = LIGHT_GRAY;
u8int	default_text_color = LIGHT_GRAY;

/* Screen params		*/
u8int	width = SCREEN_WIDTH;
u8int	height = SCREEN_HEIGHT;

/* Current cursor coordinates	*/
u8int	cur_x = 0;
u8int	cur_y = 0;

mutex_t	video_mutex;

/*------------------------------------------------------------------------------
//	Set hardware cursor in (x, y) position (text mode)
//----------------------------------------------------------------------------*/
void move_cursor(u8int x, u8int y)
{
  
  /* Word of cursor position */ 
  u16int cur_pos = y*width + x;
  
  /* Send of high byte to VGA controller */
  outb(0x3D4, 14);
  outb(0x3D5, cur_pos >> 8);
  
  /* Send of low byte to VGA controller */
  outb(0x3D4, 15);
  outb(0x3D5, cur_pos);
  
  /* Update current cursor position */
  cur_x = x;
  cur_y = y;
}

/*------------------------------------------------------------------------------
//	Set background color
//----------------------------------------------------------------------------*/
void set_bkground_color(u8int color)
{
  background_color = color;
}

/*------------------------------------------------------------------------------
//	Set text color
//----------------------------------------------------------------------------*/
void set_text_color(u8int color)
{
  text_color = color;
}

/*------------------------------------------------------------------------------
//	Screen scrolling
//----------------------------------------------------------------------------*/
static void scroll(void)
{
  /* Create attribute's byte*/
  u8int	attrib_byte = (default_background_color << 4) | (default_text_color & 0x0F);
  
  /* Blank symbol with attributes */
  u16int blank = 0x20 | (attrib_byte << 8);
  
  /* If we in end of screen */
  if (cur_y >= height)
  {
    int i;
    
    /* Shift last string to up */
    for (i = 0*width; i < (height - 1)*width; i++)
    {
      video_memory[i] = video_memory[i+width];
    }
    
    /* Put blank in last string */
    for (i = (height - 1)*width; i < height*width; i++)
    {
      video_memory[i] = blank;
    }
    
    cur_y = 24;
    move_cursor(cur_x, cur_y);
  }
}

/*------------------------------------------------------------------------------
//	Put character on screen
//----------------------------------------------------------------------------*/
void put_char(char c)
{
  u8int attrib_byte = (background_color << 4) | (text_color & 0x0F);
  
  u16int attrib_word = attrib_byte << 8;
  
  u16int* location;
  
  
  if ( c == 0x08 && cur_x ) /* Backspace */
  {
    cur_x--;
  }
  else if (c == 0x09) /* TAB */
  {
    cur_x = (cur_x + 8) &~(8-1);
  }
  else if (c == '\r') /* Return */
  {
    cur_x = 0;
  }
  else if (c == '\n') /* Enter */
  {
    cur_x = 0;
    cur_y++;
  }
  else /* Other symbols */
  {
    location = video_memory + (cur_y*width + cur_x);
    *location = c | attrib_word;
    cur_x++;
  }
  
  if (cur_x > width)
  {
    cur_x = 0;
    cur_y++;
  }
  
  scroll();
  
  move_cursor(cur_x, cur_y);
    
}

/*------------------------------------------------------------------------------
//	Clean screen
//----------------------------------------------------------------------------*/
void clear(void)
{
  u8int	attrib_byte = (default_background_color << 4) | (default_text_color & 0x0F);
  
  u16int blank = 0x20 | (attrib_byte << 8);  
  
  u32int wide_blank = (blank << 16) | blank;  
  
  u32int* wide_buf = (u32int*) video_memory;
  
  int N = (width >> 2)*height;
  
  int i;
  
  for (i = 0; i < width*height; i++)
  {
    video_memory[i] = blank;
  }
  
  /*for (i = 0; i < N; i++)
  {
    wide_buf[i] = wide_blank;
  }*/  
  
  move_cursor(0, 0);
}

/*------------------------------------------------------------------------------
//	Print text on screen
//----------------------------------------------------------------------------*/
void print_text(char* s)
{
  int i = 0;
  
  while (s[i])
  {
    put_char(s[i++]);
  }
}

/*------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------*/
void print_hex_value(u64int value)
{
  char tmp[8];
  
  dec2hex_str(value, tmp);
  
  print_text("0x");
  print_text(tmp);
}

/*------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------*/
void print_dec_value(u32int value)
{
  char tmp[256];
  
  dec2dec_str(value, tmp);
    
  print_text(tmp);
}

/*------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------*/
void print_byte(u8int value)
{
  char tmp[2];
  
  byte2hex_str(value, tmp);
    
  print_text(tmp);
}

/*------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------*/
void debug_msg(char* s, u32int value)
{
  print_text(s);
  print_hex_value(value);
  print_text("\n");
}

/*------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------*/
void print_dump(void* address, u32int size)
{
  u8int* dump = (u8int*) address;
  u32int addr_tmp = (u32int) address;
  u32int i = 0;
  u32int mark = 0; 
  
  for (i = 0; i < size; i++)
  {
    set_text_color(LIGHT_GRAY);
    
    if (mark == 0)
    {
	print_hex_value(addr_tmp);
	print_text(": ");
    }
    
    if (dump[i] != 0)
      set_text_color(LIGHT_GREEN);
    else
      set_text_color(LIGHT_GRAY);
    
    print_byte(dump[i]); 
    
    set_text_color(LIGHT_GRAY);
    
    if ( mark == 7 )
    {
      print_text("|");
      mark++;
    }
    else if ( mark == 15 )
    {
      print_text("\n");
      mark = 0;
      
      addr_tmp += 0x10;
    }
    else
    {
      print_text(" ");
      mark++;
    }
  }  
}

/*-----------------------------------------------------------------------------
 *
 * --------------------------------------------------------------------------*/
void print_whirl(void)
{
	char whirl_str[6] = "/|-|-/";
	int i = 0;
	int j = 0;

	for (i = 0; i < 10000; i++)
	{
		for (j = 0; j < 6; j++)
		{
			put_char(whirl_str[j]);
			put_char('\r');
		}
	}
}

/*-----------------------------------------------------------------------------
 *
 * --------------------------------------------------------------------------*/
void set_video_vaddr(void* vaddr)
{
	video_memory = (u16int*) vaddr;
}

/*------------------------------------------------------------------------------
//	Put character on virtual screen
//----------------------------------------------------------------------------*/
void vput_char(vscreen_t* vscr, char c)
{
  u8int attrib_byte = (background_color << 4) | (text_color & 0x0F);

  u16int attrib_word = attrib_byte << 8;

  u16int* location;


  if ( c == 0x08 && vscr->cur_x ) /* Backspace */
  {
	  vscr->cur_x--;
  }
  else if (c == 0x09) /* TAB */
  {
	  vscr->cur_x = (vscr->cur_x + 8) &~(8-1);
  }
  else if (c == '\r') /* Return */
  {
	  vscr->cur_x = 0;
  }
  else if (c == '\n') /* Enter */
  {
	  vscr->cur_x = 0;
	  vscr->cur_y++;
  }
  else /* Other symbols */
  {
    location = vscr->vmemory + (vscr->cur_y*width + vscr->cur_x);
    *location = c | attrib_word;
    vscr->cur_x++;
  }

  if (vscr->cur_x > width)
  {
	  vscr->cur_x = 0;
	  vscr->cur_y++;
  }
}

/*------------------------------------------------------------------------------
//	Print text on virtual screen
//----------------------------------------------------------------------------*/
void vprint_text(vscreen_t* vscr, char* s)
{
  int i = 0;

  while (s[i])
  {
    vput_char(vscr, s[i++]);
  }
}


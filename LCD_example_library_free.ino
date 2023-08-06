#define LCD_CS A3 // Chip Select goes to Analog 3
#define LCD_CD A2 // Command/Data goes to Analog 2
#define LCD_WR A1 // LCD Write goes to Analog 1
#define LCD_RD A0 // LCD Read goes to Analog 0

#define LCD_RESET A4 // Can alternately just connect to Arduino's reset pin

#define WR_STROBE { WR_ACTIVE; WR_IDLE; }

#define RD_ACTIVE  *rdPort &=  rdPinUnset
#define RD_IDLE    *rdPort |=  rdPinSet
#define WR_ACTIVE  *wrPort &=  wrPinUnset
#define WR_IDLE    *wrPort |=  wrPinSet
#define CD_COMMAND *cdPort &=  cdPinUnset
#define CD_DATA    *cdPort |=  cdPinSet
#define CS_ACTIVE  *csPort &=  csPinUnset
#define CS_IDLE    *csPort |=  csPinSet

#define DELAY7        \
  asm volatile(       \
    "rjmp .+0" "\n\t" \
    "rjmp .+0" "\n\t" \
    "rjmp .+0" "\n\t" \
    "nop"      "\n"   \
    ::);

#define write8inline(d) {                          \
    PORTD = (PORTD & B00000011) | ((d) & B11111100); \
    PORTB = (PORTB & B11111100) | ((d) & B00000011); \
    WR_STROBE; }
#define read8inline(result) {                       \
    RD_ACTIVE;                                        \
    DELAY7;                                           \
    result = (PIND & B11111100) | (PINB & B00000011); \
    RD_IDLE; }
#define setWriteDirInline() { DDRD |=  B11111100; DDRB |=  B00000011; }
#define setReadDirInline()  { DDRD &= ~B11111100; DDRB &= ~B00000011; }

// 9341 internal registers
#define ILI9341_SOFTRESET          0x01
#define ILI9341_SLEEPIN            0x10
#define ILI9341_SLEEPOUT           0x11
#define ILI9341_NORMALDISP         0x13
#define ILI9341_INVERTOFF          0x20
#define ILI9341_INVERTON           0x21
#define ILI9341_GAMMASET           0x26
#define ILI9341_DISPLAYOFF         0x28
#define ILI9341_DISPLAYON          0x29
#define ILI9341_COLADDRSET         0x2A
#define ILI9341_PAGEADDRSET        0x2B
#define ILI9341_MEMORYWRITE        0x2C
#define ILI9341_PIXELFORMAT        0x3A
#define ILI9341_FRAMECONTROL       0xB1
#define ILI9341_DISPLAYFUNC        0xB6
#define ILI9341_ENTRYMODE          0xB7
#define ILI9341_POWERCONTROL1      0xC0
#define ILI9341_POWERCONTROL2      0xC1
#define ILI9341_VCOMCONTROL1      0xC5
#define ILI9341_VCOMCONTROL2      0xC7
#define ILI9341_MEMCONTROL      0x36
#define ILI9341_MADCTL  0x36

#define ILI9341_MADCTL_MY  0x80
#define ILI9341_MADCTL_MX  0x40
#define ILI9341_MADCTL_MV  0x20
#define ILI9341_MADCTL_ML  0x10
#define ILI9341_MADCTL_RGB 0x00
#define ILI9341_MADCTL_BGR 0x08
#define ILI9341_MADCTL_MH  0x04

#define writeRegister8inline(a, d) { \
  CD_COMMAND; write8inline(a); CD_DATA; write8inline(d); }

#define writeRegister16inline(a, d) { \
  uint8_t hi, lo; \
  hi = (a) >> 8; lo = (a); CD_COMMAND; write8inline(hi); write8inline(lo); \
  hi = (d) >> 8; lo = (d); CD_DATA   ; write8inline(hi); write8inline(lo); }

//basic colors
#define	BLACK   0x0000
#define	BLUE    0x001F
#define LIGHT_BLUE 0x000F
#define	RED     0xF800
#define	GREEN   0x07E0
#define CYAN    0x07FF
#define MAGENTA 0xF81F
#define YELLOW  0xFFE0
#define WHITE   0xFFFF

volatile uint8_t *csPort, *cdPort, *wrPort, *rdPort;

uint8_t csPinSet, cdPinSet, wrPinSet, rdPinSet;

uint8_t csPinUnset, cdPinUnset, wrPinUnset, rdPinUnset;

int16_t _width, _height; // Display w/h as modified by current rotation
int16_t cursor_x, cursor_y;
uint16_t textcolor, textbgcolor;
uint8_t textsize, rotation;

void writeRegister32(uint8_t r, uint32_t d) {
  CS_ACTIVE;
  CD_COMMAND;
  write8inline(r);
  CD_DATA;
  //delayMicroseconds(10);
  write8inline(d >> 24);
  //delayMicroseconds(10);
  write8inline(d >> 16);
  //delayMicroseconds(10);
  write8inline(d >> 8);
  //delayMicroseconds(10);
  write8inline(d);
  CS_IDLE;
}

void setAddrWindow(int x1, int y1, int x2, int y2) {
  CS_ACTIVE;
  uint32_t t;
  t = x1;
  t <<= 16;
  t |= x2;
  writeRegister32(ILI9341_COLADDRSET, t);  // HX8357D uses same registers!
  t = y1;
  t <<= 16;
  t |= y2;
  writeRegister32(ILI9341_PAGEADDRSET, t); // HX8357D uses same registers!
  CS_IDLE;
}

void flood(uint16_t color, uint32_t len) {
  uint16_t blocks;
  uint8_t  i, hi = color >> 8,
              lo = color;

  CS_ACTIVE;
  CD_COMMAND;
  write8inline(0x2C);

  // Write first pixel normally, decrement counter by 1
  CD_DATA;
  write8inline(hi);
  write8inline(lo);
  len--;

  blocks = (uint16_t)(len / 64); // 64 pixels/block
  if(hi == lo) {
    // High and low bytes are identical.  Leave prior data
    // on the port(s) and just toggle the write strobe.
    while(blocks--) {
      i = 16; // 64 pixels/block / 4 pixels/pass
      do {
        WR_STROBE; WR_STROBE; WR_STROBE; WR_STROBE; // 2 bytes/pixel
        WR_STROBE; WR_STROBE; WR_STROBE; WR_STROBE; // x 4 pixels
      } while(--i);
    }
    // Fill any remaining pixels (1 to 64)
    for(i = (uint8_t)len & 63; i--; ) {
      WR_STROBE;
      WR_STROBE;
    }
  } else {
    while(blocks--) {
      i = 16; // 64 pixels/block / 4 pixels/pass
      do {
        write8inline(hi); write8inline(lo); write8inline(hi); write8inline(lo);
        write8inline(hi); write8inline(lo); write8inline(hi); write8inline(lo);
      } while(--i);
    }
    for(i = (uint8_t)len & 63; i--; ) {
      write8inline(hi);
      write8inline(lo);
    }
  }
  CS_IDLE;
}

void fillScreen(uint16_t color) {
  setAddrWindow(0, 0, _width - 1, _height - 1);
  flood(color, (long)_width * (long)_height);
}

void writeScreen(uint16_t color, int totalPix){
  flood(color, (long)totalPix);
}

void setCursor(int16_t x, int16_t y) {
  cursor_x = x;
  cursor_y = y;
}

void setTextSize(uint8_t s) {
  textsize = (s > 0) ? s : 1;
}

void setTextColor(uint16_t c) {
  // For 'transparent' background, we'll set the bg 
  // to the same as fg instead of using a flag
  textcolor = textbgcolor = c;
}

void setTextColor2(uint16_t c, uint16_t b) {
  textcolor   = c;
  textbgcolor = b; 
}



void lcd_init() {
  /*pin initialization*/
  csPort     = portOutputRegister(digitalPinToPort(LCD_CS));
  cdPort     = portOutputRegister(digitalPinToPort(LCD_CD));
  wrPort     = portOutputRegister(digitalPinToPort(LCD_WR));
  rdPort     = portOutputRegister(digitalPinToPort(LCD_RD));
  csPinSet   = digitalPinToBitMask(LCD_CS);
  cdPinSet   = digitalPinToBitMask(LCD_CD);
  wrPinSet   = digitalPinToBitMask(LCD_WR);
  rdPinSet   = digitalPinToBitMask(LCD_RD);
  csPinUnset = ~csPinSet;
  cdPinUnset = ~cdPinSet;
  wrPinUnset = ~wrPinSet;
  rdPinUnset = ~rdPinSet;
  *csPort   |=  csPinSet; // Set all control bits to HIGH (idle)
  *cdPort   |=  cdPinSet; // Signals are ACTIVE LOW
  *wrPort   |=  wrPinSet;
  *rdPort   |=  rdPinSet;
  pinMode(LCD_CS, OUTPUT);    // Enable outputs
  pinMode(LCD_CD, OUTPUT);
  pinMode(LCD_WR, OUTPUT);
  pinMode(LCD_RD, OUTPUT);
  //if(LCD_RESET){
  digitalWrite(LCD_RESET, HIGH);
  pinMode(LCD_RESET, OUTPUT);
  //}
  setWriteDirInline();

  /*graphics parameters*/
  rotation  = 0;
  cursor_y  = cursor_x = 0;
  textsize  = 1;
  textcolor = 0xFFFF;
  _width=240;
  _height=320;

  /*reset*/
  CS_IDLE;
  WR_IDLE;
  RD_IDLE;
  digitalWrite(LCD_RESET, LOW);
  delay(2);
  digitalWrite(LCD_RESET, HIGH);

  /*data transfer sync*/
  CS_ACTIVE;
  CD_COMMAND;
  write8inline(0x00);
  for(uint8_t i=0; i<3; i++) WR_STROBE;
  CS_IDLE;

  /*begin*/
  //another reset here?
  //delay(200);
  CS_ACTIVE;
  writeRegister8inline(ILI9341_SOFTRESET, 0);
  delay(50);
  writeRegister8inline(ILI9341_DISPLAYOFF, 0);

  writeRegister8inline(ILI9341_POWERCONTROL1, 0x23);
  writeRegister8inline(ILI9341_POWERCONTROL2, 0x10);
  writeRegister16inline(ILI9341_VCOMCONTROL1, 0x2B2B);
  writeRegister8inline(ILI9341_VCOMCONTROL2, 0xC0);
  writeRegister8inline(ILI9341_MEMCONTROL, ILI9341_MADCTL_MY | ILI9341_MADCTL_BGR);
  writeRegister8inline(ILI9341_PIXELFORMAT, 0x55);
  writeRegister16inline(ILI9341_FRAMECONTROL, 0x001B);
    
  writeRegister8inline(ILI9341_ENTRYMODE, 0x07);

  writeRegister8inline(ILI9341_SLEEPOUT, 0);
  delay(150);
  writeRegister8inline(ILI9341_DISPLAYON, 0);
  //delay(500);
  //setAddrWindow(0, 0, _width-1, _height-1);

}

void setup() {
  // put your setup code here, to run once:
  lcd_init();
  fillScreen(BLUE);
  setAddrWindow(119, 0, 119, 319);
  writeScreen(WHITE, 320); //a vertical line of 320 pixel (along height)
  setAddrWindow(120, 0, 120, 319);
  writeScreen(WHITE, 320); //a vertical line of 320 pixel (along height)
  setAddrWindow(0, 159, 239, 159);
  writeScreen(WHITE, 240); //a vertical line of 320 pixel (along height)
  setAddrWindow(0, 160, 239, 160);
  writeScreen(WHITE, 240); //a vertical line of 320 pixel (along height)
  setAddrWindow(59, 79, 60, 80); // a black dot with 4 pixels
  writeScreen(BLACK, 4);

}

void loop() {
  // put your main code here, to run repeatedly:
  while(1);

}

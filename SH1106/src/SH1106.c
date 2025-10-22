
#include "OLED.h"
#include "font.h"
void delay_ms(uint32_t ms)
{
    //use timer or systick based delay for accuracy.
    for(volatile uint32_t i = 0; i < ms * 4200; i++);
}

void I2C1_init(void)
{
    // Enable clock access to PORT B
    RCC->AHB1ENR |= GPIOBEN;

    // SET PB8 AND PB9 to alternate function
    GPIOB->MODER &= ~(1U<<16);
    GPIOB->MODER |= (1U<<17);
    GPIOB->MODER &= ~(1U<<18);
    GPIOB->MODER |= (1U<<19);

    // SET OUTTYPE AS OPEN DRAIN
    GPIOB->OTYPER |= (1U<<8);
    GPIOB->OTYPER |= (1U<<9);

    // SET PULLUP FOR BOTH PB8 AND PB9
    GPIOB->PUPDR |= (1U<<16);
    GPIOB->PUPDR &= ~(1U<<17);
    GPIOB->PUPDR |= (1U<<18);
    GPIOB->PUPDR &= ~(1U<<19);

    // SET THE ALTERNATE FUNCTION TYPE FOR I2C (AF4)
    GPIOB->AFR[1] &= ~(0xF<<0);  
    GPIOB->AFR[1] |= (0x4<<0);   
    GPIOB->AFR[1] &= ~(0xF<<4); 
    GPIOB->AFR[1] |= (0x4<<4);  

    // Enable I2C1 clock
    RCC->APB1ENR |= I2C1EN;

    //Disable the I2C peripheral
    I2C1->CR1 &=~I2C_CR1_PE;

    // Set APB1 frequency to 16 MHz(May change based on prescaler and HCLK)
    I2C1->CR2 = (16 & 0x3F);

    // Set for 400 kHz Fast Mode 
    I2C1->CCR = (1<<15)|14;

    //  Set for 300 ns max rise time 
    I2C1->TRISE = 6;

    //Enable the I2C peripheral
    I2C1->CR1 |= I2C_CR1_PE;


}

void i2cWriteByte(uint8_t addr, uint8_t control, uint8_t value)
{
    volatile int tmp;

    while(I2C1->SR2 & SR2_BUSY){}
    I2C1->CR1 |= CR1_START;
    while(!(I2C1->SR1 & SR1_SB)){}

    I2C1->DR = addr << 1;
    while(!(I2C1->SR1 & SR1_ADDR)){}
    tmp = I2C1->SR2;

    while(!(I2C1->SR1 & SR1_TXE)){}
    I2C1->DR = control;
    while(!(I2C1->SR1 & SR1_TXE)){}
    I2C1->DR = value;
    while(!(I2C1->SR1 & SR1_BTF)){}

    I2C1->CR1 |= CR1_STOP;
}

void i2cBurstWrite(char saddr, int n, char* data)
{
    volatile int tmp;

    while(I2C1->SR2 & SR2_BUSY){}
    I2C1->CR1 |= CR1_START;
    while(!(I2C1->SR1 & SR1_SB)){}

    I2C1->DR = saddr << 1;
    while(!(I2C1->SR1 & SR1_ADDR)){}
    tmp = I2C1->SR2;

    while(!(I2C1->SR1 & SR1_TXE)){}

    for(int i = 0; i < n; i++)
    {
        while(!(I2C1->SR1 & SR1_TXE)){}
        I2C1->DR = *data++;
    }

    while(!(I2C1->SR1 & SR1_BTF)){}
    I2C1->CR1 |= CR1_STOP;
}

//use i2c_scanner if you dont know i2c address of SH1106
void i2c_scanner(void)
{
    printf("Scanning I2C bus...\n");
    for(uint8_t i = 1; i < 127; i++)
    {
        volatile int tmp;

        while(I2C1->SR2 & SR2_BUSY){}
        I2C1->CR1 |= CR1_START;
        while(!(I2C1->SR1 & SR1_SB)){}

        I2C1->DR = i << 1;

        volatile int timeout = 10000;
        while(!((I2C1->SR1 & SR1_ADDR) || (I2C1->SR1 & (1U<<10))))
        {
            if(--timeout == 0)
                break;
        }

        if(I2C1->SR1 & SR1_ADDR)
        {
            printf("Device found at: 0x%02X\n", i);
            tmp = I2C1->SR1;
            tmp = I2C1->SR2;
        }
        else if(I2C1->SR1 & (1U<<10))
        {
            I2C1->SR1 &= ~(1U<<10);
        }

        I2C1->CR1 |= CR1_STOP;
        for(volatile int t = 0; t < 1000; t++);
    }
    printf("Scan complete.\n");
}

// SH1106 initialization sequence
uint8_t sh1106_init_seq[] = {
    0xAE,       // Display OFF
    0xD5, 0x80, // Clock divide ratio (default)
    0xA8, 0x3F, // Multiplex ratio (64)
    0xD3, 0x00, // Display offset = 0
    0x40,       // Start line = 0
    0x8D, 0x14, // Charge pump ON (important!)
    0xA1,       // Segment remap (mirror horizontally)
    0xC8,       // COM scan direction (flip vertically)
    0xDA, 0x12, // COM pins config
    0x81, 0x7F, // Contrast (medium value)
    0xD9, 0xF1, // Pre-charge period (fixed!)
    0xDB, 0x40, // VCOMH deselect level
    0xA4,       // Entire display ON (follow RAM)
    0xA6,       // Normal display (not inverted)
    0xAF        // Display ON
};

//write a command
void cmdwrite(uint8_t cmd)
{
    i2cWriteByte(SH1106_ADDR, 0x00, cmd);
}

//write data
void datawrite(uint8_t data)
{
    i2cWriteByte(SH1106_ADDR, 0x40, data);
}

//initialize SH1106
void SH1106_init(void)
{
    delay_ms(100); 

    for (uint8_t i = 0; i < sizeof(sh1106_init_seq); i++)
    {
        cmdwrite(sh1106_init_seq[i]);
        delay_ms(1); 
    }

    delay_ms(100); 
}



// clear display
void clear(void)
{
    for (uint8_t page = 0; page < 8; page++)
    {
        cmdwrite(0xB0 + page);
        cmdwrite(0x00); 
        cmdwrite(0x10); 

        
        uint8_t buffer[133];
        buffer[0] = 0x40;  
        for (int i = 1; i < 133; i++)
        {
            buffer[i] = 0x00;
        }

        i2cBurstWrite(SH1106_ADDR, 133, (char*)buffer);
        delay_ms(5);
    }
}


// Make all pixels high
void fill(void)
{
    for (uint8_t page = 0; page < 8; page++)
    {
        
        cmdwrite(0xB0 + page);

        
        cmdwrite(0x00); 
        cmdwrite(0x10);  

    
        for (uint8_t col = 0; col < 132; col++)
        {
            datawrite(0xFF);
        }

        delay_ms(2); 
    }
}



// Set cursor position (x: 0-127, page: 0-7)
void setcursor(uint8_t x, uint8_t page)
{
   
    x += 2;

    cmdwrite(0xB0 + page);             
    cmdwrite(0x00 + (x & 0x0F));        
    cmdwrite(0x10 + ((x >> 4) & 0x0F));
}

//write a single character to display
void writechar(char ch, uint8_t page, uint8_t x)
{
    if (ch < 32 || ch > 126) ch = ' ';
    uint8_t char_index = ch - 32;

        uint8_t font_width = 8;

        for (uint8_t col = 0; col < font_width; col++)
        {
            if (page > 7) continue;
            setcursor(x + col, page);
            datawrite(font8x8_vertical[char_index][col]);
        }


}

//bitmap display function
void drawBitmap(uint8_t x, uint8_t page, uint8_t width, uint8_t height, const uint8_t *bitmap)
{
    uint8_t pages = (height + 7) / 8;

    for (uint8_t col = 0; col < width; col++)
    {
        for (uint8_t p = 0; p < pages; p++)
        {
            uint8_t byte_out = 0;

          
            for (uint8_t bit = 0; bit < 8; bit++)
            {
                uint8_t y = p * 8 + bit;
                if (y >= height) break;

          
                uint16_t row_byte_index = (y * ((width + 7) / 8)) + (col / 8);
                uint8_t pixel_byte = bitmap[row_byte_index];
                uint8_t mask = 1 << (7 - (col % 8));

                if (pixel_byte & mask)
                    byte_out |= (1 << bit);
            }

            setcursor(x + col, page + p);
            datawrite(byte_out);
        }
    }
}


// Write string at specified position
void write(uint8_t x, uint8_t page, const char* str)
{

    while (*str)
    {
        writechar(*str, page, x);  
        x += 8;

        if (x > 116)
            break;

        str++;
    }
}

// draw horixontal line from x,y and of a certain height
void drawline_H(uint8_t x, uint8_t y, uint8_t width)
{
    if (width == 0) return;

    uint8_t page = y / 8;
    uint8_t bit = 1 << (y % 8);

    setcursor(x, page);
    for (uint8_t i = 0; i < width; i++)
    {
        datawrite(bit);
    }
}


// draw vertical line from x,y and of a certain height
void drawline_V(uint8_t x, uint8_t y, uint8_t height)
{
    if (height == 0) return;

    uint8_t start_page = y / 8;
    uint8_t end_page = (y + height - 1) / 8;

    for (uint8_t page = start_page; page <= end_page && page < 8; page++)
    {
        setcursor(x, page);

        uint8_t data;

        if (start_page == end_page) {
            uint8_t start_bit = y % 8;
            uint8_t end_bit = (y + height - 1) % 8;
            data = ((0xFF << start_bit) & (0xFF >> (7 - end_bit)));
        } else if (page == start_page) {
            uint8_t start_bit = y % 8;
            data = (0xFF << start_bit);
        } else if (page == end_page) {
            uint8_t end_bit = (y + height - 1) % 8;
            data = (0xFF >> (7 - end_bit));
        } else {
            data = 0xFF;
        }

        datawrite(data);
    }
}


//draw a rectangle of specific height and width at x,y
void rect(uint8_t x, uint8_t y, uint8_t width, uint8_t height)
{
    if (width == 0 || height == 0) return;

    drawline_H(x, y, width);                    // Top
    drawline_H(x, y + height - 1, width);       // Bottom
    drawline_V(x, y, height);                   // Left
    drawline_V(x + width - 1, y, height);       // Right
}





// Draw arrow of height and width 8 pixels
void draw_arrow(uint8_t x ,uint8_t y)

{   char buffer[]={0x3c,0x3c,0x3c,0x3c,0xff,0x7e,0x3c,0x18};
	setcursor(x,y);
	    for (uint8_t i = 0; i < 8; i++)
	    {
	        datawrite(buffer[i]);
	    }

}

// Invert a rectangle of specific height and width at x,y
void invert_rect(uint8_t x, uint8_t y, uint8_t width, uint8_t height)
{
    if(width == 0 || height == 0) return;

    uint8_t start_page = y / 8;
    uint8_t end_page   = (y + height - 1) / 8;

    uint8_t first_mask = 0xFF << (y % 8);
    uint8_t last_mask  = 0xFF >> (8 - ((y + height) % 8));

    for(uint8_t page = start_page; page <= end_page && page < 8; page++)
    {
        setcursor(x, page);

        for(uint8_t i = 0; i < width; i++)
        {
            uint8_t data = 0xFF; 

            if(page == start_page) data &= first_mask; 
            if(page == end_page)   data &= last_mask;  

            datawrite(data); 
        }
    }


}




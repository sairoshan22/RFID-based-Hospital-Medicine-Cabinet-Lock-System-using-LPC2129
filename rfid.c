#include <lpc21xx.h>

/* LCD Pin Definitions (Connected to Port 0) */
#define RS   (1 << 10)  // P0.10 -> Register Select
#define EN   (1 << 12)  // P0.12 -> Enable
#define D4   (1 << 16)  // P0.16 -> Data Bit 4
#define D5   (1 << 17)  // P0.17 -> Data Bit 5
#define D6   (1 << 18)  // P0.18 -> Data Bit 6
#define D7   (1 << 19)  // P0.19 -> Data Bit 7

/* Peripherals Definitions */
#define mf   (1 << 8)   // P0.8  -> Motor Forward (Door Open)
#define mr   (1 << 4)   // P0.4  -> Motor Reverse (Door Close)
#define SW   5          // P0.5  -> Push Button (Door Shut Trigger)
#define LED1 (1 << 6)   // P0.6  -> Status LED
#define buzz (1 << 7)   // P0.7  -> Active Buzzer

void delay_ms(unsigned int count) {
    unsigned int i, j;
    for (i = 0; i < count; i++)
        for (j = 0; j < 6000; j++);
}

void LCD_Command(char cmd) {
    IO0CLR = RS; // RS = 0 for command
    
    // Send higher nibble
    IO0CLR = (D4 | D5 | D6 | D7);
    if (cmd & 0x10) IO0SET = D4;
    if (cmd & 0x20) IO0SET = D5;
    if (cmd & 0x40) IO0SET = D6;
    if (cmd & 0x80) IO0SET = D7;
    
    IO0SET = EN; delay_ms(1); IO0CLR = EN;

    // Send lower nibble
    IO0CLR = (D4 | D5 | D6 | D7);
    if (cmd & 0x01) IO0SET = D4;
    if (cmd & 0x02) IO0SET = D5;
    if (cmd & 0x04) IO0SET = D6;
    if (cmd & 0x08) IO0SET = D7;
    
    IO0SET = EN; delay_ms(1); IO0CLR = EN;
}

void LCD_Char(char data) {
    IO0SET = RS; // RS = 1 for data
    
    // Send higher nibble
    IO0CLR = (D4 | D5 | D6 | D7);
    if (data & 0x10) IO0SET = D4;
    if (data & 0x20) IO0SET = D5;
    if (data & 0x40) IO0SET = D6;
    if (data & 0x80) IO0SET = D7;
    
    IO0SET = EN; delay_ms(1); IO0CLR = EN;

    // Send lower nibble
    IO0CLR = (D4 | D5 | D6 | D7);
    if (data & 0x01) IO0SET = D4;
    if (data & 0x02) IO0SET = D5;
    if (data & 0x04) IO0SET = D6;
    if (data & 0x08) IO0SET = D7;
    
    IO0SET = EN; delay_ms(1); IO0CLR = EN;
}

void LCD_Init(void) {
    IO0DIR |= (RS | buzz | EN | D4 | D5 | D6 | D7 | mf | mr | LED1);
    IOSET0 = buzz | LED1; // Keep buzzer and LED inactive at startup
    delay_ms(20);
    
    LCD_Command(0x02); // Initialize LCD in 4-bit mode
    LCD_Command(0x28); // 2 lines, 5x7 matrix
    LCD_Command(0x0C); // Display ON, Cursor OFF
    LCD_Command(0x06); // Auto increment cursor
    LCD_Command(0x01); // Clear display
    delay_ms(5);
}

void LCD_String(char *str) {
    while (*str) {
        LCD_Char(*str++);
    }
}

void UART0_Init(void) {
    PINSEL0 |= 0x00000005;  // P0.0 = TXD0, P0.1 = RXD0
    U0LCR = 0x83;           // 8 bits, no parity, 1 stop bit, DLAB = 1
    U0DLL = 97;             // Baud rate 9600 (Assuming 15MHz PCLK)
    U0DLM = 0;
    U0LCR = 0x03;           // DLAB = 0
    U0FCR = 0x07;           // Enable and clear FIFO
}

char UART0_ReadChar(void) {
    while (!(U0LSR & 0x01)); // Wait for data reception
    return U0RBR;
}

int main(void) {
    char rfid_settag[13] = "06003B4E2E";//Enter your valid id number
    char rfid_tag[13];
    int i, j;
    int flag;

    LCD_Init();
    UART0_Init();

    LCD_String("RFID Reader Ready");
    delay_ms(5000);
    LCD_Command(0x01); // Clear screen

    while (1) {
        // Read the incoming 12-byte stream from EM-18 RFID Reader
        for (i = 0; i < 12; i++) {
            rfid_tag[i] = UART0_ReadChar();
        }
        rfid_tag[12] = '\0'; // Null-terminate string

        // Display Tag ID Header on LCD
        LCD_Command(0x01);
        delay_ms(2);
        LCD_String("UID Detected:");
        LCD_Command(0xC0);          // Move cursor to 2nd row
        
        // Truncate to core 10 digits for clean string comparison
        rfid_tag[10] = '\0'; 
        
        // Print the scanned tag on the LCD second row
        LCD_String(rfid_tag);
        delay_ms(2000);

        // Reset flag for the new scan
        flag = 1;

        // Compare the 10-digit ID character by character
        for (j = 0; j < 10; j++) {
            if (rfid_settag[j] != rfid_tag[j]) {
                flag = 0;
                break;
            }
        }

        // Evaluate access outcome
        if (flag == 1) {
            LCD_Command(0x01);
            delay_ms(2);
            LCD_String("VALID USER:");
            LCD_Command(0xC0);
            LCD_String("DOOR OPEN");
            
            // Open door (Motor Forward)
            IOSET0 = mf;
            IOCLR0 = mr;
            delay_ms(5000);
            IOCLR0 = mf | mr;
            
            // Prompt user to shut door
            LCD_Command(0x01);
            delay_ms(2);
            LCD_String("PRESS S1 TO SHUT");
            
            while(1) {
                if (((IOPIN0 >> SW) & 1) == 0) {
                    LCD_Command(0x01);
                    delay_ms(2);
                    LCD_String("DOOR CLOSING");

                    // Close door (Motor Reverse)
                    IOSET0 = mr;
                    IOCLR0 = mf;
                    delay_ms(5000);
                    IOCLR0 = mr | mf;
                    break;
                }
            }

        } else {
            // Invalid User Sequence
            LCD_Command(0x01);
            delay_ms(2);
            LCD_String("INVALID USER:");
            LCD_Command(0xC0);          // Move cursor to Line 2
            LCD_String("ACCESS DENIED");
            
            // Turn on Buzzer and LED simultaneously for 2 seconds
            IOCLR0 = buzz | LED1;       // Active-low alert activation
            delay_ms(2000);
            IOSET0 = buzz | LED1;       // Turn off alert
        }

        delay_ms(1000); // Hold before next scan cycle
    }
}

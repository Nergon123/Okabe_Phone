/*
*FROM NERGON:
*THIS CONFIG IS FROM TFT_eSPI LIBRARY BY Bodmer
*/
#undef USER_SETUP_INFO
#define USER_SETUP_INFO "TFT_CONFIG"

#ifndef ILI9341_DRIVER
#define ILI9341_DRIVER       
#endif

#undef TFT_MISO 
#undef TFT_MOSI 
#undef TFT_SCLK 
#undef TFT_CS     
#undef TFT_DC     
#undef TFT_RST    
#undef TFT_BL     

#define TFT_MISO 12
#define TFT_MOSI 23
#define TFT_SCLK 18
#define TFT_CS   27  
#define TFT_DC   32  
#define TFT_RST  -1  
#define TFT_BL   04  





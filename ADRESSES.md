
# OFFSET

## WALLPAPERS

 WALLPAPER PLACEMENT IS 0,27

0_0 - 5_6

42 files

SIZE OF EVERY FILE IS 0x22740
RESOLUTION IS 240x294


START AT 0xD
END AT 0x5A708D

# STATUSBAR, SIGNAL AND BATTERY

## STATUSBAR PLACEMENT IS 0,0
  RESOLUTION IS 240X26
  SIZE 0x30C0
  LOCATION 0X5A708E

## BATTERY PLACEMENT IS 207,0
  0/3,1/3,2/3,3/3
  SIZE 0x6B4
  LOCATIONS 0X5AA14E (-1?),+0x6B4,+0x6B4,+0x6B4
  COUNT 4
  RESOLUTION IS 33X26

## SIGNAL PLACEMENT IS 0,0
  RESOLUTION IS 30X26
  SIZE 0x618
  LOCATIONS 0x5ABC1E
  COUNT 4


## NO SIGNAL PLACEMENT IS 0,0
  RESOLUTION IS 37X26
  SIZE 0x784
  LOCATION 0x5AD47D

## MENU BACKGROUND PLACEMENT IS 0,27
  RESOLUTION IS 240x294
  SIZE 0x22740
  LOCATION 0x5ADC01

## MENU WITHOUT ITEMS PLACEMENT IS 0,27
  RESOLUTION IS 240x294
  LOCATION 0x613D45

## MENU ITEMS PLACEMENT

  turned off icons
  size of each is +(0x17A2*1)
  beginning at 0x5D5097
  first is 55x55, 49,(43+26)
  drawFromSd(0x5D5097, 49,(43+26),55,55);
  second is 55x55 138,(42+26)
  drawFromSd(0x5D5097+(0x17A2*1),138,(42+26),55,55);
  third is 55x55 49,(122+26)
  drawFromSd(0x5D5097+(0x17A2*2),49,(122+26),55,55);
  fourth 55x55 137,(123+26)
  drawFromSd(0x5D5097+(0x17A2*3),137,(123+26),55,55);

  on icons
  first icon is 49x49, 51,(45+26)
  drawFromSd(0x5D0342,51,(45+26),49,49);
  0x5D0342
  second is 49x51, 141,(44+26)
  drawFromSd(0x5D1603,141,(44+26),49,51);
  0x5D1603
  third is 50x50,52,(125+26)
  drawFromSd(0x5D2989,52,(125+26),50,50);
  0x5D2989
  fourth is 49x51, 140,(125+26)
  drawFromSd(0x5D3D11,140,(125+26),49,51);
  0x5D3D11

# UI other
##  bluebar placement at 0,26
  size is 240,25
  beginning at 0x5DAF1F
  icons for bluebar is 25,25 0x5DDDFF
    messages, contacts, settings
  every is 0x4E2

##  mail(messages) header placement at 0,26
  size is 240,42
  location at 0x5DECA5

##  settings header placement at 0,26
  size is 240,42
  location at 0x5E3B65

##  white bottom (for headers) placement 0,68
  size 240,128
  location is at 0x5E8A25

##  Full screen notification placement 0,67
  size 240,134
  location is at 0x5F7A25

##  Context menu placement 16,100
  size 208,123
  location is at 0x607565
  chroma-key is 0x07e0

##  background placement at 0,26
  location at 0x636485
  size 240,294

##  DOTS
  COUNT 8
  location 0x658BC5
  size 7,7 , 0x62

##  NUMBER FONT WHITE
  COUNT 15
  location 0x658ED6
  size 21,27

##  VOICE ONLY LABEL
  location 0x65D147
  size 160,34

##  NUMBER FONT BLACK
  COUNT 14
  location 0x65FBC7
  BACKGROUND 0xFFFF
  size 15,19

##  PHONE
  LOCATION 0x661AF3
  BACKGROUND  0xBAD6
  SIZE 42,50
##    LIGHTNING ANIMATION
      LOCATION 0x662B5B 
      SIZE 13,14
      COUNT 2

## MAIL ICONS
   LOCATION 0x662DB1
    BACKGROUND 0x0000
    SIZE 18,21
    COUNT 4


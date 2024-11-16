#include "render.h"
void changeFont(int ch)
{
  switch (ch)
  {
  case 0:
    tft.setTextFont(1);
    break;
  case 1:
    tft.setFreeFont(&FreeSans9pt7b);
    break;
  case 2:
    tft.setFreeFont(&FreeSansBold9pt7b);
    break;
  case 3:
    tft.setFreeFont(&FreeMono9pt7b);
    break;
  case 4:
    tft.setFreeFont(&FreeSans12pt7b);
    break;
  }
}

void drawFromSdDownscale(uint32_t pos, int pos_x, int pos_y, int size_x, int size_y, int scale, File file)
{
  //PARTIALLY STOLEN FROM CHATGPT
  if (!file.available())
    sysError("SD_CARD_NOT_AVAILABLE");
  file.seek(pos);

  const int downscaled_width = size_x / scale;
  const int buffer_size = size_x * 2; // 2 bytes per pixel
  uint8_t buffer[buffer_size];

  for (int a = 0; a < size_y; a += scale)
  {
    file.read(buffer, buffer_size);

    uint16_t line[downscaled_width];
    for (int i = 0; i < downscaled_width; i++)
    {
      line[i] = (buffer[2 * i * scale] << 8) | buffer[2 * i * scale + 1];
    }

    tft.pushImage(pos_x, pos_y + (a / scale), downscaled_width, 1, line);

    // Skip the lines that won't be drawn (vertical downscaling)
    file.seek(file.position() + (size_x * 2 * (scale - 1)));
  }
  file.close();
}

void drawStatusBar()
{

  drawFromSd(0X5A708D, 0, 0, 240, 26); // statusbar
  if (getSignalLevel() == 1)
    drawFromSd(0x5AD47D, 0, 0, 37, 26);                              // no_signal
  drawFromSd(0x5ABC1D + (0x618) * getSignalLevel(), 0, 0, 30, 26);   // signal
  drawFromSd(0X5AA14D + (0x6B4) * getChargeLevel(), 207, 0, 33, 26); // battery
  //  tft.print(String(charge) + String("%"));
}

void rendermenu(int choice, bool right)
{

  switch (choice)
  {
  case 0:
    if (right)
    {
      drawFromSd(0x5D5097 + (0x17A2 * 3), 137, (123 + 26), 55, 55);
    }
    else
    {
      drawFromSd(0x5D5097 + (0x17A2 * 1), 138, (42 + 26), 55, 55);
    }
    drawFromSd(0x5D0341, 51, (45 + 26), 49, 49);
    break;
  case 1:
    if (right)
    {
      drawFromSd(0x5D5097, 49, (43 + 26), 55, 55);
    }
    else
    {
      drawFromSd(0x5D5097 + (0x17A2 * 2), 49, (122 + 26), 55, 55);
    }
    drawFromSd(0x5D1603, 141, (44 + 26), 49, 51);
    break;
  case 2:
    if (right)
    {
      drawFromSd(0x5D5097 + (0x17A2 * 1), 138, (42 + 26), 55, 55);
    }
    else
    {
      drawFromSd(0x5D5097 + (0x17A2 * 3), 137, (123 + 26), 55, 55);
    }
    drawFromSd(0x5D2989, 52, (125 + 26), 50, 50);
    break;
  case 3:
    if (right)
    {
      drawFromSd(0x5D5097 + (0x17A2 * 2), 49, (122 + 26), 55, 55);
    }
    else
    {
      drawFromSd(0x5D5097, 49, (43 + 26), 55, 55);
    }
    drawFromSd(0x5D3D11, 140, (125 + 26), 49, 51);
    break;
  }
}
void sysError(const char *reason)
{

  tft.fillScreen(0x0000);
  tft.setCursor(10, 40);
  tft.setTextFont(1);
  tft.setTextSize(4);
  tft.setTextColor(0xF001); // FOOL :3
  tft.println("==ERROR==");
  tft.setTextColor(0xFFFF);
  tft.setTextSize(1);
  tft.println(String("\n\n\nThere a problem with your device\nYou can fix it by yourself i guess\nThere some details for you:\n\n\nReason:" + String(reason)));
  for(;;);
}

void drawFromSd(uint32_t pos, int pos_x, int pos_y, int size_x, int size_y, File file, bool transp, uint16_t tc)
{
  if (!file.available())
    sysError("SD_CARD_NOT_AVAILABLE");
  file.seek(pos);
  if (!transp)
  {
    const int buffer_size = size_x * 2;
    uint8_t buffer[buffer_size];

    for (int a = 0; a < size_y; a++)
    {
      file.read(buffer, buffer_size);

      tft.pushImage(pos_x, a + pos_y, size_x, 1, (uint16_t *)buffer);
    }
  }
  else
  {
    const int buffer_size = size_x * 2; // 2 bytes per pixel
    uint8_t buffer[buffer_size];
    for (int a = 0; a < size_y; a++)
    {
      // Read a whole line (row) of pixels at once
      file.read(buffer, buffer_size);

      int draw_start = -1;
      for (int i = 0; i < size_x; i++)
      {
        uint16_t wd = (buffer[2 * i + 1] << 8) | buffer[2 * i];


        if (wd != tc)
        {

          if (draw_start == -1)
          {
            draw_start = i;
          }
        }
        else
        {
          if (draw_start != -1)
          {
            tft.pushImage(pos_x + draw_start, a + pos_y, i - draw_start, 1, (uint16_t *)(&buffer[draw_start]));
            draw_start = -1;
          }
        }
      }
      Serial.println();
      if (draw_start != -1)
      {
        tft.pushImage(pos_x + draw_start, a + pos_y, size_x - draw_start, 1, (uint16_t *)(&buffer[draw_start]));
      }
    }
  }
}

void drawFromSd(uint32_t pos, int pos_x, int pos_y, int size_x, int size_y, bool transp, uint16_t tc)
{
  drawFromSd(pos, pos_x, pos_y, size_x, size_y, SD.open("/FIRMWARE/IMAGES.SG"), transp, tc);
}

void drawFromSd(int x, int y, SDImage sprite)
{
  drawFromSd(sprite.address, x, y, sprite.w, sprite.h, sprite.transp, sprite.tc);
}
void writeCustomFont(int x, int y, String input, int type)
{
  int w = 21;
  int h = 27;
  bool transp = false;
  uint16_t tc = 0xFFFF;
  uint32_t addr = 0x658ED5;
  switch (type)
  {
  case 0:
    break;
  case 1:
    w = 15;
    h = 19;
    addr = 0x65FBC7;
    transp = true;
    break;
  default:
    break;
  }
  int size = w * h * 2;

  char buf[64];
  input.toCharArray(buf, 64);
  for (int i = 1; i < input.length() + 1; i++)
  {
    Serial.println(String(buf[i - 1]) + " " + String(w * (i - 1) + x));
    switch (buf[i - 1])
    {
    case '#':
      drawFromSd(addr + (size * 10), w * (i - 1) + x, y, w, h, transp, tc);
      break;
    case '*':
      drawFromSd(addr + (size * 11), w * (i - 1) + x, y, w, h, transp, tc);
      break;
    case 'X':
      drawFromSd(addr + (size * 12), w * (i - 1) + x, y, w, h, transp, tc);
      break;
    case '/':
      drawFromSd(addr + (size * 13), w * (i - 1) + x, y, w, h, transp, tc);
      break;
    case ':':
      drawFromSd(addr + (size * 14), w * (i - 1) + x, y, w, h, transp, tc);
      break;
    default:
      if (buf[i - 1] > 47 && buf[i - 1] < 58)
        drawFromSd(addr + (size * (buf[i - 1] - 48)), w * (i - 1) + x, y, w, h, transp, tc);
      break;
    }
  }
}

void listMenu_sub(String label, int type, int page, int pages)
{

  tft.setCursor(30, 45);
  tft.setTextSize(1);
  changeFont(1);
  tft.setTextColor(0xffff);
  tft.print(label);
  changeFont(0);
  tft.setCursor(210, 40);
  tft.print(String(page + 1) + String("/") + String(pages + 1));
  changeFont(1);
}

int listMenu(mOption *choices, int icount, bool images, int type, String label)
{
  tft.setTextWrap(false, false);
  /*


  int icount
  I know I just can count here with ArraySize but whatever
  Length counts wrong with causes OutOfBounds panic thing
  This is Application for ESP32 writed within arduino environment

  label = Title of Menu

  bool images = Is it images? (Is it wallpaper choice will be more correct)

  int type
  0 = MESSAGES
  1 = CONTACTS
  2 = SETTINGS

  */
 bool empty = false;
  if (icount == 0)
    empty= true;

  // load file with graphical resources
  int scale = 7; // downscale multiplier for images
  int x = 10;    // coordinates where begin to render text
  int y = 65;
  int mult = 20;
  int icon_x = 2;
  if (images)
  {
    mult = (294 / scale) + 1;
    x = 55;
  }
  else if (choices[0].icon.address != 0)
  {
    x = 29;
  }
  drawFromSd(0x5DAF1F, 0, 26, 240, 25); // bluebar TODO: just draw rectangle instead of loading a whole image
  if (type >= 0 && type < 3)            // with icon to draw...
    drawFromSd(0x5DDDFF + (0x4E2 * type), 0, 26, 25, 25);
  int items_per_page = 269 / mult; // max items per page (empty space height divided by spacing)
  int count = icount;
  int pages = (icount + items_per_page - 1) / items_per_page - 1;
  int page = 0;
  int icon_addr;

  uint16_t color_active = 0xFDD3;
  uint16_t color_inactive = 0x0000;
  int choice = 0;

  tft.setTextSize(1);
  tft.setTextColor(color_inactive);
  changeFont(1);

  listMenu_sub(label, type, page, pages);
  drawFromSd(0x639365, 0, 51, 240, 269);
  tft.setTextColor(color_inactive);
  if(empty){
  tft.setCursor(75,70);
  tft.print("< Empty >");
  while(buttonsHelding()==-1);
  return -1;
  }
  for (int i = 0; i < items_per_page && items_per_page * page + i < icount; i++)
  {
    if (images)
    {

      tft.drawLine(0, 50 + mult * (i + 1), 240, 50 + mult * (i + 1), 0x0000);
      drawFromSdDownscale((uint32_t)(0xE) + ((uint32_t)(0x22740) * (items_per_page * page + i)), 10, 51 + mult * i, 240, 294, scale);
    }
    else if (choices[i].icon.address != 0)
    {
      drawFromSd(icon_x, 51 + mult * i, choices[i].icon);
    }
    tft.setCursor(x, y + (mult * i));
    tft.print(choices[items_per_page * page + i].label);
  }
  tft.fillRect(0, 51 + mult * choice, 240, mult, color_active);
  tft.setCursor(x, y + (mult * choice));
  tft.print(choices[items_per_page * page + choice].label);
  if (images)
  {
    tft.drawLine(0, 50 + mult * (choice + 1), 240, 50 + mult * (choice + 1), 0x0000);
    drawFromSdDownscale((uint32_t)(0xE) + ((uint32_t)(0x22740) * (items_per_page * page + choice)), 10, 51 + mult * choice, 240, 294, scale);
  }
  if (choices[items_per_page * page + choice].icon.address != 0)
  {
    drawFromSd(icon_x, 51 + mult * choice, choices[items_per_page * page + choice].icon);
  }
  bool exit = false;
  while (!exit)
    switch (buttonsHelding())
    {
    case SELECT:
    {
      exit = true;
      
      return items_per_page * page + choice;
      break;
    }
    case UP:
    { // Up button
      bool changed = false;
      int old_choice = choice;
      if (choice > 0)
      {
        choice--;
      }
      else if (page > 0)
      {
        drawFromSd(0x5DAF1F, 0, 26, 240, 25);
        if (type >= 0 && type < 3)
          drawFromSd(0x5DDDFF + (0x4E2 * type), 0, 26, 25, 25);
        page--;
        changed = true;
        drawFromSd(0x639365, 0, 51, 240, 269);
        for (int i = 0; i < items_per_page && items_per_page * page + i < icount; i++)
        {
          if (images)
          {
            tft.drawLine(0, 50 + mult * (i + 1), 240, 50 + mult * (i + 1), 0x0000);
            tft.drawLine(0, 50 + mult * (i), 240, 50 + mult * (i), 0x0000);
            drawFromSdDownscale((uint32_t)(0xE) + ((uint32_t)(0x22740) * (items_per_page * page + i)), 10, 51 + mult * i, 240, 294, scale);
          }
          if (choices[items_per_page * page + i].icon.address != 0)
          {
            drawFromSd(icon_x, 51 + mult * i, choices[items_per_page * page + i].icon);
          }
          tft.setCursor(x, y + (mult * i));
          tft.print(choices[items_per_page * page + i].label);
        }
        choice = (items_per_page - 1);
      }
      else
      {
        drawFromSd(0x5DAF1F, 0, 26, 240, 25);
        if (type >= 0 && type < 3)
          drawFromSd(0x5DDDFF + (0x4E2 * type), 0, 26, 25, 25);
        page = pages;
        changed = true;
        drawFromSd(0x639365, 0, 51, 240, 269);
        for (int i = 0; i < items_per_page && items_per_page * page + i < icount; i++)
        {
          if (images)
          {
            tft.drawLine(0, 50 + mult * (i + 1), 240, 50 + mult * (i + 1), 0x0000);
            drawFromSdDownscale((uint32_t)(0xE) + ((uint32_t)(0x22740) * (items_per_page * page + i)), 10, 51 + mult * i, 240, 294, scale);
          }
          if (choices[items_per_page * page + i].icon.address != 0)
          {
            drawFromSd(icon_x, 51 + mult * i, choices[items_per_page * page + i].icon);
          }
          tft.setCursor(x, y + (mult * i));
          tft.print(choices[items_per_page * page + i].label);
        }
        choice = (icount % items_per_page) ? (icount % items_per_page) - 1 : (items_per_page - 1);
      }

      if (!changed)
        drawFromSd(0x639365 + (mult * old_choice * 240 * 2), 0, 51 + (mult * old_choice), 240, mult);

      listMenu_sub(label, type, page, pages);
      tft.setTextColor(color_inactive);
      tft.setCursor(x, y + (mult * old_choice));
      if (!changed)
      {
        if (images)
        {
          tft.drawLine(0, 50 + mult * (choice + 2), 240, 50 + mult * (choice + 2), 0x0000);
          drawFromSdDownscale((uint32_t)(0xE) + ((uint32_t)(0x22740) * (items_per_page * page + (old_choice))), 10, 51 + mult * (old_choice), 240, 294, scale);
        }
        if (choices[items_per_page * page + old_choice].icon.address != 0)
        {
          drawFromSd(icon_x, 51 + mult * (old_choice), choices[items_per_page * page + old_choice].icon);
        }
        tft.print(choices[items_per_page * page + old_choice].label);
      }

      tft.fillRect(0, 51 + mult * choice, 240, mult, color_active);
      tft.setCursor(x, y + (mult * choice));
      tft.print(choices[items_per_page * page + choice].label);
      if (images)
      {
        tft.drawLine(0, 50 + mult * (choice + 1), 240, 50 + mult * (choice + 1), 0x0000);
        drawFromSdDownscale((uint32_t)(0xE) + ((uint32_t)(0x22740) * (items_per_page * page + choice)), 10, 51 + mult * choice, 240, 294, scale);
      }
      if (choices[items_per_page * page + choice].icon.address != 0)
      {
        drawFromSd(icon_x, 51 + mult * choice, choices[items_per_page * page + choice].icon);
      }
      break;
    }
    case DOWN:
    { // Down button
      int old_choice = choice;
      bool changed = false;
      if (choice < (items_per_page - 1) && items_per_page * page + choice < icount - 1)
      {
        choice++;
      }
      else if (page < pages)
      {
        page++;
        changed = true;
        choice = 0;
        drawFromSd(0x639365, 0, 51, 240, 269);
        for (int i = 0; i < items_per_page && items_per_page * page + i < icount; i++)
        {
          if (images)
          {
            tft.drawLine(0, 50 + mult * (i + 1), 240, 50 + mult * (i + 1), 0x0000);
            drawFromSdDownscale((uint32_t)(0xE) + ((uint32_t)(0x22740) * (items_per_page * page + i)), 10, 51 + mult * i, 240, 294, scale);
          }
          tft.setCursor(x, y + (mult * i));
          tft.print(choices[items_per_page * page + i].label);
        }
        drawFromSd(0x5DAF1F, 0, 26, 240, 25);
        if (type >= 0 && type < 3)
          drawFromSd(0x5DDDFF + (0x4E2 * type), 0, 26, 25, 25);
      }
      else
      {
        page = 0;
        changed = true;
        choice = 0;
        drawFromSd(0x639365, 0, 51, 240, 269);
        for (int i = 0; i < items_per_page && items_per_page * page + i < icount; i++)
        {
          if (images)
          {
            tft.drawLine(0, 50 + mult * i, 240, 50 + mult * i, 0x0000);
            drawFromSdDownscale((uint32_t)(0xE) + ((uint32_t)(0x22740) * (items_per_page * page + i)), 10, 51 + mult * i, 240, 294, scale);
          }
          if (choices[items_per_page * page + i].icon.address != 0)
          {
            drawFromSd(icon_x, 51 + mult * i, choices[items_per_page * page + i].icon);
          }
          tft.setCursor(x, y + (mult * i));
          tft.print(choices[items_per_page * page + i].label);
        }

        drawFromSd(0x5DAF1F, 0, 26, 240, 25);
        if (type >= 0 && type < 3)
          drawFromSd(0x5DDDFF + (0x4E2 * type), 0, 26, 25, 25);
      }
      if (!changed)
      {

        drawFromSd(0x639365 + (mult * old_choice * 240 * 2), 0, 51 + (mult * old_choice), 240, mult);
      }
      listMenu_sub(label, type, page, pages);
      tft.setTextColor(color_inactive);
      tft.setCursor(x, y + (mult * old_choice));
      if (!changed)
      {
        if (images)
        {
          tft.drawLine(0, 50 + mult * (choice), 240, 50 + mult * (choice), 0x0000);
          drawFromSdDownscale((uint32_t)(0xE) + ((uint32_t)(0x22740) * (items_per_page * page + (old_choice))), 10, 51 + mult * (old_choice), 240, 294, scale);
        }
        if (choices[items_per_page * page + old_choice].icon.address != 0)
        {
          drawFromSd(icon_x, 51 + mult * (old_choice), choices[items_per_page * page + old_choice].icon);
        }
        tft.print(choices[items_per_page * page + old_choice].label);
      }

      tft.fillRect(0, 51 + mult * choice, 240, mult, color_active);
      tft.setCursor(x, y + (mult * choice));

      tft.print(choices[items_per_page * page + choice].label);
      if (images)
      {
        tft.drawLine(0, 50 + mult * (choice + 1), 240, 50 + mult * (choice + 1), 0x0000);
        drawFromSdDownscale((uint32_t)(0xE) + ((uint32_t)(0x22740) * (items_per_page * page + choice)), 10, 51 + mult * choice, 240, 294, scale);
      }
      if (choices[items_per_page * page + choice].icon.address != 0)
      {
        drawFromSd(icon_x, 51 + mult * choice, choices[items_per_page * page + choice].icon);
      }
      break;
    }
    case BACK:
    { // EXIT
      exit = true;
      return -1;
      break;
    }
    }

  return -1;
}
int listMenu(const String choices[], int icount, bool images, int type, String label)
{
  mOption *optionArr = new mOption[icount];
  for (int i = 0; i < icount; i++)
  {
    optionArr[i].label = choices[i];
    optionArr[i].icon = SDImage();
  }
  return listMenu(optionArr, icount, images, type, label);
}

void spinAnim(int x, int y, int size_x, int size_y, int offset, int spacing)
{
  //FIRSTLY WAS WRITED MANUALLY BUT AFTER ENCOURING A BUG 
  //I STOLE FROM CHATGPT
  //(slighty modified)
  // Open the file and seek to the position where image data starts
  File file = SD.open("/FIRMWARE/IMAGES.SG");
  file.seek(0x658BC4);
  // Read data from SD card into buffer
  const int buffer_size = 400 * 2;
  uint8_t buffer[buffer_size];
  file.read(buffer, buffer_size);
  file.close();

  // Define max frames and set up variables for animation path
  int max_count = (2 * size_x) + (2 * (size_y - 1)); // Adjusted max_count for full frames
  int printed_count = 0;
  int xt = 0;
  int yt = 0;
  bool draw = true;

  // Array for the current 7x7 frame, as uint16_t (16-bit color for pushImage)
  uint16_t currentcircle[49]; // 7x7 frame requires 49 pixels

  // Main animation loop
  while (printed_count < max_count)
  {
    for (int j = offset; j >= 0 && printed_count < max_count; j--)
    {
      if (draw)
      {
        int start_index = 98 * j; // Starting index for the 7x7 frame in buffer

        // Ensure we're within buffer bounds
        if (start_index + 98 <= buffer_size)
        {
          for (int i = 0; i < 49; i++)
          {
            // Convert two bytes per pixel into 16-bit color
            currentcircle[i] = (buffer[start_index + (i * 2)] << 8) | buffer[start_index + (i * 2) + 1];
          }

          // Display the frame at the calculated position
          tft.pushImage(x + xt, y + yt, 7, 7, currentcircle);
        }
      }

      if (printed_count < size_x)
      {
        xt += spacing;
      }
      else if (printed_count < size_x + size_y - 1)
      {
        yt += spacing;
      }
      else if (printed_count < (2 * size_x) + size_y - 1)
      {
        xt -= spacing;
      }
      else
      { // Move up
        yt -= spacing;
        if (yt <= -((size_y - 1) * spacing))
        {
          draw = false; // End animation once reaching the top again
        }
      }

      printed_count++;
    }
    offset = 7; // Reset offset after each loop iteration
  }
}

int choiceMenu(const String choices[], int count, bool context)
{
  int x = 30;
  int y = 95;
  int mul = 20;
  uint16_t color_active = 0xF9C0;
  uint16_t color_inactive = 0x0000;

  if (context)
  {
    drawFromSd(0x607565, 16, 100, 208, 123, true, 0x07e0);
    x = 40;
    y = 120;
  }
  else
  {
    drawFromSd(0x5E8A25, 0, 68, 240, 128);
    x = 30;
    y = 95;
  }

  // tft.setCursor(36, 95);
  tft.setTextSize(1);
  tft.setTextColor(color_inactive);
  changeFont(1);

  for (int i = 0; i < count; i++)
  {
    tft.setCursor(x, y + (mul * i));
    tft.print(choices[i]);
  }

  int choice = 0;
  tft.setTextColor(color_active);
  tft.setCursor(x, y + (mul * choice));
  tft.print(choices[choice]);
  int yy = (y + (mul * choice)) - 10;
  int xx = x - 5;
  tft.fillTriangle(xx - 10, yy, xx - 10, yy + 10, xx - 2, yy + 5, color_active);
  bool exit = false;
  while (!exit)
    switch (buttonsHelding())
    {
    case BACK:
    {
      exit = true;
      return -1;
      break;
    }
    case UP:
    {
      // left
      if (context)
      {
        tft.setTextSize(1);
        tft.setTextColor(color_inactive);

        drawFromSd(0x607565, 16, 100, 208, 123, true, 0x07e0);
        for (int i = 0; i < count; i++)
        {
          tft.setCursor(x, y + (mul * i));
          tft.print(choices[i]);
        }
      }
      else
      {
        tft.setTextSize(1);
        tft.setTextColor(color_inactive);

        drawFromSd(0x5E8A25, 0, 68, 240, 128);
        for (int i = 0; i < count; i++)
        {
          tft.setCursor(x, y + (mul * i));
          tft.print(choices[i]);
        }
      }
      if (choice < 1)
      {
        choice = count - 1;
      }
      else
      {
        choice--;
      }
      tft.setTextColor(color_inactive);
      tft.setCursor(x, y + (mul * (choice + 1)));
      if (choice < count - 1)
        tft.print(choices[choice + 1]);
      if (choice == count - 1)
      {
        tft.setCursor(x, y);
        tft.print(choices[0]);
      }
      tft.setTextColor(color_active);
      tft.setCursor(x, y + (mul * choice));
      int yy = (y + (mul * choice)) - 10;
      int xx = x - 5;
      tft.fillTriangle(xx - 10, yy, xx - 10, yy + 10, xx - 2, yy + 5, color_active);
      tft.print(choices[choice]);
      Serial.println(choice);
      delay(100);
      break;
    }
    case SELECT:
    {
      // middle

      exit = true;
      return choice;
      break;
    }
    case DOWN:
    {
      // right
      if (context)
      {
        tft.setTextSize(1);
        tft.setTextColor(color_inactive);

        drawFromSd(0x607565, 16, 100, 208, 123, true, 0x07e0);
        for (int i = 0; i < count; i++)
        {
          tft.setCursor(x, y + (mul * i));
          tft.print(choices[i]);
        }
      }
      else
      {
        tft.setTextSize(1);
        tft.setTextColor(color_inactive);

        drawFromSd(0x5E8A25, 0, 68, 240, 128);
        for (int i = 0; i < count; i++)
        {
          tft.setCursor(x, y + (mul * i));
          tft.print(choices[i]);
        }
      }
      if (choice > count - 2)
      {
        choice = 0;
      }
      else
      {
        choice++;
      }
      tft.setTextColor(color_inactive);
      tft.setCursor(x, y + (mul * (choice - 1)));
      if (choice > 0)
        tft.print(choices[choice - 1]);
      if (choice == 0)
      {
        tft.setCursor(x, y + (mul * (count - 1)));
        tft.print(choices[count - 1]);
      }
      tft.setTextColor(color_active);
      tft.setCursor(x, y + (mul * choice));
      int yy = (y + (mul * choice)) - 10;
      int xx = x - 5;
      tft.fillTriangle(xx - 10, yy, xx - 10, yy + 10, xx - 2, yy + 5, color_active);
      tft.print(choices[choice]);
      Serial.println(choice);

      delay(100);
      break;
    }
    }
  return -1;
}


void printSplitString(String text)
{
  int wordStart = 0;
  int wordEnd = 0;
  while ( (text.indexOf(' ', wordStart) >= 0) && ( wordStart <= text.length())) {
    wordEnd = text.indexOf(' ', wordStart + 1);
    uint16_t len = tft.textWidth(text.substring(wordStart, wordEnd));
    if (tft.getCursorX() + len >= tft.width()) {
      tft.println();
      if (wordStart > 0) wordStart++;
    }
    tft.print(text.substring(wordStart, wordEnd));
    wordStart = wordEnd;
  }
}
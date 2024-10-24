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
  }
}
void drawFromSdTrans(uint32_t pos, int pos_x, int pos_y, int size_x, int size_y, uint16_t tc, File file)
{
  file.seek(pos);

  const int buffer_size = size_x * 2; // 2 bytes per pixel
  uint8_t buffer[buffer_size];

  for (int a = 0; a < size_y; a++)
  {
    // Read a whole line (row) of pixels at once
    file.read(buffer, buffer_size);

    int draw_start = -1;
    for (int i = 0; i < size_x; i++)
    {
      uint16_t wd = (buffer[2 * i] << 8) | buffer[2 * i + 1];

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
          tft.pushImage(pos_x + draw_start, a + pos_y, i - draw_start, 1, (uint16_t *)(&buffer[draw_start * 2]));
          draw_start = -1;
        }
      }
    }
    // If there was a drawing segment that didn't end
    if (draw_start != -1)
    {
      tft.pushImage(pos_x + draw_start, a + pos_y, size_x - draw_start, 1, (uint16_t *)(&buffer[draw_start * 2]));
    }
  }
}

void drawFromSdDownscale(uint32_t pos, int pos_x, int pos_y, int size_x, int size_y, int scale, File file)
{
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
}

void writeText(int x, int y, char *str)
{
  File fontFile = SD.open("/FIRMWARE/FONT.bin", FILE_READ);
  for (int i = 0; i < strlen(str); i++)
  {
    if (str[i] < 91 && str[i] > 64)
      drawFromSdTrans(0x480 * (uint8_t(str[i]) - 54), x + (12 * i), y, 24, 24, 0xffff, fontFile);
    if (str[i] < 123 && str[i] > 96)
      drawFromSdTrans(0x480 * (uint8_t(str[i]) - 60), x + (12 * i), y, 24, 24, 0xffff, fontFile);
  }
}

void drawStatusBar()
{
  File lol2 = SD.open("/FIRMWARE/IMAGES.SG", FILE_READ);
  int charge = chrg.getBatteryLevel();
  int toIcon = (charge / 25) - 1;
  if (toIcon > 3)
    toIcon = 3;
  if (toIcon < 0)
    toIcon = 0;
  if (!lol2.available())
    blueScreen("SD_CARD_NOT_AVAILABLE");
  drawFromSd(0X5A708D, 0, 0, 240, 26, lol2); // statusbar
  drawFromSd(0x5AD47D, 0, 0, 37, 26, lol2);  // no_signal
  // drawFromSd(0x5ABC1D+ (0x618)*3,0,0,30,26,lol2); //signal
  drawFromSd(0X5AA14D + (0x6B4) * toIcon, 207, 0, 33, 26, lol2); // battery
  //  tft.print(String(charge) + String("%"));
  lol2.close();
}

void rendermenu(int choice, bool right)
{
  File lol2 = SD.open("/FIRMWARE/IMAGES.SG", FILE_READ);
  switch (choice)
  {
  case 0:
    if (right)
    {
      drawFromSd(0x5D5097 + (0x17A2 * 3), 137, (123 + 26), 55, 55, lol2);
    }
    else
    {
      drawFromSd(0x5D5097 + (0x17A2 * 1), 138, (42 + 26), 55, 55, lol2);
    }
    drawFromSd(0x5D0341, 51, (45 + 26), 49, 49, lol2);
    break;
  case 1:
    if (right)
    {
      drawFromSd(0x5D5097, 49, (43 + 26), 55, 55, lol2);
    }
    else
    {
      drawFromSd(0x5D5097 + (0x17A2 * 2), 49, (122 + 26), 55, 55, lol2);
    }
    drawFromSd(0x5D1603, 141, (44 + 26), 49, 51, lol2);
    break;
  case 2:
    if (right)
    {
      drawFromSd(0x5D5097 + (0x17A2 * 1), 138, (42 + 26), 55, 55, lol2);
    }
    else
    {
      drawFromSd(0x5D5097 + (0x17A2 * 3), 137, (123 + 26), 55, 55, lol2);
    }
    drawFromSd(0x5D2989, 52, (125 + 26), 50, 50, lol2);
    break;
  case 3:
    if (right)
    {
      drawFromSd(0x5D5097 + (0x17A2 * 2), 49, (122 + 26), 55, 55, lol2);
    }
    else
    {
      drawFromSd(0x5D5097, 49, (43 + 26), 55, 55, lol2);
    }
    drawFromSd(0x5D3D11, 140, (125 + 26), 49, 51, lol2);
    break;
  }
  lol2.close();
}
void blueScreen(char *reason)
{
  tft.fillScreen(0x0019);
  tft.setCursor(10, 40);
  tft.setTextColor(0xFFFF);
  tft.setTextFont(1);
  tft.setTextSize(4);
  tft.println("=)");
  tft.setTextSize(1);
  tft.println(String("\n\n\nThere a problem with your device\nYou can fix it by yourself i guess\nThere some details for you:\n\n\nReason:" + String(reason)));
  while (true);
}

void drawFromSd(uint32_t pos, int pos_x, int pos_y, int size_x, int size_y, File file)
{
  file.seek(pos);

  const int buffer_size = size_x * 2;
  uint8_t buffer[buffer_size];

  for (int a = 0; a < size_y; a++)
  {
    file.read(buffer, buffer_size);

    tft.pushImage(pos_x, a + pos_y, size_x, 1, (uint16_t *)buffer);
  }
}
void listMenu_sub(String label, int type, int page, int pages)
{
  File lol2 = SD.open("/FIRMWARE/IMAGES.SG", FILE_READ);
  tft.setCursor(30, 45);
  tft.setTextSize(1);
  changeFont(1);
  tft.setTextColor(0xffff);
  tft.print(label);
  changeFont(0);
  tft.setCursor(210, 40);
  tft.print(String(page + 1) + String("/") + String(pages + 1));
  changeFont(1);
  lol2.close();
}

int listMenu(const String choices[], int icount, bool images, int type, String label)
{

  File lol2 = SD.open("/FIRMWARE/IMAGES.SG", FILE_READ);
  int scale = 7;
  int x = 10;
  int y = 65;
  int mult = 20;
  if (images)
  {
    mult = (294 / scale) + 1;
    x = 55;
  }
  drawFromSd(0x5DAF1F, 0, 26, 240, 25, lol2);
  if (type >= 0 && type < 3)
    drawFromSd(0x5DDDFF + (0x4E2 * type), 0, 26, 25, 25, lol2);
  int items_per_page = 269 / mult;
  int count = icount;
  int pages = (icount + items_per_page - 1) / items_per_page - 1; // Calculate pages correctly, accounting for partial pages
  int page = 0;
  int icon_addr;

  uint16_t color_active = 0xFDD3;
  uint16_t color_inactive = 0x0000;
  int choice = 0;

  tft.setTextSize(1);
  tft.setTextColor(color_inactive);
  changeFont(1);

  listMenu_sub(label, type, page, pages);
  drawFromSd(0x639365, 0, 51, 240, 269, lol2);
  tft.setTextColor(color_inactive);

  for (int i = 0; i < items_per_page && items_per_page * page + i < icount; i++)
  {
    if (images)
    {

      tft.drawLine(0, 50 + mult * (i + 1), 240, 50 + mult * (i + 1), 0x0000);
      drawFromSdDownscale((uint32_t)(0xE) + ((uint32_t)(0x22740) * (items_per_page * page + i)), 10, 51 + mult * i, 240, 294, scale, lol2);
    }
    tft.setCursor(x, y + (mult * i));
    tft.print(choices[items_per_page * page + i]);
  }
  tft.fillRect(0, 51 + mult * choice, 240, mult, color_active);
  tft.setCursor(x, y + (mult * choice));
  tft.print(choices[items_per_page * page + choice]);
  if (images)
  {
    tft.drawLine(0, 50 + mult * (choice + 1), 240, 50 + mult * (choice + 1), 0x0000);
    drawFromSdDownscale((uint32_t)(0xE) + ((uint32_t)(0x22740) * (items_per_page * page + choice)), 10, 51 + mult * choice, 240, 294, scale, lol2);
  }
  bool exit = false;
  while (!exit)
  {

    if (digitalRead(37) == LOW)
    {
      while (digitalRead(37) == LOW)
        ;

      return items_per_page * page + choice;
    }
    if (digitalRead(38) == LOW)
    { // Up button
      bool changed = false;
      int old_choice = choice;
      if (choice > 0)
      {
        choice--;
      }
      else if (page > 0)
      {
        drawFromSd(0x5DAF1F, 0, 26, 240, 25, lol2);
        if (type >= 0 && type < 3)
          drawFromSd(0x5DDDFF + (0x4E2 * type), 0, 26, 25, 25, lol2);
        page--;
        changed = true;
        drawFromSd(0x639365, 0, 51, 240, 269, lol2);
        for (int i = 0; i < items_per_page && items_per_page * page + i < icount; i++)
        {
          if (images)
          {
            tft.drawLine(0, 50 + mult * (i + 1), 240, 50 + mult * (i + 1), 0x0000);
            tft.drawLine(0, 50 + mult * (i), 240, 50 + mult * (i), 0x0000);
            drawFromSdDownscale((uint32_t)(0xE) + ((uint32_t)(0x22740) * (items_per_page * page + i)), 10, 51 + mult * i, 240, 294, scale, lol2);
          }
          tft.setCursor(x, y + (mult * i));
          tft.print(choices[items_per_page * page + i]);
        }
        choice = (items_per_page - 1);
      }
      else
      {
        drawFromSd(0x5DAF1F, 0, 26, 240, 25, lol2);
        if (type >= 0 && type < 3)
          drawFromSd(0x5DDDFF + (0x4E2 * type), 0, 26, 25, 25, lol2);
        page = pages;
        changed = true;
        drawFromSd(0x639365, 0, 51, 240, 269, lol2);
        for (int i = 0; i < items_per_page && items_per_page * page + i < icount; i++)
        {
          if (images)
          {
            tft.drawLine(0, 50 + mult * (i + 1), 240, 50 + mult * (i + 1), 0x0000);
            drawFromSdDownscale((uint32_t)(0xE) + ((uint32_t)(0x22740) * (items_per_page * page + i)), 10, 51 + mult * i, 240, 294, scale, lol2);
          }
          tft.setCursor(x, y + (mult * i));
          tft.print(choices[items_per_page * page + i]);
        }
        choice = (icount % items_per_page) ? (icount % items_per_page) - 1 : (items_per_page - 1);
      }

      if (!changed)
        drawFromSd(0x639365 + (mult * old_choice * 240 * 2), 0, 51 + (mult * old_choice), 240, mult, lol2);

      listMenu_sub(label, type, page, pages);
      tft.setTextColor(color_inactive);
      tft.setCursor(x, y + (mult * old_choice));
      if (!changed)
      {
        if (images)
        {
          tft.drawLine(0, 50 + mult * (choice + 2), 240, 50 + mult * (choice + 2), 0x0000);
          drawFromSdDownscale((uint32_t)(0xE) + ((uint32_t)(0x22740) * (items_per_page * page + (old_choice))), 10, 51 + mult * (old_choice), 240, 294, scale, lol2);
        }
        tft.print(choices[items_per_page * page + old_choice]);
      }

      tft.fillRect(0, 51 + mult * choice, 240, mult, color_active);
      tft.setCursor(x, y + (mult * choice));
      tft.print(choices[items_per_page * page + choice]);
      if (images)
      {
        tft.drawLine(0, 50 + mult * (choice + 1), 240, 50 + mult * (choice + 1), 0x0000);
        drawFromSdDownscale((uint32_t)(0xE) + ((uint32_t)(0x22740) * (items_per_page * page + choice)), 10, 51 + mult * choice, 240, 294, scale, lol2);
      }
      while (digitalRead(38) == LOW)
        ;
    }
    if (digitalRead(39) == LOW)
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
        drawFromSd(0x639365, 0, 51, 240, 269, lol2);
        for (int i = 0; i < items_per_page && items_per_page * page + i < icount; i++)
        {
          if (images)
          {
            tft.drawLine(0, 50 + mult * (i + 1), 240, 50 + mult * (i + 1), 0x0000);
            drawFromSdDownscale((uint32_t)(0xE) + ((uint32_t)(0x22740) * (items_per_page * page + i)), 10, 51 + mult * i, 240, 294, scale, lol2);
          }
          tft.setCursor(x, y + (mult * i));
          tft.print(choices[items_per_page * page + i]);
        }
        drawFromSd(0x5DAF1F, 0, 26, 240, 25, lol2);
        if (type >= 0 && type < 3)
          drawFromSd(0x5DDDFF + (0x4E2 * type), 0, 26, 25, 25, lol2);
      }
      else
      {
        page = 0;
        changed = true;
        choice = 0;
        drawFromSd(0x639365, 0, 51, 240, 269, lol2);
        for (int i = 0; i < items_per_page && items_per_page * page + i < icount; i++)
        {
          if (images)
          {
            tft.drawLine(0, 50 + mult * i, 240, 50 + mult * i, 0x0000);
            drawFromSdDownscale((uint32_t)(0xE) + ((uint32_t)(0x22740) * (items_per_page * page + i)), 10, 51 + mult * i, 240, 294, scale, lol2);
          }
          tft.setCursor(x, y + (mult * i));
          tft.print(choices[items_per_page * page + i]);
        }

        drawFromSd(0x5DAF1F, 0, 26, 240, 25, lol2);
        if (type >= 0 && type < 3)
          drawFromSd(0x5DDDFF + (0x4E2 * type), 0, 26, 25, 25, lol2);
      }
      if (!changed)
      {

        drawFromSd(0x639365 + (mult * old_choice * 240 * 2), 0, 51 + (mult * old_choice), 240, mult, lol2);
      }
      listMenu_sub(label, type, page, pages);
      tft.setTextColor(color_inactive);
      tft.setCursor(x, y + (mult * old_choice));
      if (!changed)
      {
        if (images)
        {
          tft.drawLine(0, 50 + mult * (choice), 240, 50 + mult * (choice), 0x0000);
          drawFromSdDownscale((uint32_t)(0xE) + ((uint32_t)(0x22740) * (items_per_page * page + (old_choice))), 10, 51 + mult * (old_choice), 240, 294, scale, lol2);
        }
        tft.print(choices[items_per_page * page + old_choice]);
      }

      tft.fillRect(0, 51 + mult * choice, 240, mult, color_active);
      tft.setCursor(x, y + (mult * choice));

      tft.print(choices[items_per_page * page + choice]);
      if (images)
      {
        tft.drawLine(0, 50 + mult * (choice + 1), 240, 50 + mult * (choice + 1), 0x0000);
        drawFromSdDownscale((uint32_t)(0xE) + ((uint32_t)(0x22740) * (items_per_page * page + choice)), 10, 51 + mult * choice, 240, 294, scale, lol2);
      }
      while (digitalRead(39) == LOW)
        ;
    }
    if (digitalRead(0) == LOW)
    { // EXIT
    exit = true;
      while (digitalRead(0) == LOW);
      
      return -1;
    }
  }
  lol2.close();
}

void spinAnim(int x, int y, int size_x, int size_y, int offset)
{
  File lol2 = SD.open("/FIRMWARE/IMAGES.SG", FILE_READ);
  int max_count = (2 * x) + (2 * (y - 2));
  int printed_count = 0;
  int xt = 0;
  int yt = 0;
  int yy = 0;
  bool draw = true;
  while (printed_count < max_count)
  {
    for (int j = offset; j >= 0 && printed_count < max_count; j--)
    {
      if (draw)
        drawFromSd(0x658BC5 + (j * 0x62), x + xt, y + yt, 7, 7, lol2);
      if (printed_count < size_x)
      {
        xt += 7;
      }
      else if (printed_count >= size_x && printed_count < size_x + (size_y - 1))
      {
        yt += 7;
      }
      else if (printed_count > size_x + (size_y - 1) && printed_count < (size_x * 2) + size_y)
      {
        xt -= 7;
      }
      else if (printed_count > (size_x * 2) + (size_y)-2 && yy < size_y)
      {
        yt -= 7;
        yy++;
        if (yy > size_y - 2)
          draw = false;
      }
      printed_count++;
    }
    offset = 7;
  }
  lol2.close();
}

int choiceMenu(const String choices[], int count, bool context)
{
  while (digitalRead(37) == LOW)
    ;
  File lol2 = SD.open("/FIRMWARE/IMAGES.SG", FILE_READ);
  int x = 30;
  int y = 95;
  int mul = 20;
  uint16_t color_active = 0xF9C0;
  uint16_t color_inactive = 0x0000;

  if (context)
  {
    drawFromSdTrans(0x607565, 16, 100, 208, 123, 0x07e0, lol2);
    x = 40;
    y = 120;
  }
  else
  {

    drawFromSd(0x5E8A25, 0, 68, 240, 128, lol2);
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
  {
    if (digitalRead(0) == LOW)
    {
      exit = true;
      return -1;
    }
    if (digitalRead(38) == LOW)
    {
      // left
      if (context)
      {
        tft.setTextSize(1);
        tft.setTextColor(color_inactive);

        drawFromSdTrans(0x607565, 16, 100, 208, 123, 0x07e0, lol2);
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

        drawFromSd(0x5E8A25, 0, 68, 240, 128, lol2);
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

      while (digitalRead(38) == LOW)
        ;
      delay(100);
    }
    if (digitalRead(37) == LOW)
    {
      // middle
      exit = true;
      return choice;
    }
    if (digitalRead(39) == LOW)
    {
      // right
      if (context)
      {
        tft.setTextSize(1);
        tft.setTextColor(color_inactive);

        drawFromSdTrans(0x607565, 16, 100, 208, 123, 0x07e0, lol2);
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

        drawFromSd(0x5E8A25, 0, 68, 240, 128, lol2);
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

      while (digitalRead(39) == LOW)
        ;
      delay(100);
    }
  }
  lol2.close();
}

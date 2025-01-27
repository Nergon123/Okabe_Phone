# Resource Editor
A small script that helps edit [IMAGES.SG](../DEVINFO.MD#imagessg)


## Features
With this tool you can:
- View Image by address
- Replace Images in [IMAGES.SG](../DEVINFO.MD#imagessg) file
- Get SDImage variable
- Add new images to file

# Usage
To use you need to install Python itself, Pillow and pyperclip python packages.
Can be done with 
```
pip install -r requirements.txt
```
If requirements are met then you can launch this tool

When you open this tool you should see

- **Resource File** Field: There should be path to [IMAGES.SG](../DEVINFO.MD#imagessg)
- **Address** Field: There should be address taken from [ADRESSES.MD](../ADRESSES.MD)
- **Offset** Field With `+` and `-` buttons: you can increment or decrement value by offset
- **Width** Field and **Height** Field: Those parameters should be taken from mentioned adresses file
- **Display Image** button: will render image from file on screen
- **Replace Image** button: will let you use your file to replace image (for some reason on some systems filter doesn't work, if so you should choose `All Files (*.*)` from dropdown menu), Image should be in same resolution as source image. Press arrow to confirm replacing.
- **Output** field and **Transparent** checkbox: This one is for SDImage variable, if you want to use color for transparency (there no alpha channel, this works more like chroma-key) you need to click on color on source image.

Thats everything what I can tell about this tool.

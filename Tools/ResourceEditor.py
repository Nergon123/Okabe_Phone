import tkinter as tk
from tkinter import ttk, filedialog, messagebox
import struct
from PIL import Image, ImageTk, ImageDraw
import pyperclip  



_offset = 1
def rgb888_to_rgb565(r, g, b):
    return ((r >> 3) << 11) | ((g >> 2) << 5) | (b >> 3)


def load_image_from_binary(file_path, address, width, height):
    with open(file_path, 'rb') as f:
        f.seek(address)
        image = Image.new('RGB', (width, height))
        pixels = image.load()
        for y in range(height):
            for x in range(width):
                data = f.read(2)
                if not data:
                    break
                if len(data) < 2:
                    print(f"Buffer less than 2...")
                    break

                rgb565 = struct.unpack('<H', data)[0]
                r = ((rgb565 >> 11) & 0x1F) << 3
                g = ((rgb565 >> 5) & 0x3F) << 2
                b = (rgb565 & 0x1F) << 3
                pixels[x, y] = (r, g, b)
        return image


def save_image_to_binary(file_path, address, image):
    width, height = image.size
    with open(file_path, 'r+b') as f:
        f.seek(address)
        for y in range(height):
            for x in range(width):
                r, g, b = image.getpixel((x, y))
                rgb565 = rgb888_to_rgb565(r, g, b)
                f.write(struct.pack('<H', rgb565))

def save_image():
    try:
        save_file_path = filedialog.asksaveasfilename(filetypes=[("PNG picture","*.png")],title="Save Image As...");
        file_path = entry_file.get()
        address = int(entry_address.get(), 16) + _offset
        width = int(entry_width.get())
        height = int(entry_height.get())
        image = load_image_from_binary(file_path, address, width, height)
        image.save(save_file_path)
    except Exception as e:
        label_status.config(text=f"Error: {e}", foreground="red")

def choose_file():
    file_path = filedialog.askopenfilename(filetypes=[("OkabePhone Resource Files", "*.SG"),("All files", "*.*")])
    if file_path:
        entry_file.delete(0, tk.END)
        entry_file.insert(0, file_path)
        update_output()


def display_image():
    try:
        file_path = entry_file.get()
        address = int(entry_address.get(), 16) + _offset
        width = int(entry_width.get())
        height = int(entry_height.get())
        if width > 240 or height > 320:
            raise ValueError("Maximum dimensions are 240x320")
        global original_image, binary_file_path, binary_file_address
        original_image = load_image_from_binary(file_path, address, width, height)
        original_photo = ImageTk.PhotoImage(original_image)
        label_original.config(image=original_photo)
        label_original.image = original_photo  
        binary_file_path = file_path
        binary_file_address = address
        global current_width, current_height
        current_width, current_height = width, height
        update_output()
    except Exception as e:
        label_status.config(text=f"Error: {e}", foreground="red")


def get_color_at_cursor(event):
    try:
        x, y = event.x, event.y
        if 0 <= x < current_width and 0 <= y < current_height:
            offset = binary_file_address + (y * current_width + x) * 2
            with open(binary_file_path, 'rb') as f:
                f.seek(offset)
                data = f.read(2)
                if not data:
                    raise ValueError("Failed to read RGB565 value from file")
                rgb565 = struct.unpack('<H', data)[0]
            global selected_color
            selected_color = rgb565
            update_output()
            label_status.config(text=f"RGB565 Color at ({x}, {y}): {hex(rgb565)}", foreground="green")
        else:
            label_status.config(text="Cursor is outside the image bounds", foreground="red")
    except Exception as e:
        label_status.config(text=f"Error: {e}", foreground="red")


def update_output():

    try:
        address = entry_address.get()
        width = entry_width.get()
        height = entry_height.get()
        transparent = checkbox_transparent_var.get()
        transparent_color = selected_color if transparent else 0

        if transparent:
            output_text = f"SDImage({hex(int(address, 16))}, {width}, {height}, {hex(transparent_color)}, {str(transparent).lower()});"
        else:
            output_text = f"SDImage({hex(int(address, 16))}, {width}, {height});"

        entry_output.delete(0, tk.END)
        entry_output.insert(0, output_text)

    except Exception as e:
        label_status.config(text=f"Error: {e}", foreground="red")


def copy_to_clipboard():

    try:
        output_text = entry_output.get()
        pyperclip.copy(output_text)
        label_status.config(text="Copied to clipboard!", foreground="blue")

    except Exception as e:
        label_status.config(text=f"Error: {e}", foreground="red")


def replace_image():
    try:
        file_path = filedialog.askopenfilename(
            filetypes=[("Image files", "*.png *.jpg *.jpeg *.bmp"), ("All files", "*.*")]
        )
        
        if not file_path:
            return
        
        new_image = Image.open(file_path).convert('RGB')
        if new_image.height != int(entry_height.get()) or new_image.width != int(entry_width.get()):
            label_status.config(text=f"Image should be same size as source (Input W:{new_image.width}, H:{new_image.height}).", foreground="red")
            return
        
        new_image = new_image.resize((current_width, current_height), Image.Resampling.LANCZOS)
        save_image_to_binary(binary_file_path, binary_file_address, new_image)
        display_image()
        label_status.config(text="Image replaced successfully!", foreground="green")

    except Exception as e:
        label_status.config(text=f"Error: {e}", foreground="red")


def increment_address():
    try:
        address = int(entry_address.get(), 16) + _offset
        offset = int(entry_offset.get(), 16)
        new_address = address + offset
        if new_address<0:
            new_address = 0
        entry_address.delete(0, tk.END)
        entry_address.insert(0, hex(new_address))
        display_image()
    except Exception as e:
        label_status.config(text=f"Error: {e}", foreground="red")


def decrement_address():
    try:
        address = int(entry_address.get(), 16) + _offset
        offset = int(entry_offset.get(), 16)
        new_address = address - offset
        if new_address<0:
            new_address = 0
        entry_address.delete(0, tk.END)
        entry_address.insert(0, hex(new_address))
        display_image()
    except Exception as e:
        label_status.config(text=f"Error: {e}", foreground="red")

root = tk.Tk()
root.title("Okabe Phone Image Resources Editor v0.4.2 Alpha")

frame_input = ttk.Frame(root, padding="10")
frame_input.grid(row=0, column=0, sticky=(tk.W, tk.E))

ttk.Label(frame_input, text="Resource File:").grid(row=0, column=0, sticky=tk.W)
entry_file = ttk.Entry(frame_input, width=30)
entry_file.grid(row=0, column=1, sticky=tk.W)

button_choose_file = ttk.Button(frame_input, text="Browse", command=choose_file)
button_choose_file.grid(row=0, column=2, sticky=tk.W)

ttk.Label(frame_input, text="Address (Hex):").grid(row=1, column=0, sticky=tk.W)
entry_address = ttk.Entry(frame_input, width=10)
entry_address.insert(0, "0xD");
entry_address.grid(row=1, column=1, sticky=tk.W)
entry_address.bind("<KeyRelease>", lambda e: update_output())

entry_offset = ttk.Entry(frame_input, width=10)
entry_offset.insert(0, "0x10")  
entry_offset.grid(row=1, column=1)

button_decrement = ttk.Button(frame_input, text="-", command=decrement_address,width=1)
button_decrement.grid(row=1, column=1,padx=(0,150))

button_increment =  ttk.Button(frame_input, text="+", command=increment_address,width=1)
button_increment.grid(row=1, column=1,padx=(0,110))

ttk.Label(frame_input, text="Width:").grid(row=2, column=0, sticky=tk.W)
entry_width = ttk.Entry(frame_input, width=10)
entry_width.insert(0, "240");
entry_width.grid(row=2, column=1, sticky=tk.W)
entry_width.bind("<KeyRelease>", lambda e: update_output())

ttk.Label(frame_input, text="Height:").grid(row=3, column=0, sticky=tk.W)
entry_height = ttk.Entry(frame_input, width=10)
entry_height.insert(0, "294");
entry_height.grid(row=3, column=1, sticky=tk.W)
entry_height.bind("<KeyRelease>", lambda e: update_output())

button_display = ttk.Button(frame_input, text="Display Image", command=display_image)
button_display.grid(row=4, column=0, columnspan=3)

button_replace = ttk.Button(frame_input, text="Replace Image", command=replace_image)
button_replace.grid(row=5, column=0, columnspan=3)

button_replace = ttk.Button(frame_input, text="Save As...", command=save_image)
button_replace.grid(row=5, column=1, columnspan=3)

checkbox_transparent_var = tk.BooleanVar()
checkbox_transparent = ttk.Checkbutton(frame_input, text="Transparent", variable=checkbox_transparent_var, command=update_output)
checkbox_transparent.grid(row=6, column=0, columnspan=3)

ttk.Label(frame_input, text="Output:").grid(row=7, column=0, sticky=tk.W)
entry_output = ttk.Entry(frame_input, width=50)
entry_output.grid(row=7, column=1, sticky=tk.W)

button_copy = ttk.Button(frame_input, text="Copy to Clipboard", command=copy_to_clipboard)
button_copy.grid(row=7, column=2, sticky=tk.W)

label_status = ttk.Label(frame_input, text="Click on the image to get RGB565 color", font=("Arial", 10), foreground="blue")
label_status.grid(row=8, column=0, columnspan=3, pady=(10, 0))

frame_images = ttk.Frame(root, padding="10")
frame_images.grid(row=1, column=0, sticky=(tk.W, tk.E))

label_original = ttk.Label(frame_images)
label_original.grid(row=0, column=0, sticky=tk.W)
label_original.bind("<Button-1>", get_color_at_cursor)

selected_color = 0

root.mainloop()
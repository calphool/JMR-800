import tkinter as tk
from tkinter import simpledialog

SCREEN_WIDTH = 128
SCREEN_HEIGHT = 64
SCALE = 10
PIXEL_COLOR = "#ffffff"
BG_COLOR = "black"
GRID_COLOR = "#333333"
HELP_HEIGHT = 30

CHAR_WIDTH = 6
CHAR_HEIGHT = 8


FONT_5X7 = {
    " ": [0b00000, 0b00000, 0b00000, 0b00000, 0b00000, 0b00000, 0b00000],
    "!": [0b00100, 0b00100, 0b00100, 0b00100, 0b00100, 0b00000, 0b00100],
    "\"": [0b01010, 0b01010, 0b01010, 0b00000, 0b00000, 0b00000, 0b00000],
    "#": [0b01010, 0b01010, 0b11111, 0b01010, 0b11111, 0b01010, 0b01010],
    "$": [0b00100, 0b01111, 0b10100, 0b01110, 0b00101, 0b11110, 0b00100],
    "%": [0b11000, 0b11001, 0b00010, 0b00100, 0b01000, 0b10011, 0b00011],
    "&": [0b01100, 0b10010, 0b10100, 0b01000, 0b10101, 0b10010, 0b01101],
    "'": [0b01100, 0b00100, 0b01000, 0b00000, 0b00000, 0b00000, 0b00000],
    "(": [0b00010, 0b00100, 0b01000, 0b01000, 0b01000, 0b00100, 0b00010],
    ")": [0b01000, 0b00100, 0b00010, 0b00010, 0b00010, 0b00100, 0b01000],
    "*": [0b00000, 0b00100, 0b10101, 0b01110, 0b10101, 0b00100, 0b00000],
    "+": [0b00000, 0b00100, 0b00100, 0b11111, 0b00100, 0b00100, 0b00000],
    ",": [0b00000, 0b00000, 0b00000, 0b00000, 0b01100, 0b00100, 0b01000],
    "-": [0b00000, 0b00000, 0b00000, 0b11111, 0b00000, 0b00000, 0b00000],
    ".": [0b00000, 0b00000, 0b00000, 0b00000, 0b00000, 0b01100, 0b01100],
    "/": [0b00000, 0b00001, 0b00010, 0b00100, 0b01000, 0b10000, 0b00000],
    "0": [0b01110, 0b10001, 0b10011, 0b10101, 0b11001, 0b10001, 0b01110],
    "1": [0b00100, 0b01100, 0b00100, 0b00100, 0b00100, 0b00100, 0b01110],
    "2": [0b01110, 0b10001, 0b00001, 0b00010, 0b00100, 0b01000, 0b11111],
    "3": [0b11111, 0b00010, 0b00100, 0b00010, 0b00001, 0b10001, 0b01110],
    "4": [0b00010, 0b00110, 0b01010, 0b10010, 0b11111, 0b00010, 0b00010],
    "5": [0b11111, 0b10000, 0b11110, 0b00001, 0b00001, 0b10001, 0b01110],
    "6": [0b00110, 0b01000, 0b10000, 0b11110, 0b10001, 0b10001, 0b01110],
    "7": [0b11111, 0b00001, 0b00010, 0b00100, 0b01000, 0b01000, 0b01000],
    "8": [0b01110, 0b10001, 0b10001, 0b01110, 0b10001, 0b10001, 0b01110],
    "9": [0b01110, 0b10001, 0b10001, 0b01111, 0b00001, 0b00010, 0b01100],
    ":": [0b00000, 0b01100, 0b01100, 0b00000, 0b01100, 0b01100, 0b00000],
    ";": [0b00000, 0b01100, 0b01100, 0b00000, 0b01100, 0b00100, 0b01000],
    "<": [0b00010, 0b00100, 0b01000, 0b10000, 0b01000, 0b00100, 0b00010],
    "=": [0b00000, 0b00000, 0b11111, 0b00000, 0b11111, 0b00000, 0b00000],
    ">": [0b01000, 0b00100, 0b00010, 0b00001, 0b00010, 0b00100, 0b01000],
    "?": [0b01110, 0b10001, 0b00001, 0b00010, 0b00100, 0b00000, 0b00100],
    "@": [0b01110, 0b10001, 0b00001, 0b01101, 0b10101, 0b10101, 0b01110],
    "A": [0b01110, 0b10001, 0b10001, 0b10001, 0b11111, 0b10001, 0b10001],
    "B": [0b11110, 0b10001, 0b10001, 0b11110, 0b10001, 0b10001, 0b11110],
    "C": [0b01110, 0b10001, 0b10000, 0b10000, 0b10000, 0b10001, 0b01110],
    "D": [0b11100, 0b10010, 0b10001, 0b10001, 0b10001, 0b10010, 0b11100],
    "E": [0b11111, 0b10000, 0b10000, 0b11110, 0b10000, 0b10000, 0b11111],
    "F": [0b11111, 0b10000, 0b10000, 0b11110, 0b10000, 0b10000, 0b10000],
    "G": [0b01110, 0b10001, 0b10000, 0b10111, 0b10001, 0b10001, 0b01111],
    "H": [0b10001, 0b10001, 0b10001, 0b11111, 0b10001, 0b10001, 0b10001],
    "I": [0b01110, 0b00100, 0b00100, 0b00100, 0b00100, 0b00100, 0b01110],
    "J": [0b00111, 0b00010, 0b00010, 0b00010, 0b00010, 0b10010, 0b01100],
    "K": [0b10001, 0b10010, 0b10100, 0b11000, 0b10100, 0b10010, 0b10001],
    "L": [0b10000, 0b10000, 0b10000, 0b10000, 0b10000, 0b10000, 0b11111],
    "M": [0b10001, 0b11011, 0b10101, 0b10101, 0b10001, 0b10001, 0b10001],
    "N": [0b10001, 0b10001, 0b11001, 0b10101, 0b10011, 0b10001, 0b10001],
    "O": [0b01110, 0b10001, 0b10001, 0b10001, 0b10001, 0b10001, 0b01110],
    "P": [0b11110, 0b10001, 0b10001, 0b11110, 0b10000, 0b10000, 0b10000],
    "Q": [0b01110, 0b10001, 0b10001, 0b10001, 0b10101, 0b10010, 0b01101],
    "R": [0b11110, 0b10001, 0b10001, 0b11110, 0b10100, 0b10010, 0b10001],
    "S": [0b01111, 0b10000, 0b10000, 0b01110, 0b00001, 0b00001, 0b11110],
    "T": [0b11111, 0b00100, 0b00100, 0b00100, 0b00100, 0b00100, 0b00100],
    "U": [0b10001, 0b10001, 0b10001, 0b10001, 0b10001, 0b10001, 0b01110],
    "V": [0b10001, 0b10001, 0b10001, 0b10001, 0b10001, 0b01010, 0b00100],
    "W": [0b10001, 0b10001, 0b10001, 0b10101, 0b10101, 0b10101, 0b01010],
    "X": [0b10001, 0b10001, 0b01010, 0b00100, 0b01010, 0b10001, 0b10001],
    "Y": [0b10001, 0b10001, 0b10001, 0b01010, 0b00100, 0b00100, 0b00100],
    "Z": [0b11111, 0b00001, 0b00010, 0b00100, 0b01000, 0b10000, 0b11111],
    "[": [0b01110, 0b01000, 0b01000, 0b01000, 0b01000, 0b01000, 0b01110],
    "\\": [0b00000, 0b10000, 0b01000, 0b00100, 0b00010, 0b00001, 0b00000],
    "]": [0b01110, 0b00010, 0b00010, 0b00010, 0b00010, 0b00010, 0b01110],
    "^": [0b00100, 0b01010, 0b10001, 0b00000, 0b00000, 0b00000, 0b00000],
    "_": [0b00000, 0b00000, 0b00000, 0b00000, 0b00000, 0b00000, 0b11111],
    "`": [0b01000, 0b00100, 0b00010, 0b00000, 0b00000, 0b00000, 0b00000],
    "a": [0b00000, 0b00000, 0b01110, 0b00001, 0b01111, 0b10001, 0b01111],
    "b": [0b10000, 0b10000, 0b10110, 0b11001, 0b10001, 0b10001, 0b11110],
    "c": [0b00000, 0b00000, 0b01110, 0b10000, 0b10000, 0b10001, 0b01110],
    "d": [0b00001, 0b00001, 0b01101, 0b10011, 0b10001, 0b10001, 0b01111],
    "e": [0b00000, 0b00000, 0b01110, 0b10001, 0b11111, 0b10000, 0b01110],
    "f": [0b00110, 0b01001, 0b01000, 0b11100, 0b01000, 0b01000, 0b01000],
    "g": [0b00000, 0b01111, 0b10001, 0b10001, 0b01111, 0b00001, 0b01110],
    "h": [0b10000, 0b10000, 0b10110, 0b11001, 0b10001, 0b10001, 0b10001],
    "i": [0b00100, 0b00000, 0b01100, 0b00100, 0b00100, 0b00100, 0b01110],
    "j": [0b00010, 0b00000, 0b00110, 0b00010, 0b00010, 0b10010, 0b01100],
    "k": [0b10000, 0b10000, 0b10010, 0b10100, 0b11000, 0b10100, 0b10010],
    "l": [0b01100, 0b00100, 0b00100, 0b00100, 0b00100, 0b00100, 0b01110],
    "m": [0b00000, 0b00000, 0b11010, 0b10101, 0b10101, 0b10001, 0b10001],
    "n": [0b00000, 0b00000, 0b10110, 0b11001, 0b10001, 0b10001, 0b10001],
    "o": [0b00000, 0b00000, 0b01110, 0b10001, 0b10001, 0b10001, 0b01110],
    "p": [0b00000, 0b00000, 0b11110, 0b10001, 0b11110, 0b10000, 0b10000],
    "q": [0b00000, 0b00000, 0b01101, 0b10011, 0b01111, 0b00001, 0b00001],
    "r": [0b00000, 0b00000, 0b10110, 0b11001, 0b10000, 0b10000, 0b10000],
    "s": [0b00000, 0b00000, 0b01110, 0b10000, 0b01110, 0b00001, 0b11110],
    "t": [0b01000, 0b01000, 0b11100, 0b01000, 0b01000, 0b01001, 0b00110],
    "u": [0b00000, 0b00000, 0b10001, 0b10001, 0b10001, 0b10011, 0b01101],
    "v": [0b00000, 0b00000, 0b10001, 0b10001, 0b10001, 0b01010, 0b00100],
    "w": [0b00000, 0b00000, 0b10001, 0b10001, 0b10101, 0b10101, 0b01010],
    "x": [0b00000, 0b00000, 0b10001, 0b01010, 0b00100, 0b01010, 0b10001],
    "y": [0b00000, 0b00000, 0b10001, 0b10001, 0b01111, 0b00001, 0b01110],
    "z": [0b00000, 0b00000, 0b11111, 0b00010, 0b00100, 0b01000, 0b11111],
    "{": [0b00010, 0b00100, 0b00100, 0b01000, 0b00100, 0b00100, 0b00010],
    "|": [0b00100, 0b00100, 0b00100, 0b00100, 0b00100, 0b00100, 0b00100],
    "}": [0b01000, 0b00100, 0b00100, 0b00010, 0b00100, 0b00100, 0b01000],
    "~": [0b00000, 0b00000, 0b00000, 0b01101, 0b10010, 0b00000, 0b00000],
}


CANVAS_HEIGHT = SCREEN_HEIGHT * SCALE
TOTAL_HEIGHT = CANVAS_HEIGHT + HELP_HEIGHT


class SH1106Editor:
    def __init__(self, root):
        self.root = root
        self.mode = "TEXT"
        self.current_char = "A"
        self.start_pos = None
        self.preview_end = None
        self.current_color = 1  # 1 = white, 0 = black
        self.pixel_grid = [[0 for _ in range(SCREEN_WIDTH)] for _ in range(SCREEN_HEIGHT)]
        self.canvas = tk.Canvas(
            root,
            width=SCREEN_WIDTH * SCALE,
            height=TOTAL_HEIGHT,
            bg=BG_COLOR
        )
        self.canvas.pack()
        self.canvas.bind("<Button-1>", self.on_click)
        self.canvas.bind("<B1-Motion>", self.on_drag)
        self.canvas.bind("<ButtonRelease-1>", self.on_release)
        self.root.bind("<Key>", self.on_key)
        self.root.bind("<Escape>", lambda e: self.root.destroy())
        self.string_cursor_pos = None
        self.cursor_visible = True
        self.root.after(500, self.blink_cursor)
        self.draw()

    def blink_cursor(self):
        if self.mode == "STRING" and self.string_cursor_pos:
            self.cursor_visible = not self.cursor_visible
            self.draw()
        self.root.after(500, self.blink_cursor)

    def on_click(self, event):
        if event.y > SCREEN_HEIGHT * SCALE:
            return
        x, y = event.x // SCALE, event.y // SCALE
        if self.mode == "TEXT":
            self.draw_char(self.current_char, x, y)
        elif self.mode == "STRING":
            self.string_cursor_pos = (x, y)
            self.draw()
            text = simpledialog.askstring("Input", "Enter text to draw:")
            if text:
                for i, ch in enumerate(text):
                    self.draw_char(ch, x + i * CHAR_WIDTH, y)
            self.string_cursor_pos = None


        elif self.mode == "PIXEL":
            self.draw_pixel(x, y)
        else:
            self.start_pos = (x, y)
        self.draw()

    def on_drag(self, event):
        if self.mode in ("LINE", "BOX", "FILLBOX") and event.y <= SCREEN_HEIGHT * SCALE:
            self.preview_end = (event.x // SCALE, event.y // SCALE)
            self.draw()

    def on_release(self, event):
        if self.start_pos and event.y <= SCREEN_HEIGHT * SCALE:
            end = (event.x // SCALE, event.y // SCALE)
            if self.mode == "LINE":
                self.draw_line(self.start_pos, end)
            elif self.mode == "BOX":
                self.draw_box(self.start_pos, end)
            elif self.mode == "FILLBOX":
                self.draw_filled_box(self.start_pos, end)

            self.start_pos = None
            self.preview_end = None
            self.draw()

    def on_key(self, event):
        key = event.keysym.upper()
        if key == "C":
            self.clear()
        elif key == "E":
            self.export()
        elif key == "D":
            self.current_color ^= 1
        elif key in ("T", "L", "B", "F", "P", "S", "C"):
            self.mode = {
                "T": "TEXT",
                "L": "LINE",
                "B": "BOX",
                "F": "FILLBOX",
                "P": "PIXEL",
                "S": "STRING",
                "C": "COPY"
            }[key]

        elif len(event.char) == 1 and 32 <= ord(event.char) <= 126:
            self.current_char = event.char.upper()
        self.draw()

    def draw_pixel(self, x, y):
        if 0 <= x < SCREEN_WIDTH and 0 <= y < SCREEN_HEIGHT:
            self.pixel_grid[y][x] = self.current_color

    def draw_char(self, ch, x, y):
        font = FONT_5X7.get(ch.upper())
        if not font:
            return
        for row, bits in enumerate(font):
            for col in range(5):
                if bits & (1 << (4 - col)):
                    self.draw_pixel(x + col, y + row)

    def draw_line(self, start, end):
        x0, y0 = start
        x1, y1 = end
        dx, dy = abs(x1 - x0), abs(y1 - y0)
        sx, sy = (1 if x0 < x1 else -1), (1 if y0 < y1 else -1)
        err = dx - dy
        while True:
            self.draw_pixel(x0, y0)
            if x0 == x1 and y0 == y1:
                break
            e2 = 2 * err
            if e2 > -dy:
                err -= dy
                x0 += sx
            if e2 < dx:
                err += dx
                y0 += sy

    def draw_box(self, start, end):
        x0, y0 = start
        x1, y1 = end
        for x in range(min(x0, x1), max(x0, x1) + 1):
            self.draw_pixel(x, y0)
            self.draw_pixel(x, y1)
        for y in range(min(y0, y1), max(y0, y1) + 1):
            self.draw_pixel(x0, y)
            self.draw_pixel(x1, y)

    def draw_filled_box(self, start, end):
        x0, y0 = start
        x1, y1 = end
        for y in range(min(y0, y1), max(y0, y1) + 1):
            for x in range(min(x0, x1), max(x0, x1) + 1):
                self.draw_pixel(x, y)


    def clear(self):
        self.pixel_grid = [[0 for _ in range(SCREEN_WIDTH)] for _ in range(SCREEN_HEIGHT)]
        self.draw()

    def export(self):
        from tkinter.filedialog import asksaveasfilename

        filename = asksaveasfilename(
            defaultextension=".ino",
            filetypes=[("C++ Source Files", "*.ino"), ("All Files", "*.*")],
            title="Export Drawing Commands"
        )
        if not filename:
            return

        visited = [[False for _ in range(SCREEN_WIDTH)] for _ in range(SCREEN_HEIGHT)]

        def is_pixel(x, y):
            return (
                0 <= x < SCREEN_WIDTH and
                0 <= y < SCREEN_HEIGHT and
                self.pixel_grid[y][x] == 1 and
                not visited[y][x]
            )

        with open(filename, "w") as f:
            f.write("// Exported drawing commands\n")

            # Horizontal lines
            for y in range(SCREEN_HEIGHT):
                x = 0
                while x < SCREEN_WIDTH:
                    if is_pixel(x, y):
                        start_x = x
                        while x < SCREEN_WIDTH and is_pixel(x, y):
                            visited[y][x] = True
                            x += 1
                        length = x - start_x
                        if length == 1:
                            f.write(f"display.drawPixel({start_x}, {y}, SH110X_WHITE);\n")
                        else:
                            f.write(f"display.drawFastHLine({start_x}, {y}, {length}, SH110X_WHITE);\n")
                    else:
                        x += 1

            # Vertical lines
            for x in range(SCREEN_WIDTH):
                y = 0
                while y < SCREEN_HEIGHT:
                    if is_pixel(x, y):
                        start_y = y
                        while y < SCREEN_HEIGHT and is_pixel(x, y):
                            visited[y][x] = True
                            y += 1
                        length = y - start_y
                        if length == 1:
                            f.write(f"display.drawPixel({x}, {start_y}, SH110X_WHITE);\n")
                        else:
                            f.write(f"display.drawFastVLine({x}, {start_y}, {length}, SH110X_WHITE);\n")
                    else:
                        y += 1


    def draw(self):
        self.canvas.delete("all")

        # -------- Menu/help bar content (below the grid) --------
        help_y = SCREEN_HEIGHT * SCALE + 8  # below the bottom edge of the grid

        # Draw color swatch
        swatch_color = "#ffffff" if self.current_color else "#000000"
        self.canvas.create_rectangle(
            SCREEN_WIDTH * SCALE - 30, help_y,
            SCREEN_WIDTH * SCALE - 6, help_y + 20,
            fill=swatch_color, outline="#888888"
        )
        self.canvas.create_text(
            SCREEN_WIDTH * SCALE - 35, help_y,
            anchor="ne", text="Color:", fill="#888888", font=("Arial", 10)
        )

        # Draw help text
        help_text = "T: Text | S: String | L: Line | B: Box | F: Filled Box | P: Pixel | C: Clear | E: Export | D: Toggle Color | ESC: Quit"
        self.canvas.create_text(
            4, help_y,
            text=help_text, fill="#888888", anchor='nw', font=("Arial", 8)
        )

        for y in range(SCREEN_HEIGHT):
            for x in range(SCREEN_WIDTH):
                if self.pixel_grid[y][x]:
                    self.canvas.create_rectangle(
                        x * SCALE, y * SCALE,
                        (x + 1) * SCALE, (y + 1) * SCALE,
                        fill=PIXEL_COLOR, outline=PIXEL_COLOR
                    )

        # Draw rubber-band preview
        if self.start_pos and self.preview_end:
            x0, y0 = self.start_pos
            x1, y1 = self.preview_end
            color="#00ff00"
            if self.mode == "LINE":
                self.canvas.create_line(x0 * SCALE, y0 * SCALE, x1 * SCALE, y1 * SCALE, fill=color)
            elif self.mode in ("BOX", "FILLBOX"):
                self.canvas.create_rectangle(
                    min(x0, x1) * SCALE, min(y0, y1) * SCALE,
                    (max(x0, x1) + 1) * SCALE, (max(y0, y1) + 1) * SCALE,
                    fill=color
                )

        # Draw grid
        for x in range(SCREEN_WIDTH):
            self.canvas.create_line(x*SCALE, 0, x*SCALE, SCREEN_HEIGHT*SCALE, fill=GRID_COLOR)
        for y in range(SCREEN_HEIGHT+1):
            self.canvas.create_line(0, y*SCALE, SCREEN_WIDTH*SCALE, y*SCALE, fill=GRID_COLOR)

        if self.mode == "STRING" and self.string_cursor_pos and self.cursor_visible:
            cx, cy = self.string_cursor_pos
            self.canvas.create_rectangle(
                cx * SCALE, cy * SCALE,
                (cx + CHAR_WIDTH) * SCALE, (cy + CHAR_HEIGHT) * SCALE,
                fill="#00ff00", outline=""
            )


if __name__ == "__main__":
    root = tk.Tk()
    root.title("SH1106 OLED Editor")
    app = SH1106Editor(root)
    root.mainloop()

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
        self.action_log = []
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
        elif key == "I":
            self.import_drawing()
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

    def draw_pixel(self, x, y, true_pixel=True):
        if 0 <= x < SCREEN_WIDTH and 0 <= y < SCREEN_HEIGHT:
            self.pixel_grid[y][x] = self.current_color
            if true_pixel:
                self.action_log.append(("PIXEL", x, y, self.current_color))

    def draw_char(self, ch, x, y):
        font = FONT_5X7.get(ch.upper())
        if not font:
            return
        self.action_log.append(("CHAR", ch, x, y, self.current_color))
        for row, bits in enumerate(font):
            for col in range(5):
                if bits & (1 << (4 - col)):
                    self.draw_pixel(x + col, y + row, False)

    def draw_line(self, start, end):
        x0, y0 = start
        x1, y1 = end
        dx, dy = abs(x1 - x0), abs(y1 - y0)
        sx, sy = (1 if x0 < x1 else -1), (1 if y0 < y1 else -1)
        err = dx - dy
        self.action_log.append(("LINE", start, end, self.current_color))
        while True:
            self.draw_pixel(x0, y0, False)
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
        self.action_log.append(("BOX", start, end, self.current_color))
        for x in range(min(x0, x1), max(x0, x1) + 1):
            self.draw_pixel(x, y0, False)
            self.draw_pixel(x, y1, False)
        for y in range(min(y0, y1), max(y0, y1) + 1):
            self.draw_pixel(x0, y, False)
            self.draw_pixel(x1, y, False)

    def draw_filled_box(self, start, end):
        x0, y0 = start
        x1, y1 = end
        self.action_log.append(("FILLBOX", start, end, self.current_color))
        for y in range(min(y0, y1), max(y0, y1) + 1):
            for x in range(min(x0, x1), max(x0, x1) + 1):
                self.draw_pixel(x, y, False)

    def clear(self):
        self.pixel_grid = [[0 for _ in range(SCREEN_WIDTH)] for _ in range(SCREEN_HEIGHT)]
        self.draw()

    def import_drawing(self):
        from tkinter.filedialog import askopenfilename
        import re

        filename = askopenfilename(
            defaultextension=".ino",
            filetypes=[("C++ Source Files", "*.ino"), ("All Files", "*.*")],
            title="Import Drawing Commands"
        )
        if not filename:
            return

        self.clear()
        self.action_log.clear()

        cursor_x, cursor_y = 0, 0

        with open(filename, "r") as f:
            for line in f:
                line = line.strip()
                if m := re.match(r"display\.drawPixel\((\d+), (\d+), SH110X_(WHITE|BLACK)\);", line):
                    x, y, c = int(m[1]), int(m[2]), 1 if m[3] == "WHITE" else 0
                    self.current_color = c
                    self.draw_pixel(x, y)
                elif m := re.match(r"display\.drawLine\((\d+), (\d+), (\d+), (\d+), SH110X_(WHITE|BLACK)\);", line):
                    s = (int(m[1]), int(m[2]))
                    e = (int(m[3]), int(m[4]))
                    self.current_color = 1 if m[5] == "WHITE" else 0
                    self.draw_line(s, e)
                elif m := re.match(r"display\.drawRect\((\d+), (\d+), (\d+), (\d+), SH110X_(WHITE|BLACK)\);", line):
                    x, y, w, h = map(int, m.groups()[:4])
                    self.current_color = 1 if m[5] == "WHITE" else 0
                    self.draw_box((x, y), (x + w - 1, y + h - 1))
                elif m := re.match(r"display\.fillRect\((\d+), (\d+), (\d+), (\d+), SH110X_(WHITE|BLACK)\);", line):
                    x, y, w, h = map(int, m.groups()[:4])
                    self.current_color = 1 if m[5] == "WHITE" else 0
                    self.draw_filled_box((x, y), (x + w - 1, y + h - 1))
                elif m := re.match(r"display\.setCursor\((\d+), (\d+)\);", line):
                    cursor_x, cursor_y = int(m[1]), int(m[2])
                elif m := re.match(r"display\.setTextColor\(SH110X_(WHITE|BLACK)\);", line):
                    self.current_color = 1 if m[1] == "WHITE" else 0
                elif m := re.match(r'display\.print\("(.+?)"\);', line):
                    for i, ch in enumerate(m[1]):
                        self.draw_char(ch, cursor_x + i * CHAR_WIDTH, cursor_y)

    def export(self):
        from tkinter.filedialog import asksaveasfilename
        emitted_text_size = False

        filename = asksaveasfilename(
            defaultextension=".ino",
            filetypes=[("C++ Source Files", "*.ino"), ("All Files", "*.*")],
            title="Export Drawing Commands"
        )
        if not filename:
            return

        with open(filename, "w") as f:
            f.write("// Exported drawing actions\n")
            for action in self.action_log:
                cmd = action[0]
                if cmd == "PIXEL":
                    _, x, y, color = action
                    f.write(f"display.drawPixel({x}, {y}, {'SH110X_WHITE' if color else 'SH110X_BLACK'});\n")
                elif cmd == "LINE":
                    _, start, end, color = action
                    x0, y0 = start
                    x1, y1 = end
                    f.write(
                        f"display.drawLine({x0}, {y0}, {x1}, {y1}, {'SH110X_WHITE' if color else 'SH110X_BLACK'});\n")
                elif cmd == "BOX":
                    _, start, end, color = action
                    x0, y0 = start
                    x1, y1 = end
                    width = abs(x1 - x0)
                    height = abs(y1 - y0)
                    f.write(
                        f"display.drawRect({min(x0, x1)}, {min(y0, y1)}, {width + 1}, {height + 1}, {'SH110X_WHITE' if color else 'SH110X_BLACK'});\n")
                elif cmd == "FILLBOX":
                    _, start, end, color = action
                    x0, y0 = start
                    x1, y1 = end
                    width = abs(x1 - x0)
                    height = abs(y1 - y0)
                    f.write(
                        f"display.fillRect({min(x0, x1)}, {min(y0, y1)}, {width + 1}, {height + 1}, {'SH110X_WHITE' if color else 'SH110X_BLACK'});\n")
                elif cmd == "CHAR":
                    _, ch, x, y, color = action
                    f.write(f"display.setCursor({x}, {y});\n")
                    if emitted_text_size is False:
                        f.write(f"display.setTextSize(1);\n")
                        emitted_text_size = True
                    f.write(f"display.setTextColor({'SH110X_WHITE' if color else 'SH110X_BLACK'});\n")
                    f.write(f"display.print(\"{ch}\");\n")

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
        help_text = "T: Text | S: String | L: Line | B: Box | F: Filled Box | P: Pixel | C: Clear | E: Export | I: Import | D: Toggle Color | ESC: Quit"
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
            color = "#00ff00"
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
            self.canvas.create_line(x * SCALE, 0, x * SCALE, SCREEN_HEIGHT * SCALE, fill=GRID_COLOR)
        for y in range(SCREEN_HEIGHT + 1):
            self.canvas.create_line(0, y * SCALE, SCREEN_WIDTH * SCALE, y * SCALE, fill=GRID_COLOR)

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

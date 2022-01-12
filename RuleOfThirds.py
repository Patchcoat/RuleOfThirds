import tkinter as tk
import win32gui
import win32con
import win32api
import re

class TransparentWin(tk.Tk):
	""" Transparent Tkinter Window Class. """

	def __init__(self):
		tk.Tk.__init__(self)

		# Sets focus to the window.
		self.focus_force()

		# Removes the native window boarder.
		self.overrideredirect(True)

		# Disables resizing of the widget.
		self.resizable(False, False)

		# Places window above all other windows in the window stack.
		self.wm_attributes("-topmost", True)

		# This changes the alpha value (How transparent the window should be).
		# It ranges from 0.0 (completely transparent) to 1.0 (completely opaque).
		self.attributes("-transparentcolor", '#F0F')
		self.attributes("-topmost", 1)
		self.attributes("-alpha", 0.3)
		self.config(bg='#F0F')

		# The windows overall position on the screen
		self.wm_geometry('+' + str(0) + '+' + str(0))

		screenWidth = self.winfo_screenwidth()
		screenHeight = self.winfo_screenheight()
		linewidth = 3
		lineColor = "yellow"

		canvas = tk.Canvas(self, bg='#F0F', width=screenWidth, height=screenHeight)
		canvas.create_line(screenWidth/3, 0, screenWidth/3, screenHeight, width=linewidth, fill=lineColor)
		canvas.create_line(screenWidth*2/3, 0, screenWidth*2/3, screenHeight, width=linewidth, fill=lineColor)
		canvas.create_line(0, screenHeight/3, screenWidth, screenHeight/3, width=linewidth, fill=lineColor)
		canvas.create_line(0, screenHeight*2/3, screenWidth, screenHeight*2/3, width=linewidth, fill=lineColor)

		canvas.pack(fill=tk.BOTH, expand=1)

		# Set Clickthrough
		# Get window style and perform a 'bitwise or' operation to make the style layered and transparent, achieving
		# the clickthrough property
		hwnd = canvas.winfo_id()
		styles = win32gui.GetWindowLong(hwnd, win32con.GWL_EXSTYLE)
		styles |= win32con.WS_EX_LAYERED | win32con.WS_EX_TRANSPARENT
		win32gui.SetWindowLong(hwnd, win32con.GWL_EXSTYLE, styles)
		win32gui.SetLayeredWindowAttributes(hwnd, win32api.RGB(255,0,255), 255, win32con.LWA_COLORKEY)

	def exit(self, event):
		self.destroy()

	def position(self):
		_filter = re.compile(r"(\d+)?x?(\d+)?([+-])(\d+)([+-])(\d+)")
		pos = self.winfo_geometry()
		filtered = _filter.search(pos)
		self.X = int(filtered.group(4))
		self.Y = int(filtered.group(6))

		return self.X, self.Y

def __run__():
	TransparentWin().mainloop()


if __name__ == '__main__':
	__run__()
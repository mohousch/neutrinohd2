from neutrino import *

class main(CHelpBox):
	line1 = "Test"
	line2 = "Huhu"
	line3 = "alles gut"
	def __init__(self):
		CHelpBox.__init__(self)
		self.addLine(self.line1)
		self.addLine(self.line2)
		self.addLine(self.line3)
		self.show("python: CHelpBox")
	
if __name__ == "__main__":
	main()











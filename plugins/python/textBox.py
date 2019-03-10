from neutrino import *

class main(CTextBox):
	msgText = "first test\ntesting CMessageBox\nthat's all Folk!"
	def __init__(self):
		CTextBox.__init__(self)
		self.setText(self.msgText)
		self.paint()
		CFrameBuffer.getInstance().blit()

		msg = -1
		loop = True
		bigFonts = False

		while loop:
			msg = CSwigHelpers().getRCCode()
			#data = CSwigHelpers().getRCData()

			if msg == RC_info:
				if bigFonts == False : 
					bigFonts = True
				elif bigFonts == True:
					bigFonts = False
				self.setBigFonts(bigFonts)
				self.refresh()

			if msg == RC_home:
				self.hide()
				CFrameBuffer.getInstance().blit()
				loop = False
				break

if __name__ == "__main__":
	main()











from neutrino import *

class main(CHeaders):
	def __init__(self):
		CHeaders.__init__(self)

		buttons = button_label_struct()
		buttons.button = NEUTRINO_ICON_BUTTON_HELP
		buttons.locale = NONEXISTANT_LOCALE
		buttons.localename = None

		fButtons = button_label_struct()
		fButtons.button = NEUTRINO_ICON_BUTTON_RED
		fButtons.locale = NONEXISTANT_LOCALE
		fButtons.localename = "red"
		
		self.enablePaintDate()
		self.enableLogo()

		self.setHeaderButtons(buttons, 1)

		self.paintHead(150, 50, 750, 40, "pythonTest", NEUTRINO_ICON_MOVIE)
		self.paintFoot(150, 570, 750, 40, 1, fButtons)

		CSwigHelpers().blit()

		SEC_TIMER_ID = 0
		SEC_TIMER_ID = CSwigHelpers().addTimer(1*1000*1000, False);

		msg = -1
		data = -1
		loop = True

		while loop:
			msg = CSwigHelpers().getRCCode()
			data = CSwigHelpers().getRCData()

			if msg == NeutrinoMessages().EVT_TIMER and data == SEC_TIMER_ID:
				self.paintHead(150, 50, 750, 40, "pythonTest", NEUTRINO_ICON_MOVIE)

			elif msg == RC_home:
				loop = False
				break

		CFrameBuffer.getInstance().paintBackground()

if __name__ == "__main__":
	main()











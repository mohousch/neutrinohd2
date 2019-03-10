from neutrino import *

class main(ClistBoxWidget):
	def __init__(self):
		ClistBoxWidget.__init__(self, "pythonTest", NEUTRINO_ICON_MOVIE)
		self.setWidgetType(WIDGET_TYPE_CLASSIC)
		self.addWidget(WIDGET_TYPE_FRAME)
		self.enableWidgetChange()

		self.enablePaintDate()

		item1 = ClistBoxItem("item1", True, None, self, "red action")
		item2 = ClistBoxItem("item2")
		item3 = ClistBoxItem("item3")
		item4 = ClistBoxItem("item4")

		self.addItem(item1)
		self.addItem(item2)
		self.addItem(item3)
		self.addItem(item4)

		self._exec(None, "")

		selected = -1
		selected = self.getSelected()
		self.hide()

		if selected == 0:
			messageBox()
		elif selected == 1:
			audioPlayerGui()
		elif selected == 2:
			moviePlayerGui()
		elif selected == 3:
			pictureViewerGui()

		#self._exec(None, "")

if __name__ == "__main__":
	main()











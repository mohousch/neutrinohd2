from neutrino import *

class main(CMessageBox):
	msgTitle = "pythonTest:"
	msgText = "first test\ntesting CMessageBox\nthat's all Folk!"
	def __init__(self):
		CMessageBox.__init__(self, self.msgTitle, self.msgText)
		self._exec(-1)

if __name__ == "__main__":
	main()











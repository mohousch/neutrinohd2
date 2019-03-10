from neutrino import *

class main(CFileBrowser):
	settings = SNeutrinoSettings()
	PATH = settings.network_nfs_moviedir
	def __init__(self):
		CFileBrowser.__init__(self)
		#self._exec(self.PATH)
		fileFilter = CFileFilter()
		fileFilter.addFilter("ts")

		self.Multi_Select = False
		self.Dirs_Selectable = False
		self.Filter = fileFilter

		msg = CSwigHelpers().getRCCode()

		while msg != RC_home:
			self._exec(self.PATH)
			self.PATH = self.getCurrentDir()
		
			cFile = self.getSelectedFile()

			moviePlayer = CMoviePlayerGui()
		
			moviePlayer.addToPlaylist(cFile);
			moviePlayer._exec(None, "");

if __name__ == "__main__":
	main()











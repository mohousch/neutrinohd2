#include <plugin.h>

 
extern "C" void plugin_exec(void);
extern "C" void plugin_init(void);
extern "C" void plugin_del(void);


//
void plugin_init(void)
{
}

void plugin_del(void)
{
}

void plugin_exec(void)
{
	if(MessageBox(LOCALE_MESSAGEBOX_INFO, "do you want really to restart enigma2?", CMessageBox::mbrYes, CMessageBox::mbYes | CMessageBox::mbNo, NULL, 450, 30, true) == CMessageBox::mbrYes)
					
	{
		system("rm -f /etc/.nhd2");
		system("init 4");
		system("init 3");
	}
}



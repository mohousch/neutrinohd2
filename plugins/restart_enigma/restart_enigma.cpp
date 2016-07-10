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
	system("rm -f /etc/.nhd2");
	system("touch /etc/.e2");
	
	g_RCInput->postMsg( NeutrinoMessages::RESTART, 0 );
	HintBox(LOCALE_MESSAGEBOX_INFO, "enigma2 wird gestartet !");
}



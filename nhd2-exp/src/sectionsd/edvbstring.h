#ifndef __E_STRING__
#define __E_STRING__


int readEncodingFile();
std::string convertDVBUTF8(const char *data, int len, int table, int tsidonid = 0);
std::string UTF8_to_Latin1(const char * s);

#endif // __E_STRING__

#ifndef __E_STRING__
#define __E_STRING__


int readEncodingFile();
std::string convertDVBUTF8(const char *data, int len, int table, int tsidonid = 0);

#endif // __E_STRING__

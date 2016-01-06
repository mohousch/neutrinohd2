/*
	Neutrino-HD

	License: GPL

	(C) 2012-2013 the neutrino-hd developers
	(C) 2012,2013 Stefan Seyfried

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 2 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program; if not, write to the Free Software
	Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <iostream>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/vfs.h>    /* or <sys/statfs.h> */
#include <sys/time.h>	/* gettimeofday */
#include <string.h>
#include <fcntl.h>
#include <dirent.h>
#include <stdarg.h>
#include <sys/stat.h>

#include <system/helpers.h>
#include <system/debug.h>
#include <system/settings.h>

#include <global.h>


#if defined (__USE_FILE_OFFSET64) || defined (_DARWIN_USE_64_BIT_INODE)
typedef struct dirent64 dirent_struct;
#define my_alphasort alphasort64
#define my_scandir scandir64
typedef struct stat64 stat_struct;
#define my_stat stat64
#define my_lstat lstat64
#else
typedef struct dirent dirent_struct;
#define my_alphasort alphasort
#define my_scandir scandir
typedef struct stat stat_struct;
#define my_stat stat
#define my_lstat lstat
#error not using 64 bit file offsets
#endif


off_t file_size(const char *filename)
{
	struct stat stat_buf;
	if(::stat(filename, &stat_buf) == 0)
	{
		return stat_buf.st_size;
	} else
	{
		return 0;
	}
}

bool file_exists(const char *filename)
{
	struct stat stat_buf;
	if(::stat(filename, &stat_buf) == 0)
	{
		return true;
	} else
	{
		return false;
	}
}

void  wakeup_hdd(const char *hdd_dir)
{
	if(!check_dir(hdd_dir)){
		std::string wakeup_file = hdd_dir;
		wakeup_file += "/.wakeup";
		remove(wakeup_file.c_str());
		creat(wakeup_file.c_str(),S_IREAD|S_IWRITE);
		sync();
	}
}
//use for script with full path
int my_system(const char * cmd)
{
	if (!file_exists(cmd))
		return -1;
	
	dprintf(DEBUG_NORMAL, "my_system: execute:%s\n", cmd);

	return my_system(1, cmd);
}

int my_system(int argc, const char *arg, ...)
{
	int i = 0, ret = 0, childExit = 0;
#define ARGV_MAX 64
	/* static right now but could be made dynamic if necessary */
	int argv_max = ARGV_MAX;
	const char *argv[ARGV_MAX];
	va_list args;
	argv[0] = arg;
	va_start(args, arg);

	while(++i < argc)
	{
		if (i == argv_max)
		{
			fprintf(stderr, "my_system: too many arguments!\n");
			return -1;
		}
		argv[i] = va_arg(args, const char *);
	}
	argv[i] = NULL; /* sentinel */
	//fprintf(stderr,"%s:", __func__);for(i=0;argv[i];i++)fprintf(stderr," '%s'",argv[i]);fprintf(stderr,"\n");

	pid_t parent_pid = getpgrp();
	pid_t pid;
	int maxfd = getdtablesize();// sysconf(_SC_OPEN_MAX);
	switch (pid = vfork())
	{
		case -1: /* can't vfork */
			perror("vfork");
			ret = -errno;
			break;
		case 0: /* child process */
			for(i = 3; i < maxfd; i++)
				close(i);
#if 0
			if (setsid() == -1)
				perror("my_system setsid");
#endif
			setpgid(getpid(), parent_pid);
			if (execvp(argv[0], (char * const *)argv))
			{
				ret = -errno;
				if (errno != ENOENT) /* don't complain if argv[0] only does not exist */
					fprintf(stderr, "ERROR: my_system \"%s\": %m\n", argv[0]);
			}
			_exit(ret); // terminate c h i l d proces s only
		default: /* parent returns to calling process */
			waitpid(pid, &childExit, 0);
			if (WEXITSTATUS(childExit) != 0)
				ret = (signed char)WEXITSTATUS(childExit);
			break;
	}
	va_end(args);
	return ret;
}

FILE* my_popen( pid_t& pid, const char *cmdstring, const char *type)
{
	int     pfd[2] ={-1,-1};
	FILE    *fp = NULL;

	/* only allow "r" or "w" */
	if ((type[0] != 'r' && type[0] != 'w') || type[1] != 0) {
		errno = EINVAL;     /* required by POSIX */
		return(NULL);
	}

	if (pipe(pfd) < 0)
		return(NULL);   /* errno set by pipe() */

	if ((pid = vfork()) < 0) {
		return(NULL);   /* errno set by vfork() */
	} else if (pid == 0) {                           /* child */
		if (*type == 'r') {
			close(pfd[0]);
			if (pfd[1] != STDOUT_FILENO) {
				dup2(pfd[1], STDOUT_FILENO);
				close(pfd[1]);
			}
		} else {
			close(pfd[1]);
			if (pfd[0] != STDIN_FILENO) {
				dup2(pfd[0], STDIN_FILENO);
				close(pfd[0]);
			}
		}
		int maxfd = getdtablesize();
		for(int i = 3; i < maxfd; i++)
			close(i);
#if 0
		if (setsid() == -1)
			perror("my_popen setsid");
#endif
		execl("/bin/sh", "sh", "-c", cmdstring, (char *)0);
		exit(0);
	 }

	/* parent continues... */
	if (*type == 'r') {
		close(pfd[1]);
		if ((fp = fdopen(pfd[0], type)) == NULL)
			return(NULL);
		} else {
			close(pfd[0]);
		if ((fp = fdopen(pfd[1], type)) == NULL)
			return(NULL);
	}
	return(fp);
}

int safe_mkdir(const char * path)
{
	struct statfs s;
	size_t l = strlen(path);
	char d[l + 3];
	strncpy(d, path, l);

	// skip trailing slashes
	while (l > 0 && d[l - 1] == '/')
		l--;
	// find last slash
	while (l > 0 && d[l - 1] != '/')
		l--;
	if (!l)
		return -1;
	// append a single dot
	d[l++] = '.';
	d[l] = 0;

	if(statfs(d, &s) || (s.f_type == 0x72b6 /* JFFS2 */) || (s.f_type == 0x5941ff53 /* YAFFS2 */))
		return -1;
	return mkdir(path, 0755);
}

/* function used to check is this dir writable, i.e. not flash, for record etc */
int check_dir(const char * dir, bool allow_tmp)
{
	/* default to return, if statfs fail */
	int ret = -1;
	struct statfs s;
	if (::statfs(dir, &s) == 0) {
		switch (s.f_type)	/* f_type is long */
		{
			case 0xEF53L:		/*EXT2 & EXT3*/
			case 0x6969L:		/*NFS*/
			case 0xFF534D42L:	/*CIFS*/
			case 0x517BL:		/*SMB*/
			case 0x52654973L:	/*REISERFS*/
			case 0x65735546L:	/*fuse for ntfs*/
			case 0x58465342L:	/*xfs*/
			case 0x4d44L:		/*msdos*/
			case 0x0187:		/* AUTOFS_SUPER_MAGIC */
			case 0x3153464aL:	/*jfs*/
			case 0x4006L:		/*fat*/
#if 0
			case 0x72b6L:		/*jffs2*/
#endif
				ret = 0;//ok
				break; 
			case 0x858458f6L: 	/*ramfs*/
			case 0x1021994: 	/*TMPFS_MAGIC*/
				if(allow_tmp)
					ret = 0;//ok
				break;
			default:
				fprintf(stderr, "%s: Unknown File system type 0x%x\n", dir, (int)s.f_type);
				break; // error
		}
	}
	return ret;
}

bool get_fs_usage(const char * dir, uint64_t &btotal, uint64_t &bused, long *bsize/*=NULL*/)
{
	btotal = bused = 0;
	struct statfs s;

	if (::statfs(dir, &s) == 0 && s.f_blocks) {
		btotal = s.f_blocks;
		bused = s.f_blocks - s.f_bfree;
		if (bsize != NULL)
			*bsize = s.f_bsize;
		//printf("fs (%s): total %llu used %llu\n", dir, btotal, bused);
		return true;
	}
	return false;
}

bool get_mem_usage(unsigned long &kbtotal, unsigned long &kbfree)
{
	unsigned long cached = 0, buffers = 0;
	kbtotal = kbfree = 0;

	FILE * f = fopen("/proc/meminfo", "r");
	if (!f)
		return false;

	char buffer[256];
	while (fgets(buffer, 255, f)) {
		if (!strncmp(buffer, "Mem", 3)) {
			if (!strncmp(buffer+3, "Total", 5))
				kbtotal = strtoul(buffer+9, NULL, 10);
			else if (!strncmp(buffer+3, "Free", 4))
				kbfree = strtoul(buffer+8, NULL, 10);
		}
		else if (!strncmp(buffer, "Buffers", 7)) {
			buffers = strtoul(buffer+8, NULL, 10);
		}
		else if (!strncmp(buffer, "Cached", 6)) {
			cached = strtoul(buffer+7, NULL, 10);
			break;
		}
	}
	fclose(f);
	kbfree = kbfree + cached + buffers;
	printf("mem: total %ld cached %ld free %ld\n", kbtotal, cached, kbfree);
	return true;
}

std::string _getPathName(std::string &path, std::string sep)
{
	size_t pos = path.find_last_of(sep);
	if (pos == std::string::npos)
		return path;
	return path.substr(0, pos);
}

std::string _getBaseName(std::string &path, std::string sep)
{
	size_t pos = path.find_last_of(sep);
	if (pos == std::string::npos)
		return path;
	if (path.length() == pos +1)
		return "";
	return path.substr(pos+1);
}

std::string getPathName(std::string &path)
{
	return _getPathName(path, "/");
}

std::string getBaseName(std::string &path)
{
	return _getBaseName(path, "/");
}

std::string getFileName(std::string &file)
{
	return _getPathName(file, ".");
}

std::string getFileExt(std::string &file)
{
	return _getBaseName(file, ".");
}


std::string getNowTimeStr(const char* format)
{
	char tmpStr[256];
	struct timeval tv;
	gettimeofday(&tv, NULL);        
	strftime(tmpStr, sizeof(tmpStr), format, localtime(&tv.tv_sec));
	
	return (std::string)tmpStr;
}

std::string trim(std::string &str, const std::string &trimChars /*= " \n\r\t"*/)
{
	std::string result = str.erase(str.find_last_not_of(trimChars) + 1);
	return result.erase(0, result.find_first_not_of(trimChars));
}

std::string replace_all(const std::string &in, const std::string &entity, const std::string &symbol)
{
	std::string out = in;
	std::string::size_type loc = 0;
	while (( loc = out.find(entity, loc)) != std::string::npos )
	out.replace(loc, entity.length(), symbol);
	
	return out;
}

void strReplace(std::string & orig, const char *fstr, const std::string rstr)
{
	//replace all occurrence of fstr with rstr and, and returns a reference to itself
	size_t index = 0;
	size_t fstrlen = strlen(fstr);
	size_t rstrlen = rstr.size();

	while ((index = orig.find(fstr, index)) != std::string::npos) 
	{
		orig.replace(index, fstrlen, rstr);
		index += rstrlen;
	}
}

std::string& htmlEntityDecode(std::string& text, bool removeTags)
{
	struct decode_table_s {
		const char* utf8Code;
		const char* htmlCode;
		const unsigned int htmlCodeLen;
	};

	decode_table_s dt[] = {
		{ " ",  		"&nbsp;",	6 },
		{ "&",  		"&amp;",	5 },
		{ "<",  		"&lt;",		4 },
		{ ">",  		"&gt;",		4 },
		{ "\"", 		"&quot;",	6 },
		{ "'",  		"&apos;",	6 },
		{ "€",  		"&euro;",	6 },
		{ "\xe2\x80\xa6",	"&hellip;",	8 },
		{ NULL,			NULL,		0 }
	};

	char t[text.length() + 1];
	char *u = t;
	const char *p = text.c_str();
	while (*p) {
		if (removeTags && *p == '<') {
			while (*p && *p != '>')
				p++;
			if (*p)
				p++;
			continue;
		}
		if (p[0] == '&') {
			if (p[1] == '#') {
				p += 2;
				const char *format;
				if (p[0] == 'x' || p[0] == 'X') {
					format = "%x";
					p++;
				} else
					format = "%d";
				unsigned int r = 0;
				if (1 == sscanf(p, format, &r)) {
					if (r < 0x80) {
						u[0] = r & 0x7f;
						u += 1;
					} else if (r < 0x800) {
						u[1] = 0x80 & (r & 0x3f);
						r >>= 6;
						u[0] = 0xC0 & r;
						u += 2;
					} else if (r < 0x10000) {
						u[2] = 0x80 | (0x3f & r);
						r >>= 6;
						u[1] = 0x80 | (0x3f & r);
						r >>= 6;
						u[0] = 0xE0 | r;
						u += 3;
					} else if (r < 0x110000) {
						u[3] = 0x80 | (0x3f & r);
						r >>= 6;
						u[2] = 0x80 | (0x3f & r);
						r >>= 6;
						u[1] = 0x80 | (0x3f & r);
						r >>= 6;
						u[0] = 0xF0 | r;
						u += 4;
					}
				}
				while(*p && *p != ';')
					p++;
				if (*p)
					p++;
				continue;
			}

			decode_table_s *d = dt;
			while (d->utf8Code && strncmp(p, d->htmlCode, d->htmlCodeLen))
				d++;
			if (d->utf8Code) {
				p += d->htmlCodeLen;
				const char *v = d->utf8Code;
				while (*v)
					*u++ = *v++;
				continue;
			}
		}

		*u++ = *p++;
	}
	
	*u = 0;
	text = std::string(t);
	
	return text;
}	

unsigned long long getcurrenttime()
{
	struct timeval tv;
	gettimeofday( &tv, NULL );
	return (unsigned long long) tv.tv_usec + (unsigned long long)((unsigned long long) tv.tv_sec * (unsigned long long) 1000000);
}

std::string to_string(int i)
{
	std::stringstream s;
	s << i;
	return s.str();
}

std::string to_string(unsigned int i)
{
	std::stringstream s;
	s << i;
	return s.str();
}

std::string to_string(long i)
{
	std::stringstream s;
	s << i;
	return s.str();
}

std::string to_string(unsigned long i)
{
	std::stringstream s;
	s << i;
	return s.str();
}

std::string to_string(long long i)
{
	std::stringstream s;
	s << i;
	return s.str();
}

std::string to_string(unsigned long long i)
{
	std::stringstream s;
	s << i;
	return s.str();
}

std::string to_hexstring(unsigned long long i)
{
	std::stringstream s;
	s.setf ( std::ios::hex, std::ios::basefield );
	s << i;
	return s.str();
}

std::string changeFileNameExt(std::string &filename, const char *ext)
{
	int ext_pos = 0;
	ext_pos = filename.rfind('.');
	if( ext_pos > 0)
	{
		std::string extension;
		extension = filename.substr(ext_pos + 1, filename.length() - ext_pos);
		extension = "." + extension;
		strReplace(filename, extension.c_str(), ext);
	}
	
	return filename;
}

size_t CurlWriteToString(void *ptr, size_t size, size_t nmemb, void *data)
{
        std::string* pStr = (std::string*) data;
        pStr->append((char*) ptr, nmemb);
	
        return size*nmemb;
}

bool getUrl(std::string &url, std::string &answer, std::string& userAgent)
{
	CURL * curl_handle = curl_easy_init();
	curl_easy_setopt(curl_handle, CURLOPT_URL, url.c_str());
	curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, &CurlWriteToString);
	curl_easy_setopt(curl_handle, CURLOPT_FILE, (void *)&answer);
	curl_easy_setopt(curl_handle, CURLOPT_FAILONERROR, 1);
	curl_easy_setopt(curl_handle, CURLOPT_TIMEOUT, 60);
	curl_easy_setopt(curl_handle, CURLOPT_NOSIGNAL, (long)1);
	curl_easy_setopt(curl_handle, CURLOPT_SSL_VERIFYPEER, false);
	curl_easy_setopt(curl_handle, CURLOPT_USERAGENT, userAgent.c_str());
	
	if(strcmp(g_settings.softupdate_proxyserver, "")!=0)
	{
		curl_easy_setopt(curl_handle, CURLOPT_PROXY, g_settings.softupdate_proxyserver);
		
		if(strcmp(g_settings.softupdate_proxyusername, "") != 0)
		{
			char tmp[200];
			strcpy(tmp, g_settings.softupdate_proxyusername);
			strcat(tmp, ":");
			strcat(tmp, g_settings.softupdate_proxypassword);
			curl_easy_setopt(curl_handle, CURLOPT_PROXYUSERPWD, tmp);
		}
	}

	char cerror[CURL_ERROR_SIZE];
	curl_easy_setopt(curl_handle, CURLOPT_ERRORBUFFER, cerror);

	dprintf(DEBUG_NORMAL, "cYTFeedParser::getUrl: try to get %s\n", url.c_str());
	
	CURLcode httpres = curl_easy_perform(curl_handle);

	curl_easy_cleanup(curl_handle);

	dprintf(DEBUG_NORMAL, "cYTFeedParser::getUrl: http: res %d size %d\n", httpres, (int)answer.size());

	if (httpres != 0 || answer.empty()) 
	{
		dprintf(DEBUG_NORMAL, "cYTFeedParser::getUrl: error: %s\n", cerror);
		return false;
	}
	
	return true;
}

bool DownloadUrl(std::string &url, std::string &file, std::string& userAgent)
{
	FILE * fp = fopen(file.c_str(), "wb");
	if (fp == NULL) 
	{
		perror(file.c_str());
		return false;
	}
	
	CURL * curl_handle = curl_easy_init();
	curl_easy_setopt(curl_handle, CURLOPT_URL, url.c_str());
	curl_easy_setopt(curl_handle, CURLOPT_FILE, fp);
	curl_easy_setopt(curl_handle, CURLOPT_FAILONERROR, 1);
	curl_easy_setopt(curl_handle, CURLOPT_TIMEOUT, 60);
	curl_easy_setopt(curl_handle, CURLOPT_NOSIGNAL, (long)1);
	curl_easy_setopt(curl_handle, CURLOPT_SSL_VERIFYPEER, false);
	curl_easy_setopt(curl_handle, CURLOPT_USERAGENT, userAgent.c_str());
	
	if(strcmp(g_settings.softupdate_proxyserver, "")!=0)
	{
		curl_easy_setopt(curl_handle, CURLOPT_PROXY, g_settings.softupdate_proxyserver);
		
		if(strcmp(g_settings.softupdate_proxyusername, "") != 0)
		{
			char tmp[200];
			strcpy(tmp, g_settings.softupdate_proxyusername);
			strcat(tmp, ":");
			strcat(tmp, g_settings.softupdate_proxypassword);
			curl_easy_setopt(curl_handle, CURLOPT_PROXYUSERPWD, tmp);
		}
	}

	char cerror[CURL_ERROR_SIZE];
	curl_easy_setopt(curl_handle, CURLOPT_ERRORBUFFER, cerror);

	dprintf(DEBUG_NORMAL, "cYTFeedParser::DownloadUrl: try to get %s\n", url.c_str());
	
	CURLcode httpres = curl_easy_perform(curl_handle);

	double dsize;
	curl_easy_getinfo(curl_handle, CURLINFO_SIZE_DOWNLOAD, &dsize);
	curl_easy_cleanup(curl_handle);
	fclose(fp);

	dprintf(DEBUG_NORMAL, "cYTFeedParser::DownloadUrl: http: res %d size %f.\n", httpres, dsize);

	if (httpres != 0) 
	{
		dprintf(DEBUG_NORMAL, "cYTFeedParser::DownloadUrl: curl error: %s\n", cerror);
		unlink(file.c_str());
		return false;
	}
	
	return true;
}

void decodeUrl(std::string &url)
{
	CURL * curl_handle = curl_easy_init();
	char * str = curl_easy_unescape(curl_handle, url.c_str(), 0, NULL);
	curl_easy_cleanup(curl_handle);
	
	if(str)
		url = str;
	
	curl_free(str);
}

void encodeUrl(std::string &txt)
{
	CURL * curl_handle = curl_easy_init();
	char * str = curl_easy_escape(curl_handle, txt.c_str(), txt.length());
	curl_easy_cleanup(curl_handle);
	
	if(str)
		txt = str;
	
	curl_free(str);
}

void splitString(std::string &str, std::string delim, std::vector<std::string> &strlist, int start)
{
	strlist.clear();
	int end = 0;
	
	while ((end = str.find(delim, start)) != std::string::npos) 
	{
		strlist.push_back(str.substr(start, end - start));
		start = end + delim.size();
	}
	strlist.push_back(str.substr(start));
}

void splitString(std::string &str, std::string delim, std::map<std::string,std::string> &strmap, int start)
{
	int end = 0;
	if ((end = str.find(delim, start)) != std::string::npos) 
	{
		strmap[str.substr(start, end - start)] = str.substr(end - start + delim.size());
	}
}

// FielHelpers
CFileHelpers::CFileHelpers()
{
	doCopyFlag = true;
}

CFileHelpers::~CFileHelpers()
{
}

CFileHelpers* CFileHelpers::getInstance()
{
	static CFileHelpers* FileHelpers = NULL;
	if(!FileHelpers)
		FileHelpers = new CFileHelpers();
	return FileHelpers;
}

bool CFileHelpers::copyFile(const char *Src, const char *Dst, mode_t mode)
{
	int FileBufSize = 0xFFFF;
	int fd1, fd2;

	doCopyFlag = true;
	unlink(Dst);
	if ((fd1 = open(Src, O_RDONLY)) < 0)
		return false;
	
	if ((fd2 = open(Dst, O_WRONLY | O_CREAT, mode)) < 0) 
	{
		close(fd1);
		return false;
	}

	char *FileBuf		= new char[FileBufSize];
	uint32_t block;
	off64_t fsizeSrc64 = lseek64(fd1, 0, SEEK_END);
	lseek64(fd1, 0, SEEK_SET);
	
	if (fsizeSrc64 > 0x7FFFFFF0) 
	{ // > 2GB
		off64_t fsize64 = fsizeSrc64;
		block = FileBufSize;
		//printf("#####[%s] fsizeSrc64: %lld 0x%010llX - large file\n", __FUNCTION__, fsizeSrc64, fsizeSrc64);
		while(fsize64 > 0) 
		{
			if(fsize64 < (off64_t)FileBufSize)
				block = (uint32_t)fsize64;
			read(fd1, FileBuf, block);
			write(fd2, FileBuf, block);
			fsize64 -= block;
			if (!doCopyFlag)
				break;
		}
		
		if (doCopyFlag) 
		{
			lseek64(fd2, 0, SEEK_SET);
			off64_t fsizeDst64 = lseek64(fd2, 0, SEEK_END);
			if (fsizeSrc64 != fsizeDst64){
				close(fd1);
				close(fd2);
				delete [] FileBuf;
				return false;
			}
		}
	}
	else 
	{ // < 2GB
		off_t fsizeSrc = lseek(fd1, 0, SEEK_END);
		lseek(fd1, 0, SEEK_SET);
		off_t fsize = fsizeSrc;
		block = FileBufSize;
		//printf("#####[%s] fsizeSrc: %ld 0x%08lX - normal file\n", __FUNCTION__, fsizeSrc, fsizeSrc);
		while(fsize > 0) 
		{
			if(fsize < (off_t)FileBufSize)
				block = (uint32_t)fsize;
			read(fd1, FileBuf, block);
			write(fd2, FileBuf, block);
			fsize -= block;
			if (!doCopyFlag)
				break;
		}
		
		if (doCopyFlag) 
		{
			lseek(fd2, 0, SEEK_SET);
			off_t fsizeDst = lseek(fd2, 0, SEEK_END);
			if (fsizeSrc != fsizeDst)
			{
				close(fd1);
				close(fd2);
				delete [] FileBuf;
				return false;
			}
		}
	}
	close(fd1);
	close(fd2);
	delete [] FileBuf;

	if (!doCopyFlag) 
	{
		sync();
		unlink(Dst);
		return false;
	}

	return true;
}

bool CFileHelpers::copyDir(const char *Src, const char *Dst, bool backupMode)
{
	DIR *Directory;
	struct dirent *CurrentFile;
	static struct stat FileInfo;
	std::string srcPath;
	std::string dstPath;
	char buf[PATH_MAX];

	//open directory
	if ((Directory = opendir(Src)) == NULL)
		return false;
	if (lstat(Src, &FileInfo) == -1) 
	{
		closedir(Directory);
		return false;
	}
	// create directory
		// is symlink
	if (S_ISLNK(FileInfo.st_mode)) 
	{
		int len = readlink(Src, buf, sizeof(buf)-1);
		if (len != -1) 
		{
			buf[len] = '\0';
			symlink(buf, Dst);
		}
	}
	else 
	{
		// directory
		if (createDir(Dst, FileInfo.st_mode & 0x0FFF) == false) 
		{
			if (errno != EEXIST) {
				closedir(Directory);
				return false;
			}
		}
	}

	// read directory
	while ((CurrentFile = readdir(Directory)) != NULL) 
	{
		// ignore '.' and '..'
		if (strcmp(CurrentFile->d_name, ".") && strcmp(CurrentFile->d_name, "..")) {
			srcPath = std::string(Src) + "/" + std::string(CurrentFile->d_name);
			if (lstat(srcPath.c_str(), &FileInfo) == -1) {
				closedir(Directory);
				return false;
			}
			dstPath = std::string(Dst) + "/" + std::string(CurrentFile->d_name);
			// is symlink
			if (S_ISLNK(FileInfo.st_mode)) {
				int len = readlink(srcPath.c_str(), buf, sizeof(buf)-1);
				if (len != -1 && len < (int) sizeof(buf)) {
					buf[len] = '\0';
					symlink(buf, dstPath.c_str());
				}
			}
			// is directory
			else if (S_ISDIR(FileInfo.st_mode)) {
				copyDir(srcPath.c_str(), dstPath.c_str());
			}
			// is file
			else if (S_ISREG(FileInfo.st_mode)) {
				std::string save = "";
				(void)backupMode; /* squelch unused parameter warning */
#if ENABLE_EXTUPDATE
				if (backupMode && (CExtUpdate::getInstance()->isBlacklistEntry(srcPath)))
					save = ".save";
#endif
				copyFile(srcPath.c_str(), (dstPath + save).c_str(), FileInfo.st_mode & 0x0FFF);
			}
		}
	}
	closedir(Directory);
	return true;
}

bool CFileHelpers::createDir(const char *Dir, mode_t mode)
{
	char dirPath[strlen(Dir) + 1];
	DIR *dir;
	if ((dir = opendir(Dir)) != NULL) {
		closedir(dir);
		errno = EEXIST;
		return false;
	}

	int ret = -1;
	while (ret == -1) 
	{
		strcpy(dirPath, Dir);
		ret = mkdir(dirPath, mode);
		if ((errno == ENOENT) && (ret == -1)) 
		{
			char * pos = strrchr(dirPath,'/');
			if (pos != NULL) {
				pos[0] = '\0';
				createDir(dirPath, mode);
			}
		}
		else
			return !ret || (errno == EEXIST);
	}
	errno = 0;
	return true;
}

bool CFileHelpers::removeDir(const char *Dir)
{
	DIR *dir;
	struct dirent *entry;
	char path[PATH_MAX];

	dir = opendir(Dir);
	if (dir == NULL) 
	{
		printf("Error opendir()\n");
		return false;
	}
	
	while ((entry = readdir(dir)) != NULL) 
	{
		if (strcmp(entry->d_name, ".") && strcmp(entry->d_name, "..")) 
		{
			snprintf(path, (size_t) PATH_MAX, "%s/%s", Dir, entry->d_name);
			if (entry->d_type == DT_DIR)
				removeDir(path);
			else
				unlink(path);
		}
	}
	closedir(dir);
	rmdir(Dir);

	errno = 0;
	return true;
}

bool CFileHelpers::readDir(const std::string& dirname, CFileList* flist, CFileFilter * fileFilter)
{
	dprintf(DEBUG_NORMAL, "CFileHelpers::readDir %s\n", dirname.c_str());
	
	stat_struct statbuf;
	dirent_struct **namelist;
	int n;

	n = my_scandir(dirname.c_str(), &namelist, 0, my_alphasort);
	if (n < 0)
	{
		perror(("CFileHelpers scandir: " + dirname).c_str());
		return false;
	}
	
	for(int i = 0; i < n;i++)
	{
		CFile file;
		if(strcmp(namelist[i]->d_name, ".") != 0)
		{
			// name
			file.Name = dirname + namelist[i]->d_name;

			// stat
			if(my_stat((file.Name).c_str(),&statbuf) != 0)
				perror("stat error");
			else
			{
				file.Mode = statbuf.st_mode;
				file.Size = statbuf.st_size;
				file.Time = statbuf.st_mtime;

				// skip dirs
				if(S_ISDIR(file.Mode))
				{
					continue;
				}

				// skip not matched filter
				if(fileFilter != NULL )
				{
					if(!fileFilter->matchFilter(file.Name))
					{
						continue;
					}
				}
				//
				
				flist->push_back(file);

				dprintf(DEBUG_NORMAL, "CFileHelpers::add file %s\n", file.Name.c_str());
			}
		}
		free(namelist[i]);
	}

	free(namelist);

	return true;
}




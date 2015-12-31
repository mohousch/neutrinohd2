/*
	Neutrino-GUI  -   DBoxII-Project
	
	$Id: framebuffer.cpp 2013/10/12 mohousch Exp $

	Copyright (C) 2001 Steffen Hehn 'McClean'
                      2003 thegoodguy

	License: GPL

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

#include <driver/framebuffer.h>

#include <stdio.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <memory.h>
#include <math.h>

#include <linux/kd.h>

#include <stdint.h>

#include <gui/color.h>
#include <global.h>
#include <system/debug.h>

// opengl
#ifdef USE_OPENGL
#include <GL/glew.h>
#include "rcinput.h"
#include "glthread.h"
#endif


#define BACKGROUNDIMAGEWIDTH 	DEFAULT_XRES
#define BACKGROUNDIMAGEHEIGHT	DEFAULT_YRES

#define ICON_CACHE_SIZE 1024*1024*2 // 2mb


// png/jpg handling
CFormathandler * fh_root;
void init_handlers(void);
void add_format(int (*picsize)(const char *,int *,int*,int,int),int (*picread)(const char *,unsigned char **,int*,int*), int (*id)(const char*));

//
static uint32_t * virtual_fb = NULL;

CFrameBuffer::CFrameBuffer()
: active ( true )
{
	iconBasePath = "";
	available  = 0;
	cmap.start = 0;
	cmap.len = 256;
	cmap.red = red;
	cmap.green = green;
	cmap.blue  = blue;
	cmap.transp = trans;
	backgroundColor = 0;
	useBackgroundPaint = false;
	background = NULL;
	backupBackground = NULL;
	backgroundFilename = "";
	fd  = 0;
	m_manual_blit = -1;
	
//FIXME: test
	memset(red, 0, 256*sizeof(__u16));
	memset(green, 0, 256*sizeof(__u16));
	memset(blue, 0, 256*sizeof(__u16));
	memset(trans, 0, 256*sizeof(__u16));

	// png/jpg/bmp/crw handlers
	fh_root = NULL;
	init_handlers();

	//
	q_circle = NULL;
	initQCircle();
	corner_tl = false;
	corner_tr = false;
	corner_bl = false;
	corner_br = false;
}

CFrameBuffer* CFrameBuffer::getInstance()
{
	static CFrameBuffer * frameBuffer = NULL;

	if(!frameBuffer) 
	{
		frameBuffer = new CFrameBuffer();
		dprintf(DEBUG_NORMAL, "CFrameBuffer::getInstance: frameBuffer Instance created\n");
	} 

	return frameBuffer;
}

void CFrameBuffer::init(const char * const fbDevice)
{
	int tr = 0xFF;
	
#if defined (USE_OPENGL)
	fd = -1;
	if(!mpGLThreadObj)
	{
		screeninfo.bits_per_pixel = DEFAULT_BPP;
		screeninfo.xres = DEFAULT_XRES;
		screeninfo.xres_virtual = screeninfo.xres;
		screeninfo.yres = DEFAULT_YRES;
		screeninfo.yres_virtual = screeninfo.yres;
		screeninfo.bits_per_pixel = DEFAULT_BPP;
		screeninfo.blue.length = 8;
		screeninfo.blue.offset = 0;
		screeninfo.green.length = 8;
		screeninfo.green.offset = 8;
		screeninfo.red.length = 8;
		screeninfo.red.offset = 16;
		screeninfo.transp.length = 8;
		screeninfo.transp.offset = 24;
		
		mpGLThreadObj = new GLThreadObj(screeninfo.xres, screeninfo.yres);

		if(mpGLThreadObj)
		{ 
			/* kick off the GL thread for the window */
			mpGLThreadObj->Start();
			mpGLThreadObj->waitInit();
		}
	}
	
	lfb = reinterpret_cast<fb_pixel_t*>(mpGLThreadObj->getOSDBuffer());
	memset(lfb, 0x7f, screeninfo.xres * screeninfo.yres * 4);
#else	

	fd = open(fbDevice, O_RDWR);

	if(!fd) 
		fd = open(fbDevice, O_RDWR);

	if (fd < 0) 
	{
		perror(fbDevice);
		goto nolfb;
	}

	// get variable screeninfo
	if (ioctl(fd, FBIOGET_VSCREENINFO, &screeninfo) < 0) 
	{
		perror("FBIOGET_VSCREENINFO");
		goto nolfb;
	}

	memcpy(&oldscreen, &screeninfo, sizeof(screeninfo));

	// get fix screen info
	fb_fix_screeninfo fix;

	if (ioctl(fd, FBIOGET_FSCREENINFO, &fix)<0) 
	{
		perror("FBIOGET_FSCREENINFO");
		goto nolfb;
	}

	available = fix.smem_len;
	
	dprintf(DEBUG_NORMAL, "CFrameBuffer::init %dk video mem\n", available/1024);
	
	lfb = (fb_pixel_t *)mmap(0, available, PROT_WRITE|PROT_READ, MAP_SHARED, fd, 0);

	if (!lfb) 
	{
		perror("mmap");
		goto nolfb;
	}

#ifdef __sh__ 
	//we add 2MB at the end of the buffer, the rest does the blitter 
	lfb += 1920 * 1080;

	if (available / (1024 * 1024) < 12)
	{
		printf("CFrameBuffer::init: to less memory for stmfb given, need at least 12mb\n"); 
		goto nolfb;
	}
#else
	enableManualBlit();
#endif /*sh*/ 
#endif /* USE_OPENGL */

	// icons cache
	cache_size = 0;
	
	// Windows Colors
	paletteSetColor(0x1, 0x010101, tr);
        paletteSetColor(COL_DARK_RED0, 0x800000, tr);
        paletteSetColor(COL_DARK_GREEN0, 0x008000, tr);
	paletteSetColor(0x4, 0x808000, tr);
        paletteSetColor(COL_DARK_BLUE0, 0x000080, tr);
        paletteSetColor(0x6, 0x800080, tr);
        paletteSetColor(0x7, 0x008080, tr);
        paletteSetColor(COL_LIGHT_GRAY0, 0xA0A0A0, tr);
        paletteSetColor(COL_DARK_GRAY0, 0x505050, tr);
        paletteSetColor(COL_RED0, 0xFF0000, tr);
        paletteSetColor(COL_GREEN0, 0x00FF00, tr);
        paletteSetColor(COL_YELLOW0, 0xFFFF00, tr);
        paletteSetColor(COL_BLUE0, 0x0000FF, tr);
        paletteSetColor(COL_PURPLE0, 0xFF00FF, tr);
        paletteSetColor(COL_LIGHT_BLUE0, 0x00FFFF, tr);
        paletteSetColor(COL_WHITE0, 0xFFFFFF, tr);
        paletteSetColor(COL_BLACK0, 0x000000, tr);
        paletteSetColor(COL_BACKGROUND, 0x000000, 0xffff);

        paletteSet(&cmap);

        useBackground(false);

	return;

nolfb:
	dprintf(DEBUG_NORMAL, "CFrameBuffer::init: framebuffer not available.\n");
	lfb = 0;
}


CFrameBuffer::~CFrameBuffer()
{
	std::map<std::string, Icon>::iterator it;

	for(it = icon_cache.begin(); it != icon_cache.end(); it++) 
	{
		free(it->second.data);
	}
	icon_cache.clear();
	
	if (background) 
	{
		delete[] background;
		background = NULL;
	}

	if (backupBackground) 
	{
		delete[] backupBackground;
		backupBackground = NULL;
	}

	if (lfb)
		munmap(lfb, available);
	
	if (virtual_fb)
	{
		delete[] virtual_fb;
		virtual_fb = NULL;
	}
	
#if defined (USE_OPENGL)
	active = false; /* keep people/infoclocks from accessing */
	mpGLThreadObj->shutDown();
	mpGLThreadObj->join();
#else	
	close(fd);
#endif	
}

int CFrameBuffer::getFileHandle() const
{
	return fd;
}

unsigned int CFrameBuffer::getStride() const
{
	return stride;
}

unsigned int CFrameBuffer::getScreenWidth(bool real)
{
	if(real)
		return xRes;
	else
		return g_settings.screen_EndX - g_settings.screen_StartX;
}

unsigned int CFrameBuffer::getScreenHeight(bool real)
{
	if(real)
		return yRes;
	else
		return g_settings.screen_EndY - g_settings.screen_StartY;
}

unsigned int CFrameBuffer::getScreenX()
{
	return g_settings.screen_StartX;
}

unsigned int CFrameBuffer::getScreenY()
{
	return g_settings.screen_StartY;
}

fb_pixel_t * CFrameBuffer::getFrameBufferPointer() const
{	  
	if (active || (virtual_fb == NULL))
	{
		return lfb;		
	}	
	else
		return (fb_pixel_t *) virtual_fb;
}

unsigned int CFrameBuffer::getAvailableMem() const
{
	return available;
}

bool CFrameBuffer::getActive() const
{
	return (active || (virtual_fb != NULL));
}

void CFrameBuffer::setActive(bool enable)
{
	active = enable;
		
#if !defined (__sh__)
	if(enable)
	{
		// set manual blit when fb is activ
		if (m_manual_blit == 0) 
		{
			enableManualBlit();
		}
	}
	else
	{
		if (m_manual_blit == 1) 
		{
			disableManualBlit();
		}
	}
#endif
}

t_fb_var_screeninfo *CFrameBuffer::getScreenInfo()
{
	return &screeninfo;
}

// borrowed from e2 :-P
void CFrameBuffer::setFrameBufferMode(unsigned int nxRes, unsigned int nyRes, unsigned int nbpp)
{
	screeninfo.xres_virtual = screeninfo.xres = nxRes;
	screeninfo.yres_virtual = (screeninfo.yres = nyRes)*2;
	screeninfo.height = 0;
	screeninfo.width = 0;
	screeninfo.xoffset = screeninfo.yoffset = 0;
	screeninfo.bits_per_pixel = nbpp;

	switch (nbpp) 
	{
		case 16:
			// ARGB 1555
			screeninfo.transp.offset = 15;
			screeninfo.transp.length = 1;
			screeninfo.red.offset = 10;
			screeninfo.red.length = 5;
			screeninfo.green.offset = 5;
			screeninfo.green.length = 5;
			screeninfo.blue.offset = 0;
			screeninfo.blue.length = 5;
			break;
		case 32:
			// ARGB 8888
			screeninfo.transp.offset = 24;
			screeninfo.transp.length = 8;
			screeninfo.red.offset = 16;
			screeninfo.red.length = 8;
			screeninfo.green.offset = 8;
			screeninfo.green.length = 8;
			screeninfo.blue.offset = 0;
			screeninfo.blue.length = 8;
			break;
	}
	
	// num of pages
	m_number_of_pages = screeninfo.yres_virtual / nyRes;
	
	if (ioctl(fd, FBIOPUT_VSCREENINFO, &screeninfo) < 0)
	{
		// try single buffering
		screeninfo.yres_virtual = screeninfo.yres = nyRes;

		if (ioctl(fd, FBIOPUT_VSCREENINFO, &screeninfo) < 0)
		{
			perror("FBIOPUT_VSCREENINFO");
		}
		
		printf("CFrameBuffer::setVideoMode: double buffering not available.\n");
	} 
	else
		printf("CFrameBuffer::setVideoMode: double buffering available!\n");
	
	ioctl(fd, FBIOGET_VSCREENINFO, &screeninfo);

	if ((screeninfo.xres != nxRes) && (screeninfo.yres != nyRes) && (screeninfo.bits_per_pixel != nbpp))
	{
		printf("CFrameBuffer::setVideoMode: failed: wanted: %dx%dx%d, got %dx%dx%d\n", nxRes, nyRes, nbpp, screeninfo.xres, screeninfo.yres, screeninfo.bits_per_pixel);
	}
	
	xRes = screeninfo.xres;
	yRes = screeninfo.yres;
	bpp  = screeninfo.bits_per_pixel;
	
	// stride
	fb_fix_screeninfo fix;

	if (ioctl(fd, FBIOGET_FSCREENINFO, &fix)<0)
	{
		perror("FBIOGET_FSCREENINFO");
	}

	stride = fix.line_length;
}

int CFrameBuffer::setMode()
{
	if (!available && !active)
		return -1;
	
	dprintf(DEBUG_NORMAL, "CFrameBuffer::setMode: FB: %dx%dx%d\n", DEFAULT_XRES, DEFAULT_YRES, DEFAULT_BPP);

#if defined (__sh__) || defined (USE_OPENGL)
	xRes = DEFAULT_XRES;
	yRes = DEFAULT_YRES;
	bpp = DEFAULT_BPP;
	stride = xRes * 4;
#else
	setFrameBufferMode(DEFAULT_XRES, DEFAULT_YRES, DEFAULT_BPP);
#endif	

	// clear frameBuffer
	paintBackground();
	
	blit();

	return 0;
}

// blend mode: 0=non-premultiplied alpha | 1=premultiplied alpha
void CFrameBuffer::setBlendMode(uint8_t mode)
{
#ifdef __sh__	
	struct stmfbio_var_screeninfo_ex varEx = {0};
	memset(&varEx, 0, sizeof(varEx));

	varEx.layerid  = 0;
	varEx.activate = STMFBIO_ACTIVATE_IMMEDIATE;
	varEx.caps |= STMFBIO_VAR_CAPS_PREMULTIPLIED;
	varEx.premultiplied_alpha = mode;

	if(ioctl(fd, STMFBIO_SET_VAR_SCREENINFO_EX, &varEx) < 0)
		perror("STMFBIO_SET_VAR_SCREENINFO_EX");
#endif
}

// blendlevel (e.g transparency)
void CFrameBuffer::setBlendLevel(int level)
{
#ifdef __sh__
	struct stmfbio_var_screeninfo_ex varEx = {0};
	memset(&varEx, 0, sizeof(varEx));

	varEx.layerid  = 0;
	varEx.activate = STMFBIO_ACTIVATE_IMMEDIATE;
	varEx.caps = STMFBIO_VAR_CAPS_OPACITY;
	varEx.opacity = level;


	if(ioctl(fd, STMFBIO_SET_VAR_SCREENINFO_EX, &varEx) < 0)
		perror("STMFBIO_SET_VAR_SCREENINFO_EX");
#endif	
}

void CFrameBuffer::paletteFade(int i, __u32 rgb1, __u32 rgb2, int level)
{
	__u16 *r = cmap.red + i;
	__u16 *g = cmap.green + i;
	__u16 *b = cmap.blue + i;
	
	*r= ((rgb2&0xFF0000)>>16)*level;
	*g= ((rgb2&0x00FF00)>>8 )*level;
	*b= ((rgb2&0x0000FF)    )*level;
	*r += ((rgb1&0xFF0000)>>16)*(255 - level);
	*g += ((rgb1&0x00FF00)>>8 )*(255 - level);
	*b += ((rgb1&0x0000FF))*(255 - level);
}

void CFrameBuffer::paletteGenFade(int in, __u32 rgb1, __u32 rgb2, int num, int tr)
{
	for (int i = 0; i < num; i++) 
	{
		paletteFade(in + i, rgb1, rgb2, i*(255/(num - 1)));
		cmap.transp[in + i] = tr;
		tr--; //FIXME
	}
}

void CFrameBuffer::paletteSetColor(int i, __u32 rgb, int tr)
{
	cmap.red[i] = (rgb&0xFF0000)>>8;
	cmap.green[i] = (rgb&0x00FF00)   ;
	cmap.blue[i] = (rgb&0x0000FF)<<8;
	cmap.transp[i] = tr;
}

inline fb_pixel_t make16color(uint16_t r, uint16_t g, uint16_t b, uint16_t t,
				  uint32_t  /*rl*/ = 0, uint32_t  /*ro*/ = 0,
				  uint32_t  /*gl*/ = 0, uint32_t  /*go*/ = 0,
				  uint32_t  /*bl*/ = 0, uint32_t  /*bo*/ = 0,
				  uint32_t  /*tl*/ = 0, uint32_t  /*to*/ = 0)
{
	return ((t << 24) & 0xFF000000) | ((r << 8) & 0xFF0000) | ((g << 0) & 0xFF00) | (b >> 8 & 0xFF);
}

void CFrameBuffer::paletteSet(struct fb_cmap *map)
{
	if (!active)
		return;
	
	if(map == NULL)
		map = &cmap;

	uint32_t rl, ro, gl, go, bl, bo, tl, to;
	
	rl = screeninfo.red.length;
	ro = screeninfo.red.offset;
	gl = screeninfo.green.length;
	go = screeninfo.green.offset;
	bl = screeninfo.blue.length;
	bo = screeninfo.blue.offset;
	tl = screeninfo.transp.length;
	to = screeninfo.transp.offset;

	for (int i = 0; i < 256; i++)
	{
		realcolor[i] = make16color(cmap.red[i], cmap.green[i], cmap.blue[i], cmap.transp[i], rl, ro, gl, go, bl, bo, tl, to);
	}
}

//
int CFrameBuffer::limitRadius(const int& dx, const int& dy, int& radius)
{
	if (radius > dx)
		return dx;
	if (radius > dy)
		return dy;
	if (radius > 540)
		return 540;
	return radius;
}

void CFrameBuffer::setCornerFlags(const int& type)
{
	corner_tl = (type & CORNER_TOP_LEFT)     == CORNER_TOP_LEFT;
	corner_tr = (type & CORNER_TOP_RIGHT)    == CORNER_TOP_RIGHT;
	corner_bl = (type & CORNER_BOTTOM_LEFT)  == CORNER_BOTTOM_LEFT;
	corner_br = (type & CORNER_BOTTOM_RIGHT) == CORNER_BOTTOM_RIGHT;
}

void CFrameBuffer::initQCircle()
{
	/* this table contains the x coordinates for a quarter circle (the bottom right quarter) with fixed
	   radius of 540 px which is the half of the max HD graphics size of 1080 px. So with that table we
	   ca draw boxes with round corners and als circles by just setting dx = dy = radius (max 540). */
	static const int _q_circle[541] = {
		540, 540, 540, 540, 540, 540, 540, 540, 540, 540, 540, 540, 540, 540, 540, 540, 540, 540, 540, 540,
		540, 540, 540, 540, 539, 539, 539, 539, 539, 539, 539, 539, 539, 539, 539, 539, 539, 539, 539, 539,
		539, 538, 538, 538, 538, 538, 538, 538, 538, 538, 538, 538, 538, 537, 537, 537, 537, 537, 537, 537,
		537, 537, 536, 536, 536, 536, 536, 536, 536, 536, 535, 535, 535, 535, 535, 535, 535, 535, 534, 534,
		534, 534, 534, 534, 533, 533, 533, 533, 533, 533, 532, 532, 532, 532, 532, 532, 531, 531, 531, 531,
		531, 531, 530, 530, 530, 530, 529, 529, 529, 529, 529, 529, 528, 528, 528, 528, 527, 527, 527, 527,
		527, 526, 526, 526, 526, 525, 525, 525, 525, 524, 524, 524, 524, 523, 523, 523, 523, 522, 522, 522,
		522, 521, 521, 521, 521, 520, 520, 520, 519, 519, 519, 518, 518, 518, 518, 517, 517, 517, 516, 516,
		516, 515, 515, 515, 515, 514, 514, 514, 513, 513, 513, 512, 512, 512, 511, 511, 511, 510, 510, 510,
		509, 509, 508, 508, 508, 507, 507, 507, 506, 506, 506, 505, 505, 504, 504, 504, 503, 503, 502, 502,
		502, 501, 501, 500, 500, 499, 499, 499, 498, 498, 498, 497, 497, 496, 496, 496, 495, 495, 494, 494,
		493, 493, 492, 492, 491, 491, 490, 490, 490, 489, 489, 488, 488, 487, 487, 486, 486, 485, 485, 484,
		484, 483, 483, 482, 482, 481, 481, 480, 480, 479, 479, 478, 478, 477, 477, 476, 476, 475, 475, 474,
		473, 473, 472, 472, 471, 471, 470, 470, 469, 468, 468, 467, 466, 466, 465, 465, 464, 464, 463, 462,
		462, 461, 460, 460, 459, 459, 458, 458, 457, 456, 455, 455, 454, 454, 453, 452, 452, 451, 450, 450,
		449, 449, 448, 447, 446, 446, 445, 445, 444, 443, 442, 441, 441, 440, 440, 439, 438, 437, 436, 436,
		435, 435, 434, 433, 432, 431, 431, 430, 429, 428, 427, 427, 426, 425, 425, 424, 423, 422, 421, 421,
		420, 419, 418, 417, 416, 416, 415, 414, 413, 412, 412, 411, 410, 409, 408, 407, 406, 405, 404, 403,
		403, 402, 401, 400, 399, 398, 397, 397, 395, 394, 393, 393, 392, 391, 390, 389, 388, 387, 386, 385,
		384, 383, 382, 381, 380, 379, 378, 377, 376, 375, 374, 373, 372, 371, 369, 368, 367, 367, 365, 364,
		363, 362, 361, 360, 358, 357, 356, 355, 354, 353, 352, 351, 350, 348, 347, 346, 345, 343, 342, 341,
		340, 339, 337, 336, 335, 334, 332, 331, 329, 328, 327, 326, 324, 323, 322, 321, 319, 317, 316, 315,
		314, 312, 310, 309, 308, 307, 305, 303, 302, 301, 299, 297, 296, 294, 293, 291, 289, 288, 287, 285,
		283, 281, 280, 278, 277, 275, 273, 271, 270, 268, 267, 265, 263, 261, 259, 258, 256, 254, 252, 250,
		248, 246, 244, 242, 240, 238, 236, 234, 232, 230, 228, 225, 223, 221, 219, 217, 215, 212, 210, 207,
		204, 202, 200, 197, 195, 192, 190, 187, 184, 181, 179, 176, 173, 170, 167, 164, 160, 157, 154, 150,
		147, 144, 140, 136, 132, 128, 124, 120, 115, 111, 105, 101,  95,  89,  83,  77,  69,  61,  52,  40,
		 23};
	if (q_circle == NULL)
		q_circle = new int[sizeof(_q_circle) / sizeof(int)];
	memcpy(q_circle, _q_circle, sizeof(_q_circle));
}

bool CFrameBuffer::calcCorners(int *ofs, int *ofl, int *ofr, const int& dy, const int& line, const int& radius, const int& type)
{
// just an multiplicator for all math to reduce rounding errors
#define MUL 32768
	int scl, _ofs = 0;
	bool ret = false;
	if (ofl != NULL) *ofl = 0;
	if (ofr != NULL) *ofr = 0;
	int scf = (540 * MUL) / ((radius < 1) ? 1 : radius);

	// one of the top corners
	if (line < radius && (type & CORNER_TOP)) 
	{
		// uper round corners
		scl = scf * (radius - line) / MUL;
		if ((scf * (radius - line) % MUL) >= (MUL / 2)) // round up
			scl++;
		_ofs =  radius - (q_circle[scl] * MUL / scf);
		if (ofl != NULL) *ofl = corner_tl ? _ofs : 0;
		if (ofr != NULL) *ofr = corner_tr ? _ofs : 0;
	}

	// one of the bottom corners 
	else if ((line >= dy - radius) && (type & CORNER_BOTTOM)) 
	{
		// lower round corners
		scl = scf * (radius - (dy - (line + 1))) / MUL;
		if ((scf * (radius - (dy - (line + 1))) % MUL) >= (MUL / 2)) // round up
			scl++;
		_ofs =  radius - (q_circle[scl] * MUL / scf);
		if (ofl != NULL) *ofl = corner_bl ? _ofs : 0;
		if (ofr != NULL) *ofr = corner_br ? _ofs : 0;
	}
	else
		ret = true;

	if (ofs != NULL) 
		*ofs = _ofs;
	return ret;
}

void CFrameBuffer::paintHLineRelInternal2Buf(const int& x, const int& dx, const int& y, const int& box_dx, const fb_pixel_t& col, fb_pixel_t* buf)
{
	uint8_t * pos = ((uint8_t *)buf) + x * sizeof(fb_pixel_t) + box_dx * sizeof(fb_pixel_t) * y;
	fb_pixel_t * dest = (fb_pixel_t *)pos;
	for (int i = 0; i < dx; i++)
		*(dest++) = col;
}

fb_pixel_t* CFrameBuffer::paintBoxRel2Buf(const int dx, const int dy, const fb_pixel_t col, fb_pixel_t* buf, int radius, int type)
{
	if (!getActive())
		return buf;

	if (dx == 0 || dy == 0) 
	{
		//dprintf(DEBUG_INFO, "[CFrameBuffer] [%s - %d]: radius %d, dx %d dy %d\n", __func__, __LINE__, radius, dx, dy);
		return buf;
	}

	fb_pixel_t* pixBuf = buf;
	if (pixBuf == NULL) 
	{
		pixBuf = (fb_pixel_t*)malloc(dx*dy*sizeof(fb_pixel_t));
		if (pixBuf == NULL) 
		{
			//dprintf(DEBUG_NORMAL, "[%s #%d] Error malloc\n", __func__, __LINE__);
			return NULL;
		}
	}
	memset((void*)pixBuf, '\0', dx*dy*sizeof(fb_pixel_t));

	if (type && radius) 
	{
		setCornerFlags(type);
		radius = limitRadius(dx, dy, radius);

		int line = 0;
		while (line < dy) 
		{
			int ofl, ofr;
			calcCorners(NULL, &ofl, &ofr, dy, line, radius, type);

			if (dx - ofr - ofl < 1) 
			{
				if (dx-ofr-ofl == 0) 
				{
					dprintf(DEBUG_INFO, "[%s - %d]: radius %d, end x %d y %d\n", __func__, __LINE__, radius, dx-ofr-ofl, line);
				}
				else {
					dprintf(DEBUG_INFO, "[%s - %04d]: Calculated width: %d\n		      (radius %d, dx %d, offsetLeft %d, offsetRight %d).\n		      Width can not be less than 0, abort.\n",
					       __func__, __LINE__, dx-ofr-ofl, radius, dx, ofl, ofr);
				}
				line++;
				continue;
			}
			paintHLineRelInternal2Buf(ofl, dx - ofl - ofr, line, dx, col, pixBuf);
			line++;
		}
	} 
	else 
	{
		fb_pixel_t *bp = pixBuf;
		int line = 0;

		while (line < dy) 
		{
			for (int pos = 0; pos < dx; pos++)
				*(bp + pos) = col;
			bp += dx;
			line++;
		}
	}

	return pixBuf;
}

//

void CFrameBuffer::paintBoxRel(const int x, const int y, const int dx, const int dy, fb_pixel_t col, int radius, int type, bool fadeColor, int mode)
{
	if (!getActive())
		return;

#define MASK 0xFFFFFFFF

	// boxBuf
	fb_pixel_t* boxBuf = paintBoxRel2Buf(dx, dy, fadeColor? MASK : col, NULL, radius, type);
        if (!boxBuf)
               return;

	// gradientBuf
	if(fadeColor)
	{
		fb_pixel_t* gradientBuf = gradientOneColor(col, NULL, dy, mode);

		fb_pixel_t *bp = boxBuf;
		fb_pixel_t *gra = gradientBuf;

		// vertical
		for (int pos = 0; pos < dx; pos++) 
		{
			for(int count = 0; count < dy; count++) 
			{
				if (*(bp + pos) == MASK)
					*(bp + pos) = (fb_pixel_t)(*(gra + count));
				bp += dx;
			}
			bp = boxBuf;
		}
	}

	blit2FB(boxBuf, dx, dy, x, y);
}

void CFrameBuffer::paintVLine(int x, int ya, int yb, const fb_pixel_t col)
{
	if (!getActive())
		return;

	uint8_t * pos = ((uint8_t *)getFrameBufferPointer()) + x * sizeof(fb_pixel_t) + stride * ya;

	int dy = yb - ya;
	
	for (int count = 0; count < dy; count++) 
	{
		*(fb_pixel_t *)pos = col;
		pos += stride;
	}	
}

void CFrameBuffer::paintVLineRel(int x, int y, int dy, const fb_pixel_t col)
{
	if (!getActive())
		return;

	uint8_t * pos = ((uint8_t *)getFrameBufferPointer()) + x * sizeof(fb_pixel_t) + stride * y;

	for(int count = 0; count < dy; count++) 
	{
		*(fb_pixel_t *)pos = col;
		pos += stride;
	}	
}

void CFrameBuffer::paintHLine(int xa, int xb, int y, const fb_pixel_t col)
{
	if (!getActive())
		return;

	uint8_t * pos = ((uint8_t *)getFrameBufferPointer()) + xa * sizeof(fb_pixel_t) + stride * y;

	int dx = xb - xa;
	fb_pixel_t * dest = (fb_pixel_t *)pos;
	for (int i = 0; i < dx; i++)
		*(dest++) = col;	
}

void CFrameBuffer::paintHLineRel(int x, int dx, int y, const fb_pixel_t col)
{
	if (!getActive())
		return;

	uint8_t * pos = ((uint8_t *)getFrameBufferPointer()) + x * sizeof(fb_pixel_t) + stride * y;

	fb_pixel_t * dest = (fb_pixel_t *)pos;
	for (int i = 0; i < dx; i++)
		*(dest++) = col;	
}

void CFrameBuffer::setIconBasePath(const std::string & iconPath)
{
	iconBasePath = iconPath;
}

// get icon size
void CFrameBuffer::getIconSize(const char * const filename, int * width, int * height)
{

	if(filename == NULL)
		return;
	
	int icon_fd;
	std::string iconfile = filename;
	
	if( !strstr(iconfile.c_str(), ".png"))
		iconfile = iconBasePath + filename + ".png";

	icon_fd = open(iconfile.c_str(), O_RDONLY);

	if (icon_fd == -1)
	{
		// check for fullname icon
		std::string iconfile1 = iconBasePath + filename + ".raw";
			
		icon_fd = open(iconfile1.c_str(), O_RDONLY);
		
		if (icon_fd == -1)
		{
			return;
		}
		else
		{
			// raw
			struct rawHeader header;
			uint16_t x;
			uint16_t y;
				
			read(icon_fd, &header, sizeof(struct rawHeader));
			
			x = (header.width_hi << 8) | header.width_lo;
			y = (header.height_hi << 8) | header.height_lo;
				
			*width = x;
			*height = y;
		}
		close(icon_fd);
	}
	else
	{
		CFormathandler * fh;
		int x, y;
		
		fh = fh_getsize(iconfile.c_str(), &x, &y, INT_MAX, INT_MAX);
		
		if (fh == NULL) 
		{
			*width = 0;
			*height = 0;
		}
		else
		{
			*width = x;
			*height = y;
		}
	}

	close(icon_fd);
}

bool CFrameBuffer::paintIcon8(const std::string & filename, const int x, const int y, const unsigned char offset)
{
	if (!getActive())
		return false;

	struct rawHeader header;
	uint16_t width, height;
	int _fd;

	_fd = open((iconBasePath + filename).c_str(), O_RDONLY);

	if (_fd == -1) 
	{
		printf("CFrameBuffer::paintIcon8: error while loading icon: %s%s\n", iconBasePath.c_str(), filename.c_str());
		return false;
	}

	read(_fd, &header, sizeof(struct rawHeader));

	width  = (header.width_hi  << 8) | header.width_lo;
	height = (header.height_hi << 8) | header.height_lo;

	unsigned char pixbuf[768];

	uint8_t * d = ((uint8_t *)getFrameBufferPointer()) + x * sizeof(fb_pixel_t) + stride * y;
	
	fb_pixel_t * d2;

	for (int count = 0; count < height; count ++ ) 
	{
		read(_fd, &pixbuf[0], width );
		unsigned char *pixpos = &pixbuf[0];
		d2 = (fb_pixel_t *) d;
		for (int count2 = 0; count2 < width; count2 ++ ) 
		{
			unsigned char color = *pixpos;
			
			if (color != header.transp) 
			{
				paintPixel(d2, color + offset);
			}
			d2++;
			pixpos++;
		}
		d += stride;
	}
	close(_fd);

	return true;
}

// paint icon raw
bool CFrameBuffer::paintIconRaw(const std::string & filename, const int x, const int y, const int h, const unsigned char offset, bool paint)
{
	if (!getActive())
		return false;
	
	struct rawHeader header;
	
	int width, height;
	int lfd;
	
	fb_pixel_t * data;
	struct Icon tmpIcon;
	std::map<std::string, Icon>::iterator it;
	int dsize;
	int  yy = y;

	/* we cache and check original name */
	it = icon_cache.find(filename);
	if(it == icon_cache.end()) 
	{
		std::string newname = iconBasePath + filename.c_str() + ".raw";

		lfd = open(newname.c_str(), O_RDONLY);

		if (lfd == -1) 
		{
			dprintf(DEBUG_NORMAL, "paintIcon: error while loading icon: %s\n", newname.c_str());
			return false;
		}
		
		read(lfd, &header, sizeof(struct rawHeader));

		tmpIcon.width = width  = (header.width_hi  << 8) | header.width_lo;
		tmpIcon.height = height = (header.height_hi << 8) | header.height_lo;
		
		dsize = width*height*sizeof(fb_pixel_t);
		tmpIcon.data = (fb_pixel_t*)malloc(dsize);
		data = tmpIcon.data;
		
		unsigned char pixbuf[768];
		for (int count = 0; count < height; count ++ ) 
		{
			read(lfd, &pixbuf[0], width >> 1 );
			unsigned char *pixpos = &pixbuf[0];
			for (int count2 = 0; count2 < width >> 1; count2 ++ ) 
			{
				unsigned char compressed = *pixpos;
				unsigned char pix1 = (compressed & 0xf0) >> 4;
				unsigned char pix2 = (compressed & 0x0f);
				if (pix1 != header.transp)
					*data++ = realcolor[pix1+offset];
				else
					*data++ = 0;
				if (pix2 != header.transp)
					*data++ = realcolor[pix2+offset];
				else
					*data++ = 0;
				pixpos++;
			}
		}
		
		close(lfd);
		
		/* cache it */
		data = tmpIcon.data;

		if(cache_size+dsize < ICON_CACHE_SIZE) 
		{
			cache_size += dsize;
			icon_cache.insert(std::pair <std::string, Icon> (filename, tmpIcon));
		}
	} 
	else 
	{
		data = it->second.data;
		width = it->second.width;
		height = it->second.height;
	}
	
	if(!paint)
		return true;

	if (h != 0)
		yy += (h - height) / 2;	

	blit2FB(data, width, height, x, yy);

	return true;
}
//

/* 
* paint icon at position x/y,
* if height h is given, center vertically between y and y+h
* offset is a color offset (probably only useful with palette) 
*/
bool CFrameBuffer::paintIcon(const std::string & filename, const int x, const int y, const int h, bool paint, int width, int height)
{
	if (!getActive())
		return false;
	
	fb_pixel_t * data;
	struct Icon tmpIcon;
	std::map<std::string, Icon>::iterator it;
	int dsize;
	int  yy = y;

	/* we cache and check original name */
	it = icon_cache.find(filename);
	if(it == icon_cache.end()) 
	{
		std::string newname = iconBasePath + filename.c_str() + ".png";
		
		if(width == 0 || height == 0)	
			getIconSize(newname.c_str(), &width, &height);
		data = getImage(newname, width, height);
		
		if(!data) 
		{
			dprintf(DEBUG_DEBUG, "paintIcon: error while loading icon: %s\n", newname.c_str());
			
			if(width == 0 || height == 0)	
				getIconSize(filename.c_str(), &width, &height);
			data = getImage(filename, width, height);
		}

		if(data) 
		{
			// cache it
			dsize = width*height*sizeof(fb_pixel_t);
			
			if(cache_size + dsize < ICON_CACHE_SIZE) 
			{
				cache_size += dsize;
				tmpIcon.width = width;
				tmpIcon.height = height;
				tmpIcon.data = data;
				icon_cache.insert(std::pair <std::string, Icon> (filename, tmpIcon));
			}
			
			// display icon
			goto _display;
		}
		else
		{
			dprintf(DEBUG_DEBUG, "paintIcon: error while loading icon: %s\n", filename.c_str());
			return false;
		}
	} 
	else 
	{
		data = it->second.data;
		width = it->second.width;
		height = it->second.height;
	}
	
_display:
	if(!paint)
		return true;

	if (h != 0)
		yy += (h - height) / 2;	

	blit2FB(data, width, height, x, yy, 0, 0, true);

	return true;
}

void CFrameBuffer::loadPal(const std::string & filename, const unsigned char offset, const unsigned char endidx)
{
	if (!getActive())
		return;

	struct rgbData rgbdata;
	int _fd;

	_fd = open((iconBasePath + filename).c_str(), O_RDONLY);

	if (_fd == -1) 
	{
		printf("CFrameBuffer::loadPal: error while loading palette: %s%s\n", iconBasePath.c_str(), filename.c_str());
		return;
	}

	int pos = 0;
	int readb = read(_fd, &rgbdata,  sizeof(rgbdata) );
	
	while(readb) 
	{
		__u32 rgb = (rgbdata.r<<16) | (rgbdata.g<<8) | (rgbdata.b);
		int colpos = offset + pos;
		if( colpos > endidx)
			break;

		paletteSetColor(colpos, rgb, 0xFF);
		readb = read(_fd, &rgbdata,  sizeof(rgbdata) );
		pos++;
	}
	
	paletteSet(&cmap);
	close(_fd);
}

void CFrameBuffer::paintPixel(const int x, const int y, const fb_pixel_t col)
{
	if (!getActive())
		return;

	fb_pixel_t * pos = getFrameBufferPointer();

	pos += (stride / sizeof(fb_pixel_t)) * y;
	pos += x;

	*pos = col;
}

void CFrameBuffer::paintLine(int xa, int ya, int xb, int yb, const fb_pixel_t col)
{
	if (!getActive())
		return;
	
	int dx = abs (xa - xb);
	int dy = abs (ya - yb);
	int x;
	int y;
	int End;
	int step;

	if ( dx > dy )
	{
		int	p = 2 * dy - dx;
		int	twoDy = 2 * dy;
		int	twoDyDx = 2 * (dy-dx);

		if ( xa > xb )
		{
			x = xb;
			y = yb;
			End = xa;
			step = ya < yb ? -1 : 1;
		}
		else
		{
			x = xa;
			y = ya;
			End = xb;
			step = yb < ya ? -1 : 1;
		}

		paintPixel(x, y, col);

		while( x < End )
		{
			x++;
			if ( p < 0 )
				p += twoDy;
			else
			{
				y += step;
				p += twoDyDx;
			}
			paintPixel(x, y, col);
		}
	}
	else
	{
		int	p = 2 * dx - dy;
		int	twoDx = 2 * dx;
		int	twoDxDy = 2 * (dx-dy);

		if ( ya > yb )
		{
			x = xb;
			y = yb;
			End = ya;
			step = xa < xb ? -1 : 1;
		}
		else
		{
			x = xa;
			y = ya;
			End = yb;
			step = xb < xa ? -1 : 1;
		}

		paintPixel(x, y, col);

		while( y < End )
		{
			y++;
			if ( p < 0 )
				p += twoDx;
			else
			{
				x += step;
				p += twoDxDy;
			}
			paintPixel(x, y, col);
		}
	}
}

void CFrameBuffer::setBackgroundColor(const fb_pixel_t color)
{
	backgroundColor = color;
}

bool CFrameBuffer::loadBackgroundPic(const std::string & filename, bool show)
{
	if ((backgroundFilename == filename) && (background))
		return true;
	
	dprintf(DEBUG_INFO, "CFrameBuffer::loadBackgroundPic: %s\n", filename.c_str());	

	if (background)
		delete[] background;
	
	// get bg image
	background = getImage(iconBasePath + filename, BACKGROUNDIMAGEWIDTH, BACKGROUNDIMAGEHEIGHT);
	
	if(!background) 
		background = getImage(filename, BACKGROUNDIMAGEWIDTH, BACKGROUNDIMAGEHEIGHT);

	// if not found
	if (background == NULL) 
	{
		background = 0;
		return false;
	}

	backgroundFilename = filename;
	
	if(show) 
	{
		useBackgroundPaint = true;
		paintBackground();
	}
	
	return true;
}

void CFrameBuffer::useBackground(bool ub)
{
	useBackgroundPaint = ub;
	
	if(!useBackgroundPaint) 
	{
		delete[] background;
		background = 0;
	}
}

bool CFrameBuffer::getuseBackground(void)
{
	return useBackgroundPaint;
}

void CFrameBuffer::saveBackgroundImage(void)
{
	if (backupBackground != NULL)
		delete[] backupBackground;

	backupBackground = background;
	useBackgroundPaint = false;
	background = NULL;
}

void CFrameBuffer::restoreBackgroundImage(void)
{
	fb_pixel_t * tmp = background;

	if (backupBackground != NULL)
	{
		background = backupBackground;
		backupBackground = NULL;
	}
	else
		useBackground(false); 		// <- necessary since no background is available

	if (tmp != NULL)
		delete[] tmp;
}

void CFrameBuffer::paintBackgroundBoxRel(int x, int y, int dx, int dy)
{
	if (!getActive())
		return;

	if(!useBackgroundPaint)
	{		
		paintBoxRel(x, y, dx, dy, backgroundColor);
	}
	else
	{
		uint8_t * fbpos = ((uint8_t *)getFrameBufferPointer()) + x * sizeof(fb_pixel_t) + stride * y;

		fb_pixel_t * bkpos = background + x + BACKGROUNDIMAGEWIDTH * y;

		for(int count = 0;count < dy; count++)
		{
			memcpy(fbpos, bkpos, dx * sizeof(fb_pixel_t));
			fbpos += stride;
			bkpos += BACKGROUNDIMAGEWIDTH;
		}
	}
}

void CFrameBuffer::paintBackground()
{
	if (!getActive())
		return;

	if (useBackgroundPaint && (background != NULL))
	{
		for (int i = 0; i < BACKGROUNDIMAGEHEIGHT; i++)
			memcpy(((uint8_t *)getFrameBufferPointer()) + i * stride, (background + i * BACKGROUNDIMAGEWIDTH), BACKGROUNDIMAGEWIDTH * sizeof(fb_pixel_t));
	}
	else
	{
		paintBoxRel(0, 0, xRes, yRes, backgroundColor);
	}	
}

void CFrameBuffer::SaveScreen(int x, int y, int dx, int dy, fb_pixel_t * const memp)
{
	dprintf(DEBUG_DEBUG, "CFrameBuffer::SaveScreen\n");
	
	if (!getActive())
		return;

	uint8_t * pos = ((uint8_t *)getFrameBufferPointer()) + x * sizeof(fb_pixel_t) + stride * y;

	fb_pixel_t * bkpos = memp;

	for (int count = 0; count < dy; count++) 
	{
		fb_pixel_t * dest = (fb_pixel_t *)pos;
		for (int i = 0; i < dx; i++)
			*(bkpos++) = *(dest++);
		
		pos += stride;
	}
}

void CFrameBuffer::RestoreScreen(int x, int y, int dx, int dy, fb_pixel_t * const memp)
{
	dprintf(DEBUG_DEBUG, "CFrameBuffer::RestoreScreen\n");
	
	if (!getActive())
		return;

	uint8_t * fbpos = ((uint8_t *)getFrameBufferPointer()) + x * sizeof(fb_pixel_t) + stride * y;
	
	fb_pixel_t * bkpos = memp;

	for (int count = 0; count < dy; count++)
	{
		memcpy(fbpos, bkpos, dx * sizeof(fb_pixel_t));
		fbpos += stride;
		bkpos += dx;
	}
}

void CFrameBuffer::ClearFrameBuffer()
{
	paintBackground();
}

void * CFrameBuffer::convertRGB2FB(unsigned char * rgbbuff, unsigned long x, unsigned long y, int transp, int m_transparent, bool alpha)
{
	unsigned long i;
	unsigned int * fbbuff;
	unsigned long count = x*y;

	fbbuff = (unsigned int *) malloc(count * sizeof(unsigned int));
	
	if(fbbuff == NULL)
	{
		dprintf(DEBUG_INFO, "CFrameBuffer::convertRGB2FB: Error: malloc\n");
		return NULL;
	}
	
	if(alpha)
	{
		for(i = 0; i < count ; i++)
			fbbuff[i] = ((rgbbuff[i*4+3] << 24) & 0xFF000000) | 
			            ((rgbbuff[i*4]   << 16) & 0x00FF0000) | 
		        	    ((rgbbuff[i*4+1] <<  8) & 0x0000FF00) | 
			            ((rgbbuff[i*4+2])       & 0x000000FF);
	}
	else
	{
		switch (m_transparent) 
		{
			case CFrameBuffer::TM_BLACK:
				for(i = 0; i < count ; i++) 
				{
					transp = 0;
					if(rgbbuff[i*3] || rgbbuff[i*3+1] || rgbbuff[i*3+2])
						transp = 0xFF;
					fbbuff[i] = (transp << 24) | ((rgbbuff[i*3] << 16) & 0xFF0000) | ((rgbbuff[i*3+1] << 8) & 0xFF00) | (rgbbuff[i*3+2] & 0xFF);
				}
				break;
							
			case CFrameBuffer::TM_INI:
				for(i = 0; i < count ; i++)
					fbbuff[i] = (transp << 24) | ((rgbbuff[i*3] << 16) & 0xFF0000) | ((rgbbuff[i*3+1] << 8) & 0xFF00) | (rgbbuff[i*3+2] & 0xFF);
				break;
							
			case CFrameBuffer::TM_NONE:
			default:
				for(i = 0; i < count ; i++)
					fbbuff[i] = 0xFF000000 | ((rgbbuff[i*3] << 16) & 0xFF0000) | ((rgbbuff[i*3+1] << 8) & 0xFF00) | (rgbbuff[i*3+2] & 0xFF);
				break;
		}
	}

	return (void *) fbbuff;
}

// blit2FB
void CFrameBuffer::blit2FB(void * fbbuff, uint32_t width, uint32_t height, uint32_t xoff, uint32_t yoff, uint32_t xp, uint32_t yp, bool transp )
{ 
	int xc = (width > xRes) ? xRes : width;
	int yc = (height > yRes) ? yRes : height;

	fb_pixel_t * data = (fb_pixel_t *) fbbuff;

	uint8_t * d = ((uint8_t *)getFrameBufferPointer()) + xoff * sizeof(fb_pixel_t) + stride * yoff;
	fb_pixel_t * d2;

	for (int count = 0; count < yc; count++ ) 
	{
		fb_pixel_t * pixpos = &data[(count + yp) * width];
		d2 = (fb_pixel_t *) d;
		for (int count2 = 0; count2 < xc; count2++ ) 
		{
			fb_pixel_t pix = *(pixpos + xp);
			
			if (!transp || (pix & 0xff000000) == 0xff000000)
				*d2 = pix;
			else //alpha blending
			{
				uint8_t *in = (uint8_t *)(pixpos + xp);
				uint8_t *out = (uint8_t *)d2;
				int a = in[3];	//TODO: big/little endian
				*out = (*out + ((*in - *out) * a) / 256);
				in++; out++;
				*out = (*out + ((*in - *out) * a) / 256);
				in++; out++;
				*out = (*out + ((*in - *out) * a) / 256);
			}
			
			d2++;
			pixpos++;
		}
		d += stride;
	}
}

// display RGB (used by pictureviewer)
void CFrameBuffer::displayRGB(unsigned char * rgbbuff, int x_size, int y_size, int x_pan, int y_pan, int x_offs, int y_offs, bool clearfb, int transp)
{
        void * fbbuff = NULL;

        if(rgbbuff == NULL)
                return;

        // correct panning
        if(x_pan > x_size - (int)xRes) 
		x_pan = 0;
        if(y_pan > y_size - (int)yRes) 
		y_pan = 0;

        // correct offset
        if(x_offs + x_size > (int)xRes) 
		x_offs = 0;
        if(y_offs + y_size > (int)yRes) 
		y_offs = 0;

        // convert
        fbbuff = convertRGB2FB(rgbbuff, x_size, y_size, transp);
        if(fbbuff == NULL)
                return;

        // ClearFB if image is smaller
        if(clearfb)
                ClearFrameBuffer();

	// blit2fb
        blit2FB(fbbuff, x_size, y_size, x_offs, y_offs, x_pan, y_pan);
	
        free(fbbuff);
}

// resize.cpp
unsigned char * CFrameBuffer::Resize(unsigned char * origin, int ox, int oy, int dx, int dy, ScalingMode type, unsigned char * dst, bool alpha)
{
	unsigned char * cr;
	
	if(dst == NULL) 
	{
		cr = (unsigned char*) malloc(dx*dy*(alpha? 4 : 3));

		if(cr == NULL)
		{
			dprintf(DEBUG_INFO, "Error: malloc\n");
			return(origin);
		}
	} 
	else
		cr = dst;

	if(type == SIMPLE) 
	{
		unsigned char *p, *l;
		int i, j, k, ip;
		l = cr;

		for(j = 0; j < dy; j++, l += dx*3)
		{
			p = origin + (j*oy/dy*ox*3);
			for(i = 0, k = 0; i < dx; i++, k += 3)
			{
				ip = i*ox/dx*3;
				memmove(l+k, p+ip, 3);
			}
		}
	} 
	else 
	{
		unsigned char *p, *q;
		int i, j, k, l, ya, yb;
		int sq, r, g, b, a;

		p = cr;

		int xa_v[dx];
		for(i = 0; i < dx; i++)
			xa_v[i] = i*ox/dx;
		int xb_v[dx+1];
		
		for(i = 0; i < dx; i++)
		{
			xb_v[i] = (i+1)*ox/dx;
			if(xb_v[i] >= ox)
				xb_v[i] = ox - 1;
		}
		
		if (alpha)
		{
			for(j = 0;j < dy;j++)
			{
				ya = j*oy/dy;
				yb = (j + 1)*oy/dy; 
				if(yb >= oy) 
					yb = oy - 1;
				
				for(i = 0; i < dx; i++, p += 4)
				{
					for(l = ya, r = 0, g = 0, b = 0, a = 0, sq = 0; l <= yb; l++)
					{
						q = origin + ((l*ox + xa_v[i])*4);
						for(k = xa_v[i]; k <= xb_v[i]; k++, q += 4, sq++)
						{
							r += q[0]; g += q[1]; b += q[2]; a += q[3];
						}
					}
					p[0] = r/sq; 
					p[1] = g/sq; 
					p[2] = b/sq; 
					p[3] = a/sq;
				}
			}
		}
		else
		{
			for(j = 0; j < dy; j++)
			{
				ya = j*oy/dy;
				yb = (j + 1)*oy/dy; 
				if(yb >= oy) 
					yb = oy - 1;
					
				for(i = 0; i < dx; i++, p += 3)
				{
					for(l = ya, r = 0, g = 0, b = 0, sq = 0; l <= yb; l++)
					{
						q = origin + ((l*ox+xa_v[i])*3);
							
						for(k = xa_v[i]; k <= xb_v[i]; k++, q += 3, sq++)
						{
							r += q[0]; g += q[1]; b += q[2];
						}
					}
					p[0] = r/sq; p[1] = g/sq; p[2] = b/sq;
				}
			}
		}
	}
	
	free(origin);
	
	return(cr);
}

// PNG
extern int fh_png_getsize(const char *name, int *x, int *y, int wanted_width, int wanted_height);
extern int fh_png_load(const char *name, unsigned char **buffer, int* xp, int* yp);
extern int fh_png_id(const char *name);
extern int png_load_ext(const char * name, unsigned char ** buffer, int * xp, int * yp, int * bpp);

// JPG
extern int fh_jpeg_getsize (const char *, int *, int *, int, int);
extern int fh_jpeg_load (const char *, unsigned char **, int *, int *);
extern int fh_jpeg_id (const char *);

// BMP
extern int fh_bmp_getsize (const char *, int *, int *, int, int);
extern int fh_bmp_load (const char *, unsigned char **, int *, int *);
extern int fh_bmp_id (const char *);

void add_format (int (*picsize) (const char *, int *, int *, int, int), int (*picread) (const char *, unsigned char **, int *, int *), int (*id) (const char *))
{
	CFormathandler * fhn;
	fhn = (CFormathandler *) malloc(sizeof (CFormathandler));
	fhn->get_size = picsize;
	fhn->get_pic = picread;
	fhn->id_pic = id;
	fhn->next = fh_root;
	fh_root = fhn;
}

void init_handlers (void)
{
	// add png format
  	add_format (fh_png_getsize, fh_png_load, fh_png_id);
	
	// add jpg format
	add_format (fh_jpeg_getsize, fh_jpeg_load, fh_jpeg_id);
	
	// add bmp
	add_format (fh_bmp_getsize, fh_bmp_load, fh_bmp_id);
}

CFormathandler * fh_getsize(const char *name, int *x, int *y, int width_wanted, int height_wanted)
{
	CFormathandler * fh;
	
	for (fh = fh_root; fh != NULL; fh = fh->next) 
	{
		if (fh->id_pic (name))
			if (fh->get_size (name, x, y, width_wanted, height_wanted) == FH_ERROR_OK)
				return (fh);
	}

	return (NULL);
}

fb_pixel_t * CFrameBuffer::getImage(const std::string &name, int width, int height)
{
	int x, y;
	CFormathandler * fh;
	unsigned char * buffer;
	fb_pixel_t * ret = NULL;
	int load_ret;
	int _bpp = 0;

  	fh = fh_getsize(name.c_str(), &x, &y, INT_MAX, INT_MAX);
	
  	if (fh) 
	{
		buffer = (unsigned char *) malloc(x*y*4);
		
		if (buffer == NULL) 
		{
		  	dprintf(DEBUG_DEBUG, "CFrameBuffer::getImage: Error: malloc\n");
		  	return false;
		}
		
		if ((name.find(".png") == (name.length() - 4)) && (fh_png_id(name.c_str())))
			load_ret = png_load_ext(name.c_str(), &buffer, &x, &y, &_bpp);
		else
			load_ret = fh->get_pic(name.c_str(), &buffer, &x, &y);

		if (load_ret == FH_ERROR_OK) 
		{
			// resize
			if(x != width || y != height)
			{
				// alpha
				if(_bpp == 4)
					buffer = Resize(buffer, x, y, width, height, COLOR, NULL, true);
				else
					buffer = Resize(buffer, x, y, width, height, COLOR);
				
				x = width ;
				y = height;
			} 
			
			// convert RGB2FB
			if( name.find(".png") == (name.length() - 4) )
			{
				// alpha
				if (_bpp == 4)
					ret = (fb_pixel_t *) convertRGB2FB(buffer, x, y, 0, TM_INI, true);
				else
					ret = (fb_pixel_t *)convertRGB2FB(buffer, x, y, convertSetupAlpha2Alpha(g_settings.infobar_alpha));
			}
			else
				ret = (fb_pixel_t *)convertRGB2FB(buffer, x, y, convertSetupAlpha2Alpha(g_settings.infobar_alpha), TM_NONE);
			
			free(buffer);
		} 
		else 
		{
	  		dprintf(DEBUG_DEBUG, "CFrameBuffer::getImage: Error decoding file %s\n", name.c_str ());
	  		free (buffer);
	  		buffer = NULL;
		}
  	} 
	else
	{
		dprintf(DEBUG_DEBUG, "CFrameBuffer::getImage: Error open file %s\n", name.c_str ());
	}

	return ret;
}

// display image
bool CFrameBuffer::DisplayImage(const std::string & name, int posx, int posy, int width, int height)
{
	dprintf(DEBUG_INFO, "CFrameBuffer::DisplayImage %s\n", name.c_str());
	
	if(!getActive())
		return false;
	
	bool isPNG = false;
	
	if( name.find(".png") == (name.length() - 4) )
		isPNG = true;
	
	fb_pixel_t * data = getImage(name, width, height);

	if(data) 
	{
		blit2FB(data, width, height, posx, posy, 0, 0, isPNG? true : false);
		free(data);
		return true;
	}
	
	return false;
}

//
#ifndef FBIO_WAITFORVSYNC
#define FBIO_WAITFORVSYNC _IOW('F', 0x20, __u32)
#endif

#ifndef FBIO_BLIT
#define FBIO_SET_MANUAL_BLIT _IOW('F', 0x21, __u8)
#define FBIO_BLIT 0x22
#endif

void CFrameBuffer::enableManualBlit()
{
#if !defined USE_OPENGL  
	unsigned char tmp = 1;
	
	if (ioctl(fd,FBIO_SET_MANUAL_BLIT, &tmp) < 0) 
	{
		perror("FB: FBIO_SET_MANUAL_BLIT");
		printf("FB: failed\n");
	}
	else 
	{
		m_manual_blit = 1;
	}
#endif	
}

void CFrameBuffer::disableManualBlit()
{
#if !defined USE_OPENGL  
	unsigned char tmp = 0;
	
	if (ioctl(fd,FBIO_SET_MANUAL_BLIT, &tmp) < 0) 
	{
		perror("FB: FBIO_SET_MANUAL_BLIT");
		printf("FB: failed\n");
	}
	else 
	{
		m_manual_blit = 0;
	}
#endif	
}

void CFrameBuffer::blit(int mode3d)
{
#if defined USE_OPENGL  
	mpGLThreadObj->blit();
#elif defined (__sh__)
	STMFBIO_BLT_DATA  bltData; 
	memset(&bltData, 0, sizeof(STMFBIO_BLT_DATA)); 

	bltData.operation  = BLT_OP_COPY;
	bltData.ulFlags |= BLT_OP_FLAGS_BLEND_SRC_ALPHA | BLT_OP_FLAGS_BLEND_DST_COLOR;

	// src
	bltData.srcOffset  = 1920 *1080 * 4;
	bltData.srcPitch   = xRes * 4; // stride

	bltData.src_left   = 0; 
	bltData.src_top    = 0; 
	bltData.src_right  = xRes; 
	bltData.src_bottom = yRes;

		
	bltData.srcFormat = SURF_ARGB8888;
	bltData.srcMemBase = STMFBGP_FRAMEBUFFER;
	
	// get variable screeninfo
	if (ioctl(fd, FBIOGET_VSCREENINFO, &screeninfo) == -1)
	{
		perror("frameBuffer <FBIOGET_VSCREENINFO>");
	}

	/* dst */
	bltData.dstOffset  = 0; 
	bltData.dstPitch   = screeninfo.xres * 4;

	bltData.dst_left   = 0; 
	bltData.dst_top    = 0;
	
	// right
	if(mode3d == THREE_SIDE_BY_SIDE)
		bltData.dst_right  = screeninfo.xres/2; 
	else
		bltData.dst_right  = screeninfo.xres; 
	
	// buttom
	if(mode3d == THREE_TOP_AND_BUTTOM)
		bltData.dst_bottom = screeninfo.yres/2;
	else
		bltData.dst_bottom = screeninfo.yres;

	bltData.dstFormat = SURF_ARGB8888;		
	bltData.dstMemBase = STMFBGP_FRAMEBUFFER;

	if ( (bltData.dst_right > screeninfo.xres) || (bltData.dst_bottom > screeninfo.yres) )
	{
		printf("CFrameBuffer::blit: values out of range desXb:%d desYb:%d\n", bltData.dst_right, bltData.dst_bottom);
	}

	if (ioctl(fd, STMFBIO_BLT, &bltData ) < 0) 
		perror("STMFBIO_BLIT");
	
	// sync bliter
	if(ioctl(fd, STMFBIO_SYNC_BLITTER) < 0)
		perror("ioctl STMFBIO_SYNC_BLITTER");
	
	if(mode3d != 0)
	{
		if(mode3d == THREE_SIDE_BY_SIDE)
			bltData.dst_left = screeninfo.xres/2;
		if(mode3d == THREE_TOP_AND_BUTTOM)
			bltData.dst_top = screeninfo.yres/2;
		
		
		bltData.dst_right  = screeninfo.xres;
		bltData.dst_bottom = screeninfo.yres;

		if (ioctl(fd, STMFBIO_BLT, &bltData) < 0)
			perror("ioctl STMFBIO_BLT");
		
		if(ioctl(fd, STMFBIO_SYNC_BLITTER) < 0)
			perror("ioctl STMFBIO_SYNC_BLITTER");
	}
#else
	// blit
	if (m_manual_blit == 1) 
	{
		if (ioctl(fd, FBIO_BLIT) < 0)
			perror("FBIO_BLIT");		
	}
#endif	
//#endif
}


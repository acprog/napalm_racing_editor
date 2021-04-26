/* naPalm Racing editor
  Copyright (C) 2004
  Author: Alexander Semenov <acmain@gmail.com>
*/
#ifndef _CFG_H_
#define _CFG_H_

#include "NREditorRsc.h"
#include "graphics.h"

// параметры экрана
typedef	unsigned char	color;
#ifdef COLOR_GRAY
	//#define	COLOR_DEPTH	4
	#define	BPP(x)	((x)>>1)
	#define	COLOR_SIZE	4
#endif

#ifdef COLOR_8
	//#define	COLOR_DEPTH	4
	#define	BPP(x)	(x)
	#define	COLOR_SIZE	8
#endif

#define	SCREEN_WIDTH	160
#define	SCREEN_HEIGHT	160
#define	SCREEN_OFFSET(x, y)	BPP( (x)+(y)*SCREEN_WIDTH )

#define	MAP_SIZE	128
#define	MAP_LEFT	(SCREEN_WIDTH-MAP_SIZE)
#define	MAP_TOP		(SCREEN_HEIGHT-MAP_SIZE)

extern	void error(const char *str, const char *module, int line);
#define	ERROR(str)	error(str, __FILE__, __LINE__)
#endif	// _CFG_H_

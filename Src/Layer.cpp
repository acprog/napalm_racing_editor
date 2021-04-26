/* naPalm Racing editor
  Copyright (C) 2004
  Author: Alexander Semenov <acmain@gmail.com>
*/
#include "layer.h"
#include "algorithm.h"

//==============================================================
//	Поверхность рисования
//==============================================================
layer::layer(const size<> &s, color fill)
	:image(s)
{
	clear(fill);
}



//==============================================================
void layer::redraw(bool tranparency)
{
	color	*screen=(color*)BmpGetBits(WinGetBitmap(WinGetActiveWindow()));
	if (tranparency)
		add(screen, point<>(MAP_LEFT, MAP_TOP));
	else
		put(screen, point<>(MAP_LEFT, MAP_TOP));
}



//==============================================================
void layer::pack_2bit(MemHandle h)
{
#ifdef COLOR_GRAY
	UInt8	index[16];
#endif

#ifdef COLOR_8
	UInt8	index[256];
#endif

	index[water]=0;
	index[grass]=1;
	index[sand]=2;
	index[block]=3;

	packed_2bit	*buffer=new packed_2bit[MAP_SIZE*MAP_SIZE/8],
				*dst=buffer;

	if (!buffer)
		ERROR("No free memory");

	#ifdef COLOR_GRAY
	packed_4bit	*src=(packed_4bit*)bits;
	for (int i=MAP_SIZE*MAP_SIZE/8; i--; src+=2, dst++)
	{
		(*dst)._00=index[src[0]._00];
		(*dst)._01=index[src[0]._01];
		(*dst)._10=index[src[0]._10];
		(*dst)._11=index[src[0]._11];
		(*dst)._20=index[src[1]._00];
		(*dst)._21=index[src[1]._01];
		(*dst)._30=index[src[1]._10];
		(*dst)._31=index[src[1]._11];
	}
	#endif

	#ifdef COLOR_8
	packed_8bit	*src=(packed_8bit*)bits;
	for (int i=MAP_SIZE*MAP_SIZE/8; i--; src+=4, dst++)
	{
		(*dst)._00=index[src[0]._00];
		(*dst)._01=index[src[0]._01];
		(*dst)._10=index[src[1]._00];
		(*dst)._11=index[src[1]._01];
		(*dst)._20=index[src[2]._00];
		(*dst)._21=index[src[2]._01];
		(*dst)._30=index[src[3]._00];
		(*dst)._31=index[src[3]._01];
	}
	#endif

	DmWrite(MemHandleLock(h), 0, buffer, MAP_SIZE*MAP_SIZE/4);
	MemHandleUnlock(h);
	delete[] buffer;
}


//==============================================================
void layer::pack_1bit(MemHandle h)
{
#ifdef COLOR_GRAY
	UInt8	index[16];
#endif

#ifdef COLOR_8
	UInt8	index[256];
#endif

	index[road]=1;
	index[no_road]=0;

	packed_1bit	*buffer=new packed_1bit[MAP_SIZE*MAP_SIZE/16],
				*dst=buffer;

#ifdef COLOR_GRAY
	packed_4bit	*src=(packed_4bit*)bits;
	for (int i=MAP_SIZE*MAP_SIZE/16; i--; src+=4, dst++)
	{
		(*dst)._00=index[src[0]._00];
		(*dst)._01=index[src[0]._01];
		(*dst)._10=index[src[0]._10];
		(*dst)._11=index[src[0]._11];
		(*dst)._20=index[src[1]._00];
		(*dst)._21=index[src[1]._01];
		(*dst)._30=index[src[1]._10];
		(*dst)._31=index[src[1]._11];
		(*dst)._40=index[src[2]._00];
		(*dst)._41=index[src[2]._01];
		(*dst)._50=index[src[2]._10];
		(*dst)._51=index[src[2]._11];
		(*dst)._60=index[src[3]._00];
		(*dst)._61=index[src[3]._01];
		(*dst)._70=index[src[3]._10];
		(*dst)._71=index[src[3]._11];
	}
#endif

#ifdef COLOR_8
	packed_8bit	*src=(packed_8bit*)bits;
	for (int i=MAP_SIZE*MAP_SIZE/16; i--; src+=8, dst++)
	{
		(*dst)._00=index[src[0]._00];
		(*dst)._01=index[src[0]._01];
		(*dst)._10=index[src[1]._00];
		(*dst)._11=index[src[1]._01];
		(*dst)._20=index[src[2]._00];
		(*dst)._21=index[src[2]._01];
		(*dst)._30=index[src[3]._00];
		(*dst)._31=index[src[3]._01];
		(*dst)._40=index[src[4]._00];
		(*dst)._41=index[src[4]._01];
		(*dst)._50=index[src[5]._00];
		(*dst)._51=index[src[5]._01];
		(*dst)._60=index[src[6]._00];
		(*dst)._61=index[src[6]._01];
		(*dst)._70=index[src[7]._00];
		(*dst)._71=index[src[7]._01];
	}
#endif

	DmWrite(MemHandleLock(h), 0, buffer, MAP_SIZE*MAP_SIZE/8);
	MemHandleUnlock(h);
	delete[] buffer;
}


//==============================================================
void layer::unpack_2bit(MemHandle h)
{
	UInt8	index[4];
	index[0]=water;
	index[1]=grass;
	index[2]=sand;
	index[3]=block;

#ifdef COLOR_GRAY
	packed_4bit	*dst=(packed_4bit*)bits;
#endif

#ifdef COLOR_8
	packed_8bit	*dst=(packed_8bit*)bits;
#endif

	packed_2bit	*buffer=new packed_2bit[MAP_SIZE*MAP_SIZE/8],
				*src=buffer;

	if (!buffer)
		ERROR("No Free memory");

	if (MemMove(buffer, MemHandleLock(h), MAP_SIZE*MAP_SIZE/4)!=errNone)
		ERROR("MemMove()");
	if (MemHandleUnlock(h)!=errNone)
		ERROR("MemHandleUnlock()");

#ifdef COLOR_GRAY
	for (int i=MAP_SIZE*MAP_SIZE/8; i--; src++, dst+=2)
	{
		dst[0]._00=index[(*src)._00];
		dst[0]._01=index[(*src)._01];
		dst[0]._10=index[(*src)._10];
		dst[0]._11=index[(*src)._11];
		dst[1]._00=index[(*src)._20];
		dst[1]._01=index[(*src)._21];
		dst[1]._10=index[(*src)._30];
		dst[1]._11=index[(*src)._31];
	}
#endif

#ifdef COLOR_8
	for (int i=MAP_SIZE*MAP_SIZE/8; i--; src++, dst+=4)
	{
		dst[0]._00=index[(*src)._00];
		dst[0]._01=index[(*src)._01];
		dst[1]._00=index[(*src)._10];
		dst[1]._01=index[(*src)._11];
		dst[2]._00=index[(*src)._20];
		dst[2]._01=index[(*src)._21];
		dst[3]._00=index[(*src)._30];
		dst[3]._01=index[(*src)._31];
	}
#endif

	delete[] buffer;
}


//==============================================================
void layer::unpack_1bit(MemHandle h)
{
	UInt8	index[2];
	index[1]=road;
	index[0]=no_road;

#ifdef COLOR_GRAY
	packed_4bit	*dst=(packed_4bit*)bits;
#endif

#ifdef COLOR_8
	packed_8bit	*dst=(packed_8bit*)bits;
#endif

	packed_1bit	*buffer=new packed_1bit[MAP_SIZE*MAP_SIZE/16],
				*src=buffer;
	
	if (!buffer)
		ERROR("No free memory");
	
	if (MemMove(buffer, MemHandleLock(h), MAP_SIZE*MAP_SIZE/8)!=errNone)
		ERROR("MemMove()");
	if (MemHandleUnlock(h)!=errNone)
		ERROR("MemHandleUnlock()");
	
	#ifdef COLOR_GRAY
	for (int i=MAP_SIZE*MAP_SIZE/16; i--; src++, dst+=4)
	{
		dst[0]._00=index[(*src)._00];
		dst[0]._01=index[(*src)._01];
		dst[0]._10=index[(*src)._10];
		dst[0]._11=index[(*src)._11];
		dst[1]._00=index[(*src)._20];
		dst[1]._01=index[(*src)._21];
		dst[1]._10=index[(*src)._30];
		dst[1]._11=index[(*src)._31];
		dst[2]._00=index[(*src)._40];
		dst[2]._01=index[(*src)._41];
		dst[2]._10=index[(*src)._50];
		dst[2]._11=index[(*src)._51];
		dst[3]._00=index[(*src)._60];
		dst[3]._01=index[(*src)._61];
		dst[3]._10=index[(*src)._70];
		dst[3]._11=index[(*src)._71];
	}
	#endif
	
	#ifdef COLOR_8
	for (int i=MAP_SIZE*MAP_SIZE/16; i--; src++, dst+=8)
	{
		dst[0]._00=index[(*src)._00];
		dst[0]._01=index[(*src)._01];
		dst[1]._00=index[(*src)._10];
		dst[1]._01=index[(*src)._11];
		dst[2]._00=index[(*src)._20];
		dst[2]._01=index[(*src)._21];
		dst[3]._00=index[(*src)._30];
		dst[3]._01=index[(*src)._31];
		dst[4]._00=index[(*src)._40];
		dst[4]._01=index[(*src)._41];
		dst[5]._00=index[(*src)._50];
		dst[5]._01=index[(*src)._51];
		dst[6]._00=index[(*src)._60];
		dst[6]._01=index[(*src)._61];
		dst[7]._00=index[(*src)._70];
		dst[7]._01=index[(*src)._71];
	}
	#endif

	delete[] buffer;
}


//==============================================================
void layer::big_point(const point<> &p, color c, int size)
{
	color	*pix;
	switch (size)
	{
#ifdef COLOR_8
	case 1:
		put_pixel(p, c);
		break;

	case 3:
		pix=&((*this)[p]);
		*pix=c;
		*(--pix)=c;
		pix+=2;
		*pix=c;
		pix-=width+offset+1;
		*pix=c;
		pix+=2*(width+offset);
		*pix=c;
		break;

	case 5:
		static	int mask[][8]=
		{
			{0, 1, 1, 1, 0, 0, 0, 0},
			{1, 1, 1, 1, 1, 0, 0, 0},
			{1, 1, 1, 1, 1, 0, 0, 0},
			{1, 1, 1, 1, 1, 0, 0, 0},
			{0, 1, 1, 1, 0, 0, 0, 0}
		};
		pix=&((*this)[p]);
		pix-=2*(width+offset+1);
		for (int y=0; y<5; y++, pix+=width+offset-5)
			for (int x=0; x<5; x++, pix++)
				if (mask[x][y])
					*pix=c;
		break;
#endif

#ifdef COLOR_GRAY
	case 1:
		put_pixel(p, c&0xf);
		break;

	case 3:
		pix=&((*this)[p]);
		pix-=(width+offset)/2;
		if (p.x & 1)
		{
			*pix&=0xf0;
			*pix|=c&0xf;
			pix++;
			*pix=c;

			pix+=(width+offset)/2-1;	// След. строка
			*pix&=0xf0;
			*pix|=c&0xf;
			pix++;
			*pix=c;

			pix+=(width+offset)/2-1;	// След. строка
			*pix&=0xf0;
			*pix|=c&0xf;
			pix++;
			*pix=c;
		}
		else
		{
			*pix=c;
			pix++;
			*pix&=0xf;
			*pix|=c&0xf0;

			pix+=(width+offset)/2-1;	// След. строка
			*pix=c;
			pix++;
			*pix&=0xf;
			*pix|=c&0xf0;

			pix+=(width+offset)/2-1;	// След. строка
			*pix=c;
			pix++;
			*pix&=0xf;
			*pix|=c&0xf0;
		}
		break;

	case 5:
		pix=&((*this)[p]);
		pix-=width+offset;
		if (p.x & 1)
		{
			*pix&=0xf0;
			*pix|=c&0xf;
			*(++pix)=c;

			pix+=(width+offset)/2-1;	// След. строка
			*(pix++)=c;
			*(pix++)=c;
			*pix&=0xf;
			*pix|=c&0xf0;

			pix+=(width+offset)/2-2;	// След. строка
			*(pix++)=c;
			*(pix++)=c;
			*pix&=0xf;
			*pix|=c&0xf0;

			pix+=(width+offset)/2-2;	// След. строка
			*(pix++)=c;
			*(pix++)=c;
			*pix&=0xf;
			*pix|=c&0xf0;

			pix+=(width+offset)/2-2;	// След. строка
			*pix&=0xf0;
			*pix|=c&0xf;
			*(++pix)=c;
		}
		else
		{
			*(pix++)=c;
			*pix&=0xf;
			*pix|=c&0xf0;

			pix+=(width+offset)/2-2;	// След. строка
			*pix&=0xf0;
			*pix|=c&0xf;
			*(++pix)=c;
			*(++pix)=c;

			pix+=(width+offset)/2-2;	// След. строка
			*pix&=0xf0;
			*pix|=c&0xf;
			*(++pix)=c;
			*(++pix)=c;

			pix+=(width+offset)/2-2;	// След. строка
			*pix&=0xf0;
			*pix|=c&0xf;
			*(++pix)=c;
			*(++pix)=c;

			pix+=(width+offset)/2-1;	// След. строка
			*(pix++)=c;
			*pix&=0xf;
			*pix|=c&0xf0;
		}
		break;
#endif
	}		
}


//==============================================================
void layer::line(const point<> &from, const point<> &to, color c, int size)
{
#ifdef COLOR_GRAY
	c|=(c<<4);
#endif

#define abs(x)	((x)>0 ? (x) : -(x))
	int	dx = abs ( to.x - from.x );
	int	dy = abs ( to.y - from.y );
	int	sx = to.x >= from.x ? 1 : -1;
	int	sy = to.y >= from.y ? 1 : -1;

	if ( dy <= dx )
	{
		int	d  = ( dy << 1 ) - dx;
		int	d1 = dy << 1;
		int	d2 = ( dy - dx ) << 1;

		big_point( from, c, size);

		point<>	p(from.x + sx, from.y);
		for ( int i = 1; i <= dx; i++, p.x += sx )
		{
			if ( d > 0 )
			{
				d += d2;
				p.y += sy;
			}
			else
				d += d1;

			big_point(p, c, size);
		}
	}
	else
	{
		int	d  = ( dx << 1 ) - dy;
		int	d1 = dx << 1;
		int	d2 = ( dx - dy ) << 1;

		big_point( from, c, size);

		point<>	p(from.x, from.y + sy);
		for ( int i = 1; i <= dy; i++, p.y += sy )
		{
			if ( d > 0 )
			{
				d += d2;
				p.x += sx;
			}
			else
				d += d1;

			big_point( p, c, size);
		}
	}
}

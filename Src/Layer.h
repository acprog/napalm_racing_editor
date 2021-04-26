/* naPalm Racing editor
  Copyright (C) 2004
  Author: Alexander Semenov <acmain@gmail.com>
*/
#ifndef _LAYER_H_
#define _LAYER_H_

#include "image.h"


// цвета поверхностей
#ifdef COLOR_GRAY
enum	terrains
{
	water=8,
	grass=10,
	sand=6,
	block=0,

	road=5,
	no_road=0,
};
#endif

#ifdef COLOR_8
enum	terrains
{
	water=0x63,
	grass=0xbb,
	sand=0x7f,
	block=0xe0,//0xd7,

	road=32,
	no_road=0
};
#endif


	//============================================================
	//	Упакованные биты
	struct	packed_8bit
	{
		UInt16	_00	:8,
				_01	:8;
	};
	//--------------------------------------
	struct	packed_4bit
	{
		UInt16	_00	:4,
				_01	:4,
				_10	:4,
				_11	:4;
	};
	//--------------------------------------
	struct	packed_2bit
	{
		UInt16	_00	:2,
				_01	:2,
				_10	:2,
				_11	:2,
				_20	:2,
				_21	:2,
				_30	:2,
				_31	:2;
	};
	//--------------------------------------
	struct	packed_1bit
	{
		UInt16	_00	:1,
				_01	:1,
				_10	:1,
				_11	:1,
				_20	:1,
				_21	:1,
				_30	:1,
				_31	:1,
				_40	:1,
				_41	:1,
				_50	:1,
				_51	:1,
				_60	:1,
				_61	:1,
				_70	:1,
				_71	:1;
	};


//==============================================================
//	Слой рисования
//==============================================================
class layer : public image
{
public:
	layer(){};
	layer(const size<> &s, color fill);

	//-----------------------------------------------------
	layer(const size<> &s, color *_bits, int _offset=0)
		:image(s, _bits, _offset)
	{
	}

	void line(const point<> &from, const point<> &to, color c, int size);
	void big_point(const point<> &p, color c, int size);
	void redraw(bool tranparency);
	void pack_2bit(MemHandle h);
	void pack_1bit(MemHandle h);
	void unpack_2bit(MemHandle h);
	void unpack_1bit(MemHandle h);
};

#endif	// _LAYER_H_

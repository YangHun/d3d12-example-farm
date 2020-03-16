#pragma once
#ifndef _UI_H
#define _UI_H

using namespace DirectX;
using Microsoft::WRL::ComPtr;

struct Rect
{
	FLOAT left;
	FLOAT top;
	FLOAT right;
	FLOAT bottom;
};



struct TextDesc
{
	UINT id;
	DWRITE_TEXT_ALIGNMENT textAlignment = DWRITE_TEXT_ALIGNMENT_LEADING;
	DWRITE_PARAGRAPH_ALIGNMENT paragraphAlignment = DWRITE_PARAGRAPH_ALIGNMENT_CENTER;
	
	ComPtr<ID2D1Brush> brush;
	ComPtr<IDWriteTextFormat> textFormat;
};


struct Sprite
{
	UINT id;
	std::string name;
	std::wstring filePath;
	
	ComPtr<ID2D1Bitmap1> bitmap;
};

#endif
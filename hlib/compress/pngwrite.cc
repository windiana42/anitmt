/*
 * pngwrite.cc
 * 
 * Implementation of PNG image file writer (using libpng). 
 * 
 * Copyright (c) 2003--2004 by Wolfgang Wieser (wwieser@gmx.de) 
 * 
 * This file may be distributed and/or modified under the terms of the 
 * GNU General Public License version 2 as published by the Free Software 
 * Foundation. (See COPYING.GPL for details.)
 * 
 * This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
 * WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 * 
 */


#define HLIB_IN_HLIB 1
#include <hlib/prototypes.h>  /* MUST BE FIRST */

#include "pngwrite.h"

//**** Progress callback: 
//  mypng_progress(png_structp /*png_ptr*/,png_uint_32 /*row*/,int /*pass*/)
//  png_set_write_status_fn(png_ptr,mypng_progress);

//**** set other zlib parameters: 
//  png_set_compression_mem_level(png_ptr,8);
//  png_set_compression_strategy(png_ptr,Z_DEFAULT_STRATEGY);
//  png_set_compression_window_bits(png_ptr,15);
//  png_set_compression_method(png_ptr,8);

//png_set_PLTE(png_ptr,info_ptr,
//	/*palette=*/,           // array of png_color
//	/*num_palette*/ );

//png_set_gAMA(png_ptr,info_ptr,
//	/*gamma=*/ );

//png_set_sRGB(png_ptr,info_ptr,PNG_SRGB_INTENT_??);
//png_set_sRGB_gAMA_and_cHRM(png_ptr,info_ptr,PNG_SRGB_INTENT_??);

//png_set_sBIT(png_ptr,info_ptr,16);


void PNGWriter::error_func(png_structp png_ptr,png_const_charp str)
{
	PNGWriter *pw=(PNGWriter*)png_get_error_ptr(png_ptr);
	pw->curr_error=str;
}

void PNGWriter::warning_func(png_structp /*png_ptr*/,png_const_charp /*str*/)
{
	//PNGWriter *pw=(PNGWriter*)png_get_error_ptr(png_ptr);
	/*currently nothing*/
}


void PNGWriter::_ClearTextChunks()
{
	if(!n_text_chunks)  return;
	
	if(png_ptr && info_ptr)
	{  png_set_text(png_ptr,info_ptr,text_array,0);  }
	
	for(int i=0; i<n_text_chunks; i++)
	{
		LFree(text_array[i].key);
		LFree(text_array[i].text);
	}
	text_array=(png_text*)LFree(text_array);
	n_text_chunks=0;
}


int PNGWriter::Open(const char *name,int compress_level)
{
	if(wstate!=WS_None)
	{  return(-4);  }
	
	fp=fopen(name,"wb");
	if(!fp)
	{  return(-2);  }
	
	png_ptr=png_create_write_struct(PNG_LIBPNG_VER_STRING,
		this,&error_func,&warning_func);
	if(!png_ptr)
	{
		fclose(fp);  fp=NULL;
		return(-1);
	}
	
	info_ptr=png_create_info_struct(png_ptr);
	if(!info_ptr)
	{
		fclose(fp);  fp=NULL;
		png_destroy_write_struct(&png_ptr,&info_ptr);  png_ptr=NULL;
		return(-1);
	}
	
	if(setjmp(png_jmpbuf(png_ptr)))
	{
		fclose(fp);  fp=NULL;
		png_destroy_write_struct(&png_ptr,&info_ptr);
		png_ptr=NULL;  info_ptr=NULL;
		return(-3);
	}
	
	png_init_io(png_ptr,fp);
	
	wstate=WS_Opened;
	
	if(compress_level!=-999)
	{  png_set_compression_level(png_ptr,compress_level);  }
	
	return(0);
}


int PNGWriter::SetSize(unsigned int width,unsigned int height,
	int bit_depth,int color_type)
{
	if(wstate!=WS_Opened)
	{  return(-4);  }
	if(setjmp(png_jmpbuf(png_ptr)))
	{  return(-3);  }
	png_set_IHDR(png_ptr,info_ptr,
		width,height,bit_depth,color_type,
		PNG_INTERLACE_NONE,
		PNG_COMPRESSION_TYPE_DEFAULT,PNG_FILTER_TYPE_DEFAULT);
	return(0);
}


int PNGWriter::SetFilter(int filter)
{
	if(wstate!=WS_Opened)
	{  return(-4);  }
	if(setjmp(png_jmpbuf(png_ptr)))
	{  return(-3);  }
	png_set_filter(png_ptr,0,filter);
	return(0);
}


int PNGWriter::SetGamma(double gamma)
{
	if(wstate!=WS_Opened)
	{  return(-4);  }
	if(setjmp(png_jmpbuf(png_ptr)))
	{  return(-3);  }
	png_set_gAMA(png_ptr,info_ptr,gamma);
	return(0);
}


int PNGWriter::SetBasicBitOps(int flags)
{
	if(wstate!=WS_Opened && wstate!=WS_HdrWritten)
	{  return(-4);  }
	if(setjmp(png_jmpbuf(png_ptr)))
	{  return(-3);  }
	if(flags & PNGW_Swap16)
	{  png_set_swap(png_ptr);  }
	if(flags & PNGW_InvertAlpha)
	{  png_set_invert_alpha(png_ptr);  }
	if(flags & PNGW_ColorBGR)
	{  png_set_bgr(png_ptr);  }
	if(flags & PNGW_PackSwap)
	{  png_set_packswap(png_ptr);  }
	if(flags & PMGW_InvertMono)
	{  png_set_invert_mono(png_ptr);  }
	return(0);
}


int PNGWriter::AddTextChunk(const char *_key, const char *_text,
	ssize_t text_len,int do_compress)
{
	if(wstate!=WS_HdrWritten && wstate!=WS_Opened)
	{  return(-4);  }
	
	// Allocate copies: 
	char *key=LStrDup(_key);
	if(!key)  return(-1);
	if(text_len<0)
	{  text_len=_text ? strlen(_text) : 0;  }
	char *text=NULL;
	if(text_len)
	{
		text=(char*)LMalloc(text_len);
		if(!text)
		{  LFree(key);  return(-1);  }
		memcpy(text,_text,text_len);
	}
	
	png_text *txtp=text_array;
	text_array=(png_text*)LRealloc(text_array,
		sizeof(png_text)*(n_text_chunks+1));
	if(!text_array)
	{
		text_array=txtp;
		LFree(key);
		LFree(text);
		return(-1);
	}
	
	if(do_compress<0)
	{  do_compress=(text_len>=512);  }
	
	txtp=&text_array[n_text_chunks++];
	txtp->compression=(do_compress ? 
		PNG_TEXT_COMPRESSION_zTXt : PNG_TEXT_COMPRESSION_NONE);
	txtp->key=key;
	txtp->text=text;
	txtp->text_length=text_len;
	//txtp->itxt_length=0;
	//txtp->lang=NULL;
	//txtp->lang_key=NULL;
	
	return(0);
}


int PNGWriter::WriteHeader()
{
	if(wstate!=WS_Opened)
	{  return(-4);  }
	if(setjmp(png_jmpbuf(png_ptr)))
	{  return(-3);  }
	
	if(n_text_chunks)
	{  png_set_text(png_ptr,info_ptr,text_array,n_text_chunks);  }
	
	png_write_info(png_ptr,info_ptr);
	
	_ClearTextChunks();
	
	wstate=WS_HdrWritten;
	return(0);
}


ssize_t PNGWriter::GetRowSize()
{
	if(wstate!=WS_HdrWritten && wstate!=WS_Opened)
	{  return(-4);  }
	if(setjmp(png_jmpbuf(png_ptr)))
	{  return(-3);  }
	ssize_t rowbytes=png_get_rowbytes(png_ptr,info_ptr);
	return(rowbytes);
}


int PNGWriter::WriteRow(const char *row_buf,int do_flush)
{
	if(wstate!=WS_HdrWritten)
	{  return(-4);  }
	if(setjmp(png_jmpbuf(png_ptr)))
	{  return(-3);  }
	png_write_row(png_ptr,(png_byte*)row_buf);
	if(do_flush)
	{  png_write_flush(png_ptr);  }
	return(0);
}

int PNGWriter::WriteRows(const char **_row_pointers,unsigned int nrows,
	int do_flush)
{
	if(wstate!=WS_HdrWritten)
	{  return(-4);  }
	if(setjmp(png_jmpbuf(png_ptr)))
	{  return(-3);  }
	png_bytep *row_pointers=(png_bytep*)_row_pointers;
	png_write_rows(png_ptr,row_pointers,nrows);
	if(do_flush)
	{  png_write_flush(png_ptr);  }
	return(0);
}


int PNGWriter::WriteTail()
{
	if(wstate!=WS_HdrWritten)
	{  return(-4);  }
	if(setjmp(png_jmpbuf(png_ptr)))
	{  return(-3);  }
	
	if(n_text_chunks)
	{  png_set_text(png_ptr,info_ptr,text_array,n_text_chunks);  }
	
	png_write_end(png_ptr,info_ptr);
	
	_ClearTextChunks();
	
	wstate=WS_TailWritten;
	return(0);
}


int PNGWriter::Close()
{
	int rv=0;
	
	if(png_ptr)
	{
		if(setjmp(png_jmpbuf(png_ptr)))
		{  rv=-3;  goto skipit;  }
		_ClearTextChunks();
		png_destroy_write_struct(&png_ptr,&info_ptr);
		skipit:;
	}
	else
	{  _ClearTextChunks();  }
	
	if(fp)
	{  fclose(fp);  }
	
	fp=NULL;
	png_ptr=NULL;
	info_ptr=NULL;
	wstate=WS_None;
	curr_error=NULL;
	
	return(rv);
}


PNGWriter::PNGWriter(int * /*failflag*/)
{
	fp=NULL;
	png_ptr=NULL;
	info_ptr=NULL;
	n_text_chunks=0;
	text_array=NULL;
	wstate=WS_None;
	curr_error=NULL;
}

PNGWriter::~PNGWriter()
{
	Close();
}

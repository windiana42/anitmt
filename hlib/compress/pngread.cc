/*
 * pngread.cc
 * 
 * Implementation of PNG image file reader (using libpng). 
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

#include "pngread.h"


/*
IN THE ORDER THEY SHOULD BE CALLED: 
       
SetBasicBitOps()
 -> SetRBG2GrayConversion() 
 -> SetRBG2GrayConversion()

SetBasicBitOps()

...gamma...??????????

SetBasicBitOps()

StartReading()
*/


void PNGReader::error_func(png_structp png_ptr,png_const_charp str)
{
	PNGReader *pw=(PNGReader*)png_get_error_ptr(png_ptr);
	pw->curr_error=str;
}

void PNGReader::warning_func(png_structp /*png_ptr*/,png_const_charp /*str*/)
{
	//PNGReader *pw=(PNGReader*)png_get_error_ptr(png_ptr);
	/*currently nothing*/
}


int PNGReader::Open(const char *name)
{
	if(rstate!=RS_None)
	{  return(-4);  }
	
	fp=fopen(name,"rb");
	if(!fp)
	{  return(-2);  }
	
	// Check signature in PNG file header: 
	char header[8];
	if(fread(header,1,8,fp)!=8 || png_sig_cmp((png_byte*)header,0,8))
	{  return(-10);  }
	
	png_ptr=png_create_read_struct(PNG_LIBPNG_VER_STRING,
		this,&error_func,&warning_func);
	if(!png_ptr)
	{
		fclose(fp);  fp=NULL;
		return(-1);
	}
	
	info_ptr=png_create_info_struct(png_ptr);
	//end_info=png_create_info_struct(png_ptr);
	if(!info_ptr /*|| !end_info*/)
	{
		fclose(fp);  fp=NULL;
		png_destroy_read_struct(&png_ptr,&info_ptr,/*&end_info*/NULL);
		png_ptr=NULL;  info_ptr=NULL;  //end_info=NULL;
		return(-1);
	}
	
	if(setjmp(png_jmpbuf(png_ptr)))
	{
		fclose(fp);  fp=NULL;
		png_destroy_read_struct(&png_ptr,&info_ptr,/*&end_info*/NULL);
		png_ptr=NULL;  info_ptr=NULL;  //end_info=NULL;
		return(-3);
	}
	
	png_init_io(png_ptr,fp);
	png_set_sig_bytes(png_ptr,8);
	
	png_read_info(png_ptr,info_ptr);
	
	rstate=RS_Opened;
	
	return(0);
}


int PNGReader::GetSize(unsigned int *width,unsigned int *height,
	int *bit_depth,int *color_type)
{
	if(rstate!=RS_Opened && rstate!=RS_Reading && rstate!=RS_Done)
	{  return(-4);  }
	if(setjmp(png_jmpbuf(png_ptr)))
	{  return(-3);  }
	int filter_method;
	int compression_type;
	int _bit_depth,_color_type;
	png_uint_32 _width,_height;
	png_get_IHDR(png_ptr,info_ptr,
		&_width,&_height,&_bit_depth,&_color_type,
		/*interlace_type=*/NULL,&compression_type,&filter_method);
	if(width)   *width=_width;
	if(height)  *height=_height;
	if(bit_depth)   *bit_depth=_bit_depth;
	if(color_type)  *color_type=_color_type;
	if(compression_type!=PNG_COMPRESSION_TYPE_BASE || 
	   filter_method!=PNG_FILTER_TYPE_BASE)
	{  return(-10);  }
	return(0);
}


ssize_t PNGReader::GetRowSize()
{
	if(rstate!=RS_Opened && rstate!=RS_Reading && rstate!=RS_Done)
	{  return(-4);  }
	if(setjmp(png_jmpbuf(png_ptr)))
	{  return(-3);  }
	ssize_t rowbytes=png_get_rowbytes(png_ptr,info_ptr);
	return(rowbytes);
}


int PNGReader::GetNChannels()
{
	if(rstate!=RS_Opened && rstate!=RS_Reading && rstate!=RS_Done)
	{  return(-4);  }
	if(setjmp(png_jmpbuf(png_ptr)))
	{  return(-3);  }
	int nchannels=png_get_channels(png_ptr,info_ptr);
	return(nchannels);
}


int PNGReader::GetInterlaceType()
{
	if(rstate!=RS_Opened && rstate!=RS_Reading && rstate!=RS_Done)
	{  return(-4);  }
	if(setjmp(png_jmpbuf(png_ptr)))
	{  return(-3);  }
	int itype=png_get_interlace_type(png_ptr,info_ptr);
	return(itype);
}


int PNGReader::GetGamma(double *gamma)
{
	if(rstate!=RS_Opened && rstate!=RS_Reading && rstate!=RS_Done)
	{  return(-4);  }
	if(setjmp(png_jmpbuf(png_ptr)))
	{  return(-3);  }
	if(png_get_gAMA(png_ptr,info_ptr,gamma))
	{
		// File gamma present. 
		return(0);
	}
	*gamma=0.454545454545454545;  // = 1/2.2
	return(1);
}


int PNGReader::GetTextChunksRaw(png_text **array,int *nchunks)
{
	if(rstate!=RS_Opened && rstate!=RS_Reading && rstate!=RS_Done)
	{  return(-4);  }
	if(setjmp(png_jmpbuf(png_ptr)))
	{  return(-3);  }
	png_get_text(png_ptr,info_ptr,array,nchunks);
	return(0);
}


int PNGReader::SetBasicBitOps(int flags,int sane)
{
	if(rstate!=RS_Opened)
	{  return(-4);  }
	if(setjmp(png_jmpbuf(png_ptr)))
	{  return(-3);  }
	
	int color_type=0;
	int bit_depth=0;
	if(sane)
	{
		color_type=png_get_color_type(png_ptr,info_ptr);
		bit_depth=png_get_bit_depth(png_ptr,info_ptr);
	}
	
	if(flags & PNGR_Palette2RGB)
	{
		if(!sane || color_type==PNG_COLOR_TYPE_PALETTE)
		{  png_set_palette_to_rgb(png_ptr);  }
	}
	if(flags & PNGR_GrayExpand)
	{
		if(!sane || (color_type==PNG_COLOR_TYPE_GRAY && bit_depth<8))
		{  png_set_expand_gray_1_2_4_to_8(png_ptr);  }
	}
	if(flags & PNGR_AddAlphaIfAvail)
	{
		// Independent from sane parameter: 
		if(png_get_valid(png_ptr,info_ptr,PNG_INFO_tRNS))
		{  png_set_tRNS_to_alpha(png_ptr);  }
	}
	if(flags & PNGR_Strip16)
	{
		if(!sane || bit_depth==16)
		{  png_set_strip_16(png_ptr);  }
	}
	if(flags & PNGR_StripAlpha)
	{
		if(!sane || color_type & PNG_COLOR_MASK_ALPHA)
		{  png_set_strip_alpha(png_ptr);  }
	}
	if(flags & PNGR_InvertAlpha && !(flags & PNGR_StripAlpha))
	{
		if(!sane || color_type & PNG_COLOR_MASK_ALPHA)
		{  png_set_invert_alpha(png_ptr);  }
	}
	if(flags & PNGR_Unpack124)
	{
		if(!sane || bit_depth<8)
		{  png_set_packing(png_ptr);  }
	}
	// if(png_get_sBIT(png_ptr, info_ptr, &sig_bit)) -> png_set_shift(png_ptr, sig_bit);
	if(flags & PNGR_ColorBGR)
	{
		if(!sane || (color_type==PNG_COLOR_TYPE_RGB ||
               color_type==PNG_COLOR_TYPE_RGB_ALPHA))
		{  png_set_bgr(png_ptr);  }
	}
	// --<call SetInsertFiller() here>--
	if(flags & PNGR_SwapAlpha)
	{
		if(!sane || color_type==PNG_COLOR_TYPE_RGB_ALPHA)
		{  png_set_swap_alpha(png_ptr);  }
	}
	if(flags & PNGR_Gray2RBG)
	{
		if(!sane || (color_type==PNG_COLOR_TYPE_GRAY || 
			color_type==PNG_COLOR_TYPE_GRAY_ALPHA) )
		{  png_set_gray_to_rgb(png_ptr);  }
	}
	// --<call SetRBG2GrayConversion() here>--
	// --<set background (if needed) here>--
	// --<call gamma correction stuff here>--
	if(flags & PNGR_InvertGray)
	{
		if( !sane || (bit_depth==1 && color_type==PNG_COLOR_TYPE_GRAY) || 
	    	(color_type==PNG_COLOR_TYPE_GRAY || 
        	 color_type==PNG_COLOR_TYPE_GRAY_ALPHA) )
    	{  png_set_invert_mono(png_ptr);  }
	}
	if(flags & PNGR_Swap16)
	{
		if(!sane || bit_depth==16)
		{  png_set_swap(png_ptr);  }
	}
	if(flags & PNGR_PackSwap)
	{
		if(!sane || bit_depth<8)
		{  png_set_packswap(png_ptr);  }
	}
	
	return(0);
}


int PNGReader::SetInsertFiller(u_int16_t filler_value,int location)
{
	if(rstate!=RS_Opened)
	{  return(-4);  }
	if(setjmp(png_jmpbuf(png_ptr)))
	{  return(-3);  }
	
	//if(color_type==PNG_COLOR_TYPE_RGB)
	{  png_set_filler(png_ptr,filler_value,
		location<0 ? PNG_FILLER_BEFORE : PNG_FILLER_AFTER);  }
	return(0);
}


int PNGReader::SetRBG2GrayConversion(int red_weight,int green_weight)
{
	if(rstate!=RS_Opened)
	{  return(-4);  }
	if(setjmp(png_jmpbuf(png_ptr)))
	{  return(-3);  }
	
	//if(color_type==PNG_COLOR_TYPE_RGB || color_type==PNG_COLOR_TYPE_RGB_ALPHA)
	{  png_set_rgb_to_gray_fixed(png_ptr,/*error_action=*/1,
		red_weight,green_weight);  }
	// If you have set error_action = 1 or 2, you can later check 
	// whether the image really was gray, after  processing the image 
	// rows, with the png_get_rgb_to_gray_status(png_ptr) function. 
	// It will return a png_byte that is zero if the image was gray 
	// or 1 if there were any non-gray pixels. 
	return(0);
}


int PNGReader::SetGamma(double screen_gamma,double file_gamma)
{
	if(rstate!=RS_Opened)
	{  return(-4);  }
	if(setjmp(png_jmpbuf(png_ptr)))
	{  return(-3);  }
	
	if(file_gamma<0.0 && !png_get_gAMA(png_ptr,info_ptr,&file_gamma))
		file_gamma=0.454545454545454545;  // = 1/2.2; No file gamma present. 
	
	png_set_gamma(png_ptr,screen_gamma,file_gamma);
	
	return(0);
}


int PNGReader::StartReading(int do_update_header)
{
	if(rstate!=RS_Opened)
	{  return(-4);  }
	if(setjmp(png_jmpbuf(png_ptr)))
	{  return(-3);  }
	
	// Can also be called if image is not interlaced. 
	interlacing_passes=png_set_interlace_handling(png_ptr);
	
	if(do_update_header)
	{  png_read_update_info(png_ptr,info_ptr);  }
	else
	{  png_start_read_image(png_ptr);  }
	
	rstate=RS_Reading;
	
	return(0);
}


#include <assert.h>
int PNGReader::ReadRow(char *row_buf)
{
	if(rstate!=RS_Reading)
	{  return(-4);  }
	// Must probably use ReadImage() if interlacing_passes!=1. 
	if(interlacing_passes!=1)
	{  return(-10);  }
	if(setjmp(png_jmpbuf(png_ptr)))
	{  return(-3);  }
	png_read_row(png_ptr,(png_byte*)row_buf,NULL);
	return(0);
}


int PNGReader::ReadImage(char **_row_pointers,unsigned int nrows)
{
	if(rstate!=RS_Reading)
	{  return(-4);  }
	// For interlaced images, the _complete_ image must be read in at once. 
	if(interlacing_passes!=1 && nrows!=png_get_image_height(png_ptr,info_ptr))
	{  return(-10);  }
	if(setjmp(png_jmpbuf(png_ptr)))
	{  return(-3);  }
	
	png_bytep *row_pointers=(png_bytep*)_row_pointers;
	for(int i=0; i<interlacing_passes; i++)
	{  png_read_rows(png_ptr,row_pointers,NULL,nrows);  }
	
	return(0);
}


int PNGReader::ReadTail()
{
	if(rstate!=RS_Reading)
	{  return(-4);  }
	if(setjmp(png_jmpbuf(png_ptr)))
	{  return(-3);  }
	
	png_read_end(png_ptr,info_ptr);
	
	rstate=RS_Done;
	
	return(0);
}


int PNGReader::Close()
{
	int rv=0;
	if(png_ptr)
	{
		if(setjmp(png_jmpbuf(png_ptr)))
		{  rv=-3;  goto skipit;  }
		//if(rstate==RS_Reading)
		//{  png_read_end(png_ptr,NULL);  }
		png_destroy_read_struct(&png_ptr,&info_ptr,/*&end_info*/NULL);
		skipit:;
	}
	if(fp)
	{  fclose(fp);  }
	
	fp=NULL;
	png_ptr=NULL;
	info_ptr=NULL;
	//end_info=NULL;
	rstate=RS_None;
	curr_error=NULL;
	interlacing_passes=-1;
	
	return(rv);
}


PNGReader::PNGReader(int * /*failflag*/)
{
	fp=NULL;
	png_ptr=NULL;
	info_ptr=NULL;
	//end_info=NULL;
	rstate=RS_None;
	curr_error=NULL;
	interlacing_passes=-1;
}

PNGReader::~PNGReader()
{
	Close();
}

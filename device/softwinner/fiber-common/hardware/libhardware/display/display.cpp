/*
 * Copyright (C) 2008 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */


#define LOG_TAG "display"

#include <cutils/log.h>

#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <stdlib.h>

#ifdef HAVE_ANDROID_OS      // just want PAGE_SIZE define
#include <asm/page.h>
#else
#include <sys/user.h>
#endif
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/mman.h>

#include <hardware/display.h>
#include <drv_display.h>
#include <g2d_driver.h>
#include <fb.h>
#include <sunxi_physmem.h>
#include <ui/DisplayCommand.h>

#ifndef LOGV
#define LOGV ALOGV
#endif

#ifndef LOGE
#define LOGE ALOGE
#endif

#ifndef LOGD
#define LOGD ALOGD
#endif

#define MAX_DISPLAY_NUM		2
#define MAX_CURSOR_SIZE     128
#define MAX_CURSOR_MEMIDX   10
static int  display_getwidth(struct display_context_t* ctx,int displayno,int type, int format);
static int  display_getheight(struct display_context_t* ctx,int displayno,int type, int format);


pthread_mutex_t             mode_lock;
bool                        mutex_inited = false;

struct display_context_t 
{
    struct display_device_t     device;
    int                         mFD_fb[MAX_DISPLAY_NUM];
    int                         mFD_cursor;
    int	                        mFD_disp;
    int                         mFD_mp;
    int                         mFD_mem;
    int                         mode;
    int                         pixel_format[2];
    int                         out_type[2];
    int                         out_format[2];
    int                         width[2];//screen total width
    int                         height[2];//screen total height
    int                         valid_width[2];//screen width that can be seen
    int                         valid_height[2];//screen height that can be seen
    int                         app_width[2];//the width that app use
    int                         app_height[2];//the height that app use
    int                         lcd_width;
    int                         lcd_height;
    int                         area_percent[2];
    int                         orientation;
    int                         layer_hdl[2];
    unsigned int                fb1_mem_start;
    bool                        isenable_mp;
};

struct tv_para_t
{
    int type;// bit3:cvbs, bit2:ypbpr, bit1:vga, bit0:hdmi
    int mode;
    int width;
    int height;
    int valid_width;
    int valid_height;
    int driver_mode;
};

static struct tv_para_t g_tv_para[]=
{
    {8, DISPLAY_TVFORMAT_NTSC,             720,    480,    690,    450,    DISP_TV_MOD_NTSC},
    {8, DISPLAY_TVFORMAT_PAL,              720,    576,    690,    546,    DISP_TV_MOD_PAL},
    {8, DISPLAY_TVFORMAT_PAL_M,            720,    576,    690,    546,    DISP_TV_MOD_PAL_M},
    {8, DISPLAY_TVFORMAT_PAL_NC,           720,    576,    690,    546,    DISP_TV_MOD_PAL_NC},
    
    {5, DISPLAY_TVFORMAT_480I,             720,    480,    690,    450,    DISP_TV_MOD_480I},
    {5, DISPLAY_TVFORMAT_576I,             720,    576,    690,    546,    DISP_TV_MOD_576I},
    {5, DISPLAY_TVFORMAT_480P,             720,    480,    690,    450,    DISP_TV_MOD_480P},
    {5, DISPLAY_TVFORMAT_576P,             720,    576,    690,    546,    DISP_TV_MOD_576P},
    {5, DISPLAY_TVFORMAT_720P_50HZ,        1280,   720,    1220,   690,    DISP_TV_MOD_720P_50HZ},
    {5, DISPLAY_TVFORMAT_720P_60HZ,        1280,   720,    1220,   690,    DISP_TV_MOD_720P_60HZ},
    {5, DISPLAY_TVFORMAT_1080I_50HZ,       1920,   1080,   1880,   1040,   DISP_TV_MOD_1080I_50HZ},
    {5, DISPLAY_TVFORMAT_1080I_60HZ,       1920,   1080,   1880,   1040,   DISP_TV_MOD_1080I_60HZ},
    {1, DISPLAY_TVFORMAT_1080P_24HZ,       1920,   1080,   1880,   1040,   DISP_TV_MOD_1080P_24HZ},
    {5, DISPLAY_TVFORMAT_1080P_50HZ,       1920,   1080,   1880,   1040,   DISP_TV_MOD_1080P_50HZ},
    {5, DISPLAY_TVFORMAT_1080P_60HZ,       1920,   1080,   1880,   1040,   DISP_TV_MOD_1080P_60HZ},
    {1, DISPLAY_TVFORMAT_1080P_25HZ,       1920,   1080,   1880,   1040,   DISP_TV_MOD_1080P_25HZ},
    {1, DISPLAY_TVFORMAT_1080P_30HZ,       1920,   1080,   1880,   1040,   DISP_TV_MOD_1080P_30HZ},
    
    {1, DISPLAY_TVFORMAT_1080P_24HZ_3D_FP, 1920,   1080,   1920,   1080,   DISP_TV_MOD_1080P_24HZ_3D_FP},
    {1, DISPLAY_TVFORMAT_720P_50HZ_3D_FP,  1280,   720,    1280,   720,    DISP_TV_MOD_720P_50HZ_3D_FP},
    {1, DISPLAY_TVFORMAT_720P_60HZ_3D_FP,  1280,   720,    1280,   720,    DISP_TV_MOD_720P_60HZ_3D_FP},
    
    {2, DISPLAY_VGA_H1680_V1050,           1668,   1050,   1668,   1050,   DISP_VGA_H1680_V1050},
    {2, DISPLAY_VGA_H1440_V900,            1440,   900,    1440,   900,    DISP_VGA_H1440_V900},
    {2, DISPLAY_VGA_H1360_V768,            1360,   768,    1360,   768,    DISP_VGA_H1360_V768},
    {2, DISPLAY_VGA_H1280_V1024,           1280,   1024,   1280,   1024,   DISP_VGA_H1280_V1024},
    {2, DISPLAY_VGA_H1024_V768,            1024,   768,    1204,   768,    DISP_VGA_H1024_V768},
    {2, DISPLAY_VGA_H800_V600,             800,    600,    800,    600,    DISP_VGA_H800_V600},
    {2, DISPLAY_VGA_H640_V480,             640,    480,    640,    480,    DISP_VGA_H640_V480},
    {2, DISPLAY_VGA_H1440_V900_RB,         1440,   900,    1440,   900,    DISP_VGA_H1440_V900},
    {2, DISPLAY_VGA_H1920_V1080,           1920,   1080,   1920,   1080,   DISP_VGA_H1920_V1080},
    {2, DISPLAY_VGA_H1280_V720,            1280,   720,    1280,   720,    DISP_VGA_H1280_V720},
};

static int open_display(const struct hw_module_t* module, const char* name,
        struct hw_device_t** device);

static struct hw_module_methods_t display_module_methods = 
{
    open:  open_display
};

struct display_module_t HAL_MODULE_INFO_SYM = 
{
    common: 
    {
        tag: HARDWARE_MODULE_TAG,
        version_major: 1,
        version_minor: 0,
        id: DISPLAY_HARDWARE_MODULE_ID,
        name: "Crane DISPLAY Module",
        author: "Google, Inc.",
        methods: &display_module_methods
    }
};

static int display_requestbuf(struct display_device_t *dev,int width,int height,int format,int buf_num)
{
    struct 	display_context_t*  ctx = (struct display_context_t*)dev;
    unsigned int addr;
    int size;

    if(format == DISPLAY_FORMAT_PYUV420UVC)
    {
        size = (width * height * 3) / 2;
    }
    else if(format == DISPLAY_FORMAT_ARGB8888)
    {
        size = (width *  height * 4);
    }
	else
	{
		LOGE("####unsupported format:%d in display_requestbuf\n",format);
		return -1;
	}

    size = size * buf_num;
	addr = ioctl(ctx->mFD_mem, SUNXI_MEM_ALLOC, &size);
	if(addr == 0)
	{
	    LOGE("alloc mem fail in %s", __FUNCTION__);
	}

    return addr;
}

static int display_releasebuf(struct display_device_t *dev,int buf_hdl)
{
    struct 	display_context_t*  ctx = (struct display_context_t*)dev;
    int ret;

    ret = ioctl(ctx->mFD_mem, SUNXI_MEM_FREE, buf_hdl);
    if(ret < 0)
    {
	    LOGE("free mem fail in %s", __FUNCTION__);
	}
    
    return ret;
}

static int display_convertfb(struct display_device_t *dev,int srcfb_id,int srcfb_bufno,int dst_buf_hdl, int dst_bufno,int bufwidth,int bufheight,int bufformat)
{
    struct 	display_context_t*  ctx = (struct display_context_t*)dev;
    
	struct fb_fix_screeninfo    fix_src;
    struct fb_fix_screeninfo    fix_dst;
    struct fb_var_screeninfo    var_src;
    struct fb_var_screeninfo    var_dst;
    unsigned int                src_width;
    unsigned int                src_height;
    unsigned int                dst_width;
    unsigned int                dst_height;
    unsigned int                addr_src;
    unsigned int                addr_dst_y,addr_dst_c;
    g2d_data_fmt                dst_fmt;
    g2d_pixel_seq               dst_seq;
    g2d_stretchblt              blit_para;
    int                         err;
    unsigned long               args[4];
    
	ioctl(ctx->mFD_fb[srcfb_id],FBIOGET_FSCREENINFO,&fix_src);
	ioctl(ctx->mFD_fb[srcfb_id],FBIOGET_VSCREENINFO,&var_src);
	addr_src = fix_src.smem_start + ((var_src.xres * (srcfb_bufno * var_src.yres) * var_src.bits_per_pixel) >> 3);
	src_width   = var_src.xres;
	src_height  = var_src.yres;
		
    addr_dst_y = dst_buf_hdl;
	if(bufformat == DISPLAY_FORMAT_PYUV420UVC)
    {
        addr_dst_y = addr_dst_y + ((bufwidth * bufheight * 3) / 2) * dst_bufno;
        addr_dst_c = addr_dst_y + (bufwidth * bufheight);
        dst_fmt = G2D_FMT_PYUV420UVC;
        dst_seq = G2D_SEQ_NORMAL;
    }
    else if(bufformat == DISPLAY_FORMAT_ARGB8888)
    {
        addr_dst_y = addr_dst_y + (bufwidth *  bufheight * 4) * dst_bufno;
        addr_dst_c = 0;
        dst_fmt = G2D_FMT_ARGB_AYUV8888;
        dst_seq = G2D_SEQ_NORMAL;
    }
    else
	{
		LOGE("####unsupported format:%d in display_convertfb\n",bufformat);
		return -1;
	}
    
    dst_width   = bufwidth;
    dst_height  = bufheight;

    blit_para.src_image.addr[0]     = addr_src;
    blit_para.src_image.addr[1]     = 0;
    blit_para.src_image.addr[2]     = 0;
    blit_para.src_image.format      = G2D_FMT_ARGB_AYUV8888;
    blit_para.src_image.h           = src_height;
    blit_para.src_image.w           = src_width;
    blit_para.src_image.pixel_seq   = G2D_SEQ_NORMAL;

    blit_para.src_rect.x            = 0;
    blit_para.src_rect.y            = 0;
    blit_para.src_rect.w            = src_width;
    blit_para.src_rect.h            = src_height;

    blit_para.dst_image.addr[0]     = addr_dst_y;
    blit_para.dst_image.addr[1]     = addr_dst_c;
    blit_para.dst_image.addr[2]     = 0;
    blit_para.dst_image.format      = dst_fmt;
    blit_para.dst_image.h           = dst_height;
    blit_para.dst_image.w           = dst_width;
    blit_para.dst_image.pixel_seq   = dst_seq;

    blit_para.dst_rect.x            = 0;
    blit_para.dst_rect.y            = 0;
    blit_para.dst_rect.w            = dst_width;
    blit_para.dst_rect.h            = dst_height;
    
    blit_para.flag                 = G2D_BLT_NONE;
    
    err = ioctl(ctx->mFD_mp , G2D_CMD_STRETCHBLT ,(unsigned long)&blit_para);				
    if(err < 0)		
    {    
        LOGE("copy fb failed!\n");
        
        return  -1;      
    }

    return  0;
}

static int display_getdispbufaddr(struct display_device_t *dev,int buf_hdl,int bufno,int bufwidth,int bufheight,int bufformat)
{
	struct 	display_context_t*  ctx = (struct display_context_t*)dev;
    int	addr=0;
	
    addr = buf_hdl;
    
    if(bufformat == DISPLAY_FORMAT_PYUV420UVC)
    {
        addr = addr + ((bufwidth * bufheight * 3)/2) * bufno;
    }
    else if(bufformat == DISPLAY_FORMAT_ARGB8888)
    {
        addr = addr + (bufwidth *  bufheight * 4) * bufno;
    }
    
    return addr;
}

static int display_isenablemp(struct display_device_t *dev)
{
    struct 	display_context_t*  ctx = (struct display_context_t*)dev;

    return ctx->isenable_mp;
}

static int display_copyfb(struct display_device_t *dev,int srcfb_id,int srcfb_bufno,
                          int dstfb_id,int dstfb_bufno)
{
    struct 	display_context_t*  ctx = (struct display_context_t*)dev;
    
	struct fb_fix_screeninfo    fix_src;
    struct fb_fix_screeninfo    fix_dst;
    struct fb_var_screeninfo    var_src;
    struct fb_var_screeninfo    var_dst;
    unsigned int                src_width;
    unsigned int                src_height;
    unsigned int                dst_width;
    unsigned int                dst_height;
    unsigned int                addr_src;
    unsigned int                addr_dst;
    unsigned int                size;
//    g2d_stretchblt              blit_para;
    g2d_blt                     blit_para;
    int                         err;

	ioctl(ctx->mFD_fb[srcfb_id],FBIOGET_FSCREENINFO,&fix_src);
	ioctl(ctx->mFD_fb[srcfb_id],FBIOGET_VSCREENINFO,&var_src);
	ioctl(ctx->mFD_fb[dstfb_id],FBIOGET_FSCREENINFO,&fix_dst);
	ioctl(ctx->mFD_fb[dstfb_id],FBIOGET_VSCREENINFO,&var_dst);
	
	src_width   = var_src.xres;
	src_height  = var_src.yres;
    dst_width   = var_dst.xres;
    dst_height  = var_dst.yres;

	addr_src = fix_src.smem_start + ((var_src.xres * (srcfb_bufno * var_src.yres) * var_src.bits_per_pixel) >> 3);
	addr_dst = fix_dst.smem_start + ((var_dst.xres * (dstfb_bufno * var_dst.yres) * var_dst.bits_per_pixel) >> 3);
	size = (var_src.xres * var_src.yres * var_src.bits_per_pixel) >> 3;//in byte unit

	switch (var_src.bits_per_pixel) 
	{			
    	case 16:
    		blit_para.src_image.format      = G2D_FMT_RGB565;
    		break;
    		
    	case 24:
    		blit_para.src_image.format      = G2D_FMT_RGBA_YUVA8888;
    		break;
    		
    	case 32:
    		blit_para.src_image.format      = G2D_FMT_RGBA_YUVA8888;
    		break;
    		
    	default:
    	    LOGE("invalid bits_per_pixel :%d\n", var_src.bits_per_pixel);
    		return -1;
	}

    blit_para.src_image.addr[0]     = addr_src;
    blit_para.src_image.addr[1]     = 0;
    blit_para.src_image.addr[2]     = 0;
    blit_para.src_image.format      = G2D_FMT_ARGB_AYUV8888;
    blit_para.src_image.h           = src_height;
    blit_para.src_image.w           = src_width;
    blit_para.src_image.pixel_seq   = G2D_SEQ_NORMAL;

    blit_para.dst_image.addr[0]     = addr_dst;
    blit_para.dst_image.addr[1]     = 0;
    blit_para.dst_image.addr[2]     = 0;
    blit_para.dst_image.format      = G2D_FMT_ARGB_AYUV8888;
    blit_para.dst_image.h           = dst_height;
    blit_para.dst_image.w           = dst_width;
    blit_para.dst_image.pixel_seq   = G2D_SEQ_NORMAL;

//    blit_para.dst_rect.x            = 0;
//    blit_para.dst_rect.y            = 0;
//    blit_para.dst_rect.w            = ctx->valid_width[dstfb_id];
//    blit_para.dst_rect.h            = ctx->valid_height[dstfb_id];
    blit_para.dst_x = 0;
    blit_para.dst_y = 0;

    blit_para.src_rect.x            = 0;
    blit_para.src_rect.y            = 0;
    blit_para.src_rect.w            = src_width;
    blit_para.src_rect.h            = src_height;
/*
    if(ctx->orientation == 0)
    {
        blit_para.flag                 = G2D_BLT_NONE;
    }
    else if(ctx->orientation == 1)
    {
        blit_para.flag                 = G2D_BLT_ROTATE270;
    }
    else if(ctx->orientation == 2)
    {
        blit_para.flag                 = G2D_BLT_ROTATE180;
    }
    else if(ctx->orientation == 3)
    {
        blit_para.flag                 = G2D_BLT_ROTATE90;
    }
  */
    __disp_rect_t src_rect;
    unsigned long args[4];


	args[0] = 1;
	  args[1] = ctx->layer_hdl[1];
	  args[2] = (unsigned long)&src_rect;
	  ioctl(ctx->mFD_disp,DISP_CMD_LAYER_GET_SRC_WINDOW,args);

	  


	ctx->app_width[1] = display_getwidth(ctx, 1, ctx->out_type[1], ctx->out_format[1]);
		  ctx->app_height[1] = display_getheight(ctx, 1, ctx->out_type[1], ctx->out_format[1]);


	int app_width = ctx->app_width[1];	 
	int app_height = ctx->app_height[1];

	LOGD("app_width=%d, app_height=%d\n", app_width, app_height);
	LOGD("src_rect.height=%d, src_rect.width=%d\n", src_rect.height, src_rect.width);

//if(app_width>app_height)

if(src_rect.height>src_rect.width)
	blit_para.flag                 = G2D_BLT_ROTATE180;
else
  	blit_para.flag                 = G2D_BLT_ROTATE270;
       err = ioctl(ctx->mFD_mp , G2D_CMD_BITBLT,(unsigned long)&blit_para);
   LOGV("==========mp========");
 //err = ioctl(ctx->mFD_mp , G2D_CMD_STRETCHBLT ,(unsigned long)&blit_para);
    if(err < 0)
    {    
        LOGE("copy fb failed!\n");
        
        return  -1;
    }

    return  0;
}
      
static int display_pandisplay(struct display_device_t *dev,int screen_id,int bufno)
{
    struct 	display_context_t*  ctx = (struct display_context_t*)dev;
    struct fb_var_screeninfo    var;
    __disp_rect_t src_rect;
    unsigned long args[4];
    __disp_init_t init_para;

    args[0] = (unsigned long)&init_para;
    ioctl(ctx->mFD_disp,DISP_CMD_GET_DISP_INIT_PARA,(unsigned long)args);

    args[0] = screen_id;
    args[1] = ctx->layer_hdl[screen_id];
    args[2] = (unsigned long)&src_rect;
	ioctl(ctx->mFD_disp,DISP_CMD_LAYER_GET_SRC_WINDOW,args);

    if(display_isenablemp(dev))
    {
    	ioctl(ctx->mFD_fb[screen_id],FBIOGET_VSCREENINFO,&var);
    	src_rect.y = bufno * var.yres_virtual / DISPLAYDISPATCH_MAXBUFNO;
    }
    else
    {
    	ioctl(ctx->mFD_fb[0],FBIOGET_VSCREENINFO,&var);
    	src_rect.y = bufno * var.yres_virtual / init_para.buffer_num[0];
    }

    args[0] = screen_id;
    args[1] = ctx->layer_hdl[screen_id];
    args[2] = (unsigned long)&src_rect;
	ioctl(ctx->mFD_disp,DISP_CMD_LAYER_SET_SRC_WINDOW,args);
	
    return 0;
}

static int display_gethdmistatus(struct display_device_t *dev)
{
    struct display_context_t* ctx = (struct display_context_t*)dev;
	unsigned long args[4];
	int ret;
	
	args[0] = 0;
	
    ret = ioctl(ctx->mFD_disp,DISP_CMD_HDMI_GET_HPD_STATUS,args);
    return ret;

    return 0;    
}

static int display_gethdmimaxmode(struct display_device_t *dev)
{
    struct display_context_t* ctx = (struct display_context_t*)dev;
	unsigned long args[4];
	
	return DISPLAY_TVFORMAT_720P_50HZ;

	if(ctx->app_width[0] <= 1920)
	{
		args[0] = 0;
		args[1] = DISP_TV_MOD_1080P_60HZ;
		if(ioctl(ctx->mFD_disp,DISP_CMD_HDMI_SUPPORT_MODE,args))
		{
			return DISPLAY_TVFORMAT_1080P_60HZ;
		}
		
		args[1] = DISP_TV_MOD_1080P_50HZ;
		if(ioctl(ctx->mFD_disp,DISP_CMD_HDMI_SUPPORT_MODE,args))
		{
			return DISPLAY_TVFORMAT_1080P_50HZ;
		}
		
		args[1] = DISPLAY_TVFORMAT_720P_60HZ;
		if(ioctl(ctx->mFD_disp,DISP_CMD_HDMI_SUPPORT_MODE,args))
		{
			return DISPLAY_TVFORMAT_720P_60HZ;
		}
		
		args[1] = DISP_TV_MOD_720P_50HZ;
		if(ioctl(ctx->mFD_disp,DISP_CMD_HDMI_SUPPORT_MODE,args))
		{
			return DISPLAY_TVFORMAT_720P_50HZ;
		}
		
		args[1] = DISP_TV_MOD_576P;
		if(ioctl(ctx->mFD_disp,DISP_CMD_HDMI_SUPPORT_MODE,args))
		{
			return DISPLAY_TVFORMAT_576P;
		}
		
		args[1] = DISP_TV_MOD_480P;
		if(ioctl(ctx->mFD_disp,DISP_CMD_HDMI_SUPPORT_MODE,args))
		{
			return DISPLAY_TVFORMAT_480P;
		}
	}
	else
	{
        args[0] = 0;
		args[1] = DISP_TV_MOD_1080P_50HZ;
		if(ioctl(ctx->mFD_disp,DISP_CMD_HDMI_SUPPORT_MODE,args))
		{
			return DISPLAY_TVFORMAT_1080P_50HZ;
		}

		args[1] = DISP_TV_MOD_720P_50HZ;
		if(ioctl(ctx->mFD_disp,DISP_CMD_HDMI_SUPPORT_MODE,args))
		{
			return DISPLAY_TVFORMAT_720P_50HZ;
		}

		args[1] = DISP_TV_MOD_1080P_60HZ;
		if(ioctl(ctx->mFD_disp,DISP_CMD_HDMI_SUPPORT_MODE,args))
		{
			return DISPLAY_TVFORMAT_1080P_60HZ;
		}
		
		args[1] = DISPLAY_TVFORMAT_720P_60HZ;
		if(ioctl(ctx->mFD_disp,DISP_CMD_HDMI_SUPPORT_MODE,args))
		{
			return DISPLAY_TVFORMAT_720P_60HZ;
		}
		
		args[1] = DISP_TV_MOD_576P;
		if(ioctl(ctx->mFD_disp,DISP_CMD_HDMI_SUPPORT_MODE,args))
		{
			return DISPLAY_TVFORMAT_576P;
		}
		
		args[1] = DISP_TV_MOD_480P;
		if(ioctl(ctx->mFD_disp,DISP_CMD_HDMI_SUPPORT_MODE,args))
		{
			return DISPLAY_TVFORMAT_480P;
		}
	}

    return DISPLAY_TVFORMAT_720P_60HZ;    
}      

int display_enablebacklight(display_device_t *dev,int displayno,int enable)
{
    struct display_context_t* ctx = (struct display_context_t*)dev;
    unsigned long args[4];
	args[0] = (long)displayno;
    if(enable == 1)
    {
        return  ioctl(ctx->mFD_disp, DISP_CMD_LCD_BACKLIGHT_ON,args);
    }
    else
    {	
        return ioctl(ctx->mFD_disp, DISP_CMD_LCD_BACKLIGHT_OFF,args);
    }
    return 0;    
}

static int display_setbacklightmode(struct display_device_t *dev,int mode)
{
    struct display_context_t* ctx = (struct display_context_t*)dev;
	unsigned long args[4];
	args[0] = 0;
	
	if(mode == 1)
	{
		return  ioctl(ctx->mFD_disp,DISP_CMD_DRC_ON,args);
	}
	else
	{	
		return ioctl(ctx->mFD_disp,DISP_CMD_DRC_OFF,args);
	}

    return 0;    
}

static int display_setbacklightdemomode(struct display_device_t *dev,int mode)
{
   struct display_context_t* ctx = (struct display_context_t*)dev;
   unsigned long args[4];
   args[0] = 0;
   __disp_rect_t window;
   window.x = 0;
   window.y = 0;
   window.width = ioctl(ctx->mFD_disp,DISP_CMD_SCN_GET_WIDTH,args);
   window.height = ioctl(ctx->mFD_disp,DISP_CMD_SCN_GET_HEIGHT,args);
   if(mode == 1)
	{
	   window.width /= 2;
	   args[1] = (unsigned long)&window;
	   return ioctl(ctx->mFD_disp, DISP_CMD_DRC_SET_WINDOW,args);
		
	}
	else
	{	
	   args[1] = (unsigned long)&window;
	   return ioctl(ctx->mFD_disp, DISP_CMD_DRC_SET_WINDOW,args);
		
	}

   return 0;

}

static int display_setenhancemode(struct display_device_t *dev,int mode,int chanel)
{
    struct display_context_t* ctx = (struct display_context_t*)dev;
    if(ctx)
    {
        if(ctx->mFD_disp)
        {
        	unsigned long args[4];
        	
        	args[0] = chanel;
			if(mode == 1)
			{
		
        		return  ioctl(ctx->mFD_disp,DISP_CMD_ENHANCE_ON ,args);

			}
			else
			{	
				return ioctl(ctx->mFD_disp,DISP_CMD_ENHANCE_OFF,args);
			}
        }
    }

    return 0;    
}

static int display_getenhanceenable(struct display_device_t *dev, int chanel)
{
	int ret = 0;
	struct display_context_t* ctx = (struct display_context_t*)dev;
	if(ctx)
	{
		if(ctx->mFD_disp)
		{
			unsigned long args[4];
			LOGD("display_getenhanceenable##########");
			args[0] = chanel;
			ret = ioctl(ctx->mFD_disp,DISP_CMD_GET_ENHANCE_EN,args);
		}
	}

	return ret;
}

static int display_setenhancedemomode(struct display_device_t *dev,int mode,int chanel)
{
    struct display_context_t* ctx = (struct display_context_t*)dev;
    if(ctx)
    {
        if(ctx->mFD_disp)
        {   unsigned long args[4];	
		    __disp_rect_t window;
			
		    args[0] = chanel;
			window.x = 0;
			window.y = 0;
			window.width = ioctl(ctx->mFD_disp,DISP_CMD_SCN_GET_WIDTH,args);
			window.height = ioctl(ctx->mFD_disp,DISP_CMD_SCN_GET_HEIGHT,args);
            ALOGE("DISP_CMD_SCN_GET_WIDTH=%d",window.width);
			ALOGE("DISP_CMD_SCN_GET_HEIGHT=%d",window.height);
            if(mode ==1)
			{
			   window.width /= 2;
			   args[1] = (unsigned long)&window;
			   return ioctl(ctx->mFD_disp,DISP_CMD_SET_ENHANCE_WINDOW,args);
			    
			}else
			{
			   args[1] = (unsigned long)&window;
			   return ioctl(ctx->mFD_disp,DISP_CMD_SET_ENHANCE_WINDOW,args);
			}
        }
    }
	return 0;

}


static int display_gettvdacstatus(struct display_device_t *dev)
{
    struct display_context_t* ctx = (struct display_context_t*)dev;
    int                       status;
    int ret = DISPLAY_TVDAC_NONE;
	unsigned long args[4];
	
	args[0] = 0;
    status = ioctl(ctx->mFD_disp,DISP_CMD_TV_GET_INTERFACE,args);

    if((status & DISP_TV_CVBS) && (status & DISP_TV_YPBPR))
    {
        ret = DISPLAY_TVDAC_ALL;
    }
    else if(status & DISP_TV_YPBPR)
    {
        ret =  DISPLAY_TVDAC_YPBPR;
    }
    else if(status & DISP_TV_CVBS)
    {
        ret =  DISPLAY_TVDAC_CVBS;
    }

    return ret;
}

static int  display_getwidth(struct display_context_t* ctx,int displayno,int type, int format)
{
    unsigned int i = 0;
    
    if(type == DISPLAY_DEVICE_TV || type == DISPLAY_DEVICE_HDMI || type == DISPLAY_DEVICE_VGA)
    {
        for(i=0; i<sizeof(g_tv_para)/sizeof(struct tv_para_t); i++)
        {
            if(g_tv_para[i].mode == format)
            {
                return g_tv_para[i].width;
            }
        }
    }
    else
    {
        return ctx->lcd_width;
    }

    return -1;
}

static int  display_getheight(struct display_context_t* ctx,int displayno,int type, int format)
{
    unsigned int i = 0;
    
    if(type == DISPLAY_DEVICE_TV || type == DISPLAY_DEVICE_HDMI || type == DISPLAY_DEVICE_VGA)
    {
        for(i=0; i<sizeof(g_tv_para)/sizeof(struct tv_para_t); i++)
        {
            if(g_tv_para[i].mode == format)
            {
                return g_tv_para[i].height;
            }
        }
    }
    else
    {
        return ctx->lcd_height;
    }
    
    return -1;
}

static int  display_getvalidwidth(struct display_context_t* ctx,int displayno,int type, int format)
{
    unsigned int i = 0;
    
    if(type == DISPLAY_DEVICE_TV || type == DISPLAY_DEVICE_HDMI || type == DISPLAY_DEVICE_VGA)
    {
        for(i=0; i<sizeof(g_tv_para)/sizeof(struct tv_para_t); i++)
        {
            if(g_tv_para[i].mode == format)
            {
                return g_tv_para[i].valid_width;
            }
        }
    }
    else
    {
        return ctx->lcd_width;
    }
    
    return -1;
} 

static int  display_getvalidheight(struct display_context_t* ctx,int displayno,int type, int format)
{
    unsigned int i = 0;
    
    if(type == DISPLAY_DEVICE_TV || type == DISPLAY_DEVICE_HDMI || type == DISPLAY_DEVICE_VGA)
    {
        for(i=0; i<sizeof(g_tv_para)/sizeof(struct tv_para_t); i++)
        {
            if(g_tv_para[i].mode == format)
            {
                return g_tv_para[i].valid_height;
            }
        }
    }
    else
    {
        return ctx->lcd_height;
    }
      
    return -1;
}

static int display_get_driver_tv_format(int format)
{
    unsigned int i = 0;
    
    for(i=0; i<sizeof(g_tv_para)/sizeof(struct tv_para_t); i++)
    {
        if(g_tv_para[i].mode == format)
        {
            return g_tv_para[i].driver_mode;
        }
    }

    return -1;
}

static int display_requestmodelock(struct display_device_t *dev)
{
    pthread_mutex_lock(&mode_lock);
    
    return 0;
}
      
static int  display_releasemodelock(struct display_device_t *dev)
{
    pthread_mutex_unlock(&mode_lock);
    
    return 0;
}

static int display_setmasterdisplay(struct display_device_t *dev,int master)
{
    return  0;
}

static int display_getmasterdisplay(struct display_device_t *dev)
{
    return  0;
}


static int display_getdisplaycount(struct display_device_t *dev)
{
	return MAX_DISPLAY_NUM;
}

//todo
static int display_getdisplaybufid(struct display_device_t *dev, int fbno)
{
    struct display_context_t*   ctx = (struct display_context_t*)dev;
    struct fb_var_screeninfo    var_src;

    ioctl(ctx->mFD_fb[fbno],FBIOGET_VSCREENINFO,&var_src);

    return  var_src.yoffset/var_src.yres;
}

static int display_getmaxdisplayno(struct display_device_t *dev)
{
    return 0;
}

static int display_setbright(struct display_device_t *dev,int displayno,int bright)
{
	struct display_context_t* 	ctx = (struct display_context_t*)dev;
    unsigned long 				args[4];
     
    if(displayno > 1)
    {
    	displayno = 1;
    }
    
    if(displayno < 0)
    {
    	displayno = 0;
    }
    
    if(bright > 100)
    {
    	bright = 100;
    }
    
    if(bright < 0)
    {
    	bright = 0;
    }
   
	args[0] = displayno;
	args[1] = 0;
	
	ioctl(ctx->mFD_disp,DISP_CMD_ENHANCE_ON,args);	
	
	args[0] = displayno;
	args[1] = bright;
	
	ioctl(ctx->mFD_disp,DISP_CMD_SET_BRIGHT,args);	
    
    return  0;
}

static int display_getbright(struct display_device_t *dev,int displayno)
{
	struct display_context_t* 	ctx = (struct display_context_t*)dev;
    unsigned long 				args[4];
     
    if(displayno > 1)
    {
    	displayno = 1;
    }
    
    if(displayno < 0)
    {
    	displayno = 0;
    }
    
	args[0] = displayno;
	args[1] = 0;
	
	return ioctl(ctx->mFD_disp,DISP_CMD_GET_BRIGHT,args);	
    
    return  0;
}

static int display_setcontrast(struct display_device_t *dev,int displayno,int contrast)
{
	struct display_context_t* 	ctx = (struct display_context_t*)dev;
    unsigned long 				args[4];
     
    if(displayno > 1)
    {
    	displayno = 1;
    }
    
    if(displayno < 0)
    {
    	displayno = 0;
    }
    
    if(contrast > 100)
    {
    	contrast = 100;
    }
    
    if(contrast < 0)
    {
    	contrast = 0;
    }

	args[0] = displayno;
	args[1] = 0;
	
	ioctl(ctx->mFD_disp,DISP_CMD_ENHANCE_ON,args);	
	
	args[0] = displayno;
	args[1] = contrast;
	
	ioctl(ctx->mFD_disp,DISP_CMD_SET_CONTRAST,args);	
    
    return  0;
}

static int display_getcontrast(struct display_device_t *dev,int displayno)
{
	struct display_context_t* 	ctx = (struct display_context_t*)dev;
    unsigned long 				args[4];
     
    if(displayno > 1)
    {
    	displayno = 1;
    }
    
    if(displayno < 0)
    {
    	displayno = 0;
    }
    
	args[0] = displayno;
	args[1] = 0;
	
	return ioctl(ctx->mFD_disp,DISP_CMD_GET_CONTRAST,args);	
    
    return  0;
}

static int display_setsaturation(struct display_device_t *dev,int displayno,int saturation)
{
	struct display_context_t* 	ctx = (struct display_context_t*)dev;
    unsigned long 				args[4];
     
    if(displayno > 1)
    {
    	displayno = 1;
    }
    
    if(displayno < 0)
    {
    	displayno = 0;
    }
    
    if(saturation > 100)
    {
    	saturation = 100;
    }
    
    if(saturation < 0)
    {
    	saturation = 0;
    }

	args[0] = displayno;
	args[1] = 0;
	
	ioctl(ctx->mFD_disp,DISP_CMD_ENHANCE_ON,args);	
	
	args[0] = displayno;
	args[1] = saturation;
	
	ioctl(ctx->mFD_disp,DISP_CMD_SET_SATURATION,args);	
    
    return  0;
}

static int display_getsaturation(struct display_device_t *dev,int displayno)
{
	struct display_context_t* 	ctx = (struct display_context_t*)dev;
    unsigned long 				args[4];
     
    if(displayno > 1)
    {
    	displayno = 1;
    }
    
    if(displayno < 0)
    {
    	displayno = 0;
    }

	args[0] = displayno;
	args[1] = 0;
	
	ioctl(ctx->mFD_disp,DISP_CMD_GET_SATURATION,args);	
    
    return  0;
}

static int display_sethue(struct display_device_t *dev,int displayno,int hue)
{
	struct display_context_t* 	ctx = (struct display_context_t*)dev;
    unsigned long 				args[4];
     
    if(displayno > 1)
    {
    	displayno = 1;
    }
    
    if(displayno < 0)
    {
    	displayno = 0;
    }
    
    if(hue > 100)
    {
    	hue = 100;
    }
    
    if(hue < 0)
    {
    	hue = 0;
    }

	args[0] = displayno;
	args[1] = 0;
	
	ioctl(ctx->mFD_disp,DISP_CMD_ENHANCE_ON,args);	
	
	args[0] = displayno;
	args[1] = hue;
	
	ioctl(ctx->mFD_disp,DISP_CMD_SET_HUE,args);	
    
    return  0;
}

static int display_gethue(struct display_device_t *dev,int displayno)
{
	struct display_context_t* 	ctx = (struct display_context_t*)dev;
    unsigned long 				args[4];
     
    if(displayno > 1)
    {
    	displayno = 1;
    }
    
    if(displayno < 0)
    {
    	displayno = 0;
    }

	args[0] = displayno;
	args[1] = 0;
	
	ioctl(ctx->mFD_disp,DISP_CMD_GET_HUE,args);	
    
    return  0;
}

static int display_issupport3dmode(struct display_device_t *dev)
{    
    struct display_context_t* ctx = (struct display_context_t*)dev;
	unsigned long args[4];
	
	args[0] = 0;
	args[1] = DISP_TV_MOD_1080P_24HZ_3D_FP;

    if(ioctl(ctx->mFD_disp,DISP_CMD_HDMI_SUPPORT_MODE,args))
    {
        return 1;
    }
            
    return  0;   
}    

int display_issupporthdmimode(struct display_device_t *dev,int mode)
{    
    struct display_context_t* ctx = (struct display_context_t*)dev;
    unsigned long args[4];
    
    args[0] = 0;
    args[1] = display_get_driver_tv_format(mode);
    if(ioctl(ctx->mFD_disp,DISP_CMD_HDMI_SUPPORT_MODE,args))
    {
        return 1;
    }

    return  0;   
}


static int display_setareapercent(struct display_device_t *dev,int displayno,int percent)
{
    struct 	display_context_t*  ctx = (struct display_context_t*)dev;
    int							ret = 0;
    unsigned long 				arg[4];
    unsigned int layer_hdl;
    __disp_rect_t scn_win;
    int i = 0;

    LOGD("####display_setareapercent:screen:%d,percent:%d\n", displayno,percent);

    if(ctx->mode == DISPLAY_MODE_DUALSAME || ctx->mode == DISPLAY_MODE_DUALSAME_TWO_VIDEO)
    {
        if(displayno == 0)
        {
            ioctl(ctx->mFD_fb[0], FBIOGET_LAYER_HDL_0, &layer_hdl);
        }
        else
        {
            ioctl(ctx->mFD_fb[1], FBIOGET_LAYER_HDL_1, &layer_hdl);
        }
    }
    else if(ctx->mode == DISPLAY_MODE_SINGLE || ctx->mode == DISPLAY_MODE_SINGLE_VAR_FE || 
            ctx->mode == DISPLAY_MODE_SINGLE_FB_VAR || ctx->mode == DISPLAY_MODE_SINGLE_VAR_GPU)
    {
        if(displayno == 0)
        {
            ioctl(ctx->mFD_fb[0], FBIOGET_LAYER_HDL_0, &layer_hdl);
        }
        else
        {
            return 0;
        }
    }    
    else if(ctx->mode == DISPLAY_MODE_SINGLE_VAR_BE)
    {
        if(displayno == 0)
        {
            ioctl(ctx->mFD_fb[1], FBIOGET_LAYER_HDL_1, &layer_hdl);
        }
        else
        {
            return 0;
        }
    }

    ctx->width[displayno]    = display_getwidth(ctx,displayno,ctx->out_type[displayno],ctx->out_format[displayno]);
    ctx->height[displayno]   = display_getheight(ctx,displayno,ctx->out_type[displayno],ctx->out_format[displayno]);
    ctx->valid_width[displayno] = (percent * ctx->width[displayno]) / 100;
    ctx->valid_height[displayno] = (percent * ctx->height[displayno]) / 100;
    
    scn_win.x = (ctx->width[displayno] - ctx->valid_width[displayno])/2;
    scn_win.width = ctx->valid_width[displayno];
    scn_win.y = (ctx->height[displayno] - ctx->valid_height[displayno])/2;
    scn_win.height = ctx->valid_height[displayno];

    arg[0] = displayno;
    arg[1] = layer_hdl;
    arg[2] = (unsigned long)&scn_win;
    ret = ioctl(ctx->mFD_disp,DISP_CMD_LAYER_SET_SCN_WINDOW,(unsigned long)arg);

    ctx->area_percent[displayno]   = percent;

    return  0;
}


static int display_getareapercent(struct display_device_t *dev,int displayno)
{
    struct display_context_t* ctx = (struct display_context_t*)dev;
    
	return ctx->area_percent[displayno];
}

static int display_open_output(struct display_device_t *dev, int screen, int out_type, int out_format)
{
    struct 	display_context_t*  ctx = (struct display_context_t*)dev;
    unsigned long 				arg[4];
    int							ret = 0;
    
    LOGV("####display_open_output,%d,%d,%d\n", screen,out_type,out_format);
    
    if(out_type == DISPLAY_DEVICE_HDMI)
    {
        arg[0] = screen;
        arg[1] = display_get_driver_tv_format(out_format);
        ret = ioctl(ctx->mFD_disp,DISP_CMD_HDMI_SET_MODE,(unsigned long)arg);
        
        arg[0] = screen;
        ret = ioctl(ctx->mFD_disp,DISP_CMD_HDMI_ON,(unsigned long)arg);
    }
    else if(out_type == DISPLAY_DEVICE_TV)
    {
        arg[0] = screen;
        arg[1] = display_get_driver_tv_format(out_format);
        ret = ioctl(ctx->mFD_disp,DISP_CMD_TV_SET_MODE,(unsigned long)arg);
        
        arg[0] = screen;
        ret = ioctl(ctx->mFD_disp,DISP_CMD_TV_ON,(unsigned long)arg);
    }
    else if(out_type == DISPLAY_DEVICE_VGA)
    {
        arg[0] = screen;
        arg[1] = display_get_driver_tv_format(out_format);
        ret = ioctl(ctx->mFD_disp,DISP_CMD_VGA_SET_MODE,(unsigned long)arg);
        
        arg[0] = screen;
        ret = ioctl(ctx->mFD_disp,DISP_CMD_VGA_ON,(unsigned long)arg);
    }
    else if(out_type == DISPLAY_DEVICE_LCD)
    {
        arg[0] = screen;
        ret = ioctl(ctx->mFD_disp,DISP_CMD_LCD_ON,(unsigned long)arg);
    
        arg[0] = screen;
        ctx->lcd_width = ioctl(ctx->mFD_disp,DISP_CMD_SCN_GET_WIDTH,arg);
        ctx->lcd_height = ioctl(ctx->mFD_disp,DISP_CMD_SCN_GET_HEIGHT,arg);
    }
    return 0;
}


static int display_close_output(struct display_device_t *dev, int screen)
{
    struct 	display_context_t*  ctx = (struct display_context_t*)dev;
    unsigned long 				arg[4];
    int							ret = 0;
    __disp_output_type_t        out_type;


    arg[0] = screen;
    out_type = (__disp_output_type_t)ioctl(ctx->mFD_disp,DISP_CMD_GET_OUTPUT_TYPE,(unsigned long)arg);

    LOGV("####display_close_output,%d,%d,%d\n", screen,out_type);

    if(out_type == DISP_OUTPUT_TYPE_HDMI)
    {
        arg[0] = screen;
        ret = ioctl(ctx->mFD_disp,DISP_CMD_HDMI_OFF,(unsigned long)arg);
    }
    else if(out_type == DISP_OUTPUT_TYPE_TV)
    {
        arg[0] = screen;
        ret = ioctl(ctx->mFD_disp,DISP_CMD_TV_OFF,(unsigned long)arg);
    }
    else if(out_type == DISP_OUTPUT_TYPE_VGA)
    {
        arg[0] = screen;
        ret = ioctl(ctx->mFD_disp,DISP_CMD_VGA_OFF,(unsigned long)arg);
    }
    else if(out_type == DISP_OUTPUT_TYPE_LCD)
    {
        arg[0] = screen;
        ret = ioctl(ctx->mFD_disp,DISP_CMD_LCD_OFF,(unsigned long)arg);
    }
    return 0;
}

static int display_setparameter(struct display_device_t *dev, int displayno, int type,int format)
{
    return  0;
}

static int display_getparameter(struct display_device_t *dev, int displayno, int param)
{
    struct 	display_context_t* ctx = (struct display_context_t*)dev;
    struct fb_var_screeninfo    var_src;
    
	if(displayno < 0 || displayno > MAX_DISPLAY_NUM)
	{
        LOGE("Invalid Display No!\n");

        return  -1;
    }

    switch(param)
    {
        case   DISPLAY_VALID_WIDTH:            return ctx->valid_width[displayno];
        case   DISPLAY_VALID_HEIGHT:            return ctx->valid_height[displayno];
        case   DISPLAY_OUTPUT_WIDTH:            return  ctx->width[displayno];
        case   DISPLAY_OUTPUT_HEIGHT:           return  ctx->height[displayno];
        case   DISPLAY_APP_WIDTH:               return ctx->app_width[displayno];
        case   DISPLAY_APP_HEIGHT:              return ctx->app_height[displayno];
        case   DISPLAY_OUTPUT_PIXELFORMAT:      return  ctx->pixel_format[displayno];
        case   DISPLAY_OUTPUT_FORMAT:           return  ctx->out_format[displayno];
        case   DISPLAY_OUTPUT_TYPE:             return  ctx->out_type[displayno];
        case   DISPLAY_OUTPUT_ISOPEN :          return 0;
        case   DISPLAY_OUTPUT_HOTPLUG:          return 0;
		case   DISPLAY_FBWIDTH:
			ioctl(ctx->mFD_fb[displayno],FBIOGET_VSCREENINFO,&var_src);
			return var_src.xres;
		case   DISPLAY_FBHEIGHT:
			ioctl(ctx->mFD_fb[displayno],FBIOGET_VSCREENINFO,&var_src);
			return var_src.yres;
		case DISPLAY_UI_HDL:
		    return ctx->layer_hdl[displayno];
        default:
            LOGE("Invalid Display Parameter!\n");

            return  -1;
    }
}

static int display_setorientation(struct display_device_t *dev,int orientation)
{
    struct 	display_context_t*  ctx = (struct display_context_t*)dev;

    LOGD("####display_setorientation %d\n", orientation);
    
    if(ctx->mode == DISPLAY_MODE_SINGLE_VAR_GPU)
    {
        unsigned int layer_hdl;
        __disp_layer_info_t layer_para;
        struct fb_var_screeninfo    var;
        int                         ret = 0;
        unsigned long               arg[4];
        int app_width = ctx->app_width[0];
        int app_height = ctx->app_height[0];

        ioctl(ctx->mFD_fb[0],FBIOGET_VSCREENINFO,&var);

        ioctl(ctx->mFD_fb[0], FBIOGET_LAYER_HDL_0, &layer_hdl);
        arg[0] = 0;
        arg[1] = layer_hdl;
        arg[2] = (unsigned long)&layer_para;
        ret = ioctl(ctx->mFD_disp,DISP_CMD_LAYER_GET_PARA,(unsigned long)arg);

        if(orientation == 1 || orientation==3)
        {
            if(ctx->out_type[0] != DISPLAY_DEVICE_LCD)
            {
                app_width = ctx->app_height[0];
                app_height = ctx->app_width[0];
            }
        }

        ctx->width[0] = display_getwidth(ctx,0,ctx->out_type[0],ctx->out_format[0]);
        ctx->height[0] = display_getheight(ctx,0,ctx->out_type[0],ctx->out_format[0]);
        ctx->valid_width[0] = (ctx->area_percent[0] * ctx->width[0]) / 100;
        ctx->valid_height[0] = (ctx->area_percent[0] * ctx->height[0]) / 100;
        if((app_width < app_height) && (ctx->valid_width[0] > ctx->valid_height[0]))
        {
            ctx->valid_width[0] = app_width * ctx->valid_height[0] / app_height;
        }
        else if((app_width > app_height) && (ctx->valid_width[0] < ctx->valid_height[0]))
        {
            ctx->valid_height[0] = app_height * ctx->valid_width[0] / app_width;
        }
        
        layer_para.scn_win.x = (ctx->width[0] - ctx->valid_width[0])/2;
        layer_para.scn_win.y = (ctx->height[0] - ctx->valid_height[0])/2;
        layer_para.scn_win.width = ctx->valid_width[0];
        layer_para.scn_win.height = ctx->valid_height[0];

        if(orientation == 0)
        {
            layer_para.src_win.x = 0;
            layer_para.src_win.y = 0;
        }
        else if(orientation == 1)
        {
            layer_para.src_win.x = var.xres - ctx->valid_width[0];
            layer_para.src_win.y = 0;
        }
        else if(orientation == 2)
        {
            layer_para.src_win.x = var.xres - ctx->valid_width[0];
            layer_para.src_win.y = var.yres - ctx->valid_height[0];
        }
        else if(orientation == 3)
        {
            layer_para.src_win.x = 0;
            layer_para.src_win.y = var.yres - ctx->valid_height[0];
        }
        
        ret = ioctl(ctx->mFD_disp,DISP_CMD_LAYER_SET_PARA,(unsigned long)arg);

        LOGD("GGGGG %d,%d,  %d,%d,  %d,%d\n",
        layer_para.src_win.x,layer_para.src_win.y,layer_para.scn_win.x,layer_para.scn_win.y,layer_para.scn_win.width,layer_para.scn_win.height);


    }
    else if(ctx->isenable_mp && ctx->mode == DISPLAY_MODE_DUALSAME)
    {
        unsigned int layer_hdl;
        __disp_layer_info_t layer_para;
        struct fb_fix_screeninfo    fix_fb0;
        int                         ret = 0;
        unsigned long               arg[4];

        ioctl(ctx->mFD_fb[1], FBIOGET_LAYER_HDL_1, &layer_hdl);
       
        arg[0] = 1;
        arg[1] = layer_hdl;
        arg[2] = (unsigned long)&layer_para;
        ret = ioctl(ctx->mFD_disp,DISP_CMD_LAYER_GET_PARA,(unsigned long)arg);

      //  if(orientation == 0 || orientation == 2)
       if(orientation == 1 || orientation == 3)
        {
            ctx->valid_height[1] = ctx->height[1];
            ctx->valid_width[1] = ctx->width[0] * ctx->valid_height[1] / ctx->height[0];
            layer_para.src_win.x = 0;
            layer_para.src_win.y = 0;
            layer_para.src_win.width = ctx->width[0];
            layer_para.src_win.height = ctx->height[0];
        }
      //  else if(orientation == 1 || orientation == 3)
      else  if(orientation == 0 || orientation == 2)
        {
            ctx->valid_height[1] = ctx->height[1];
            ctx->valid_width[1] =ctx->width[1];
            layer_para.src_win.x = 0;
            layer_para.src_win.y = 0;
            layer_para.src_win.width = ctx->height[0];//ctx->valid_width[1];
            layer_para.src_win.height = ctx->width[0];
        }

        layer_para.fb.addr[0] = ctx->fb1_mem_start;
        
        layer_para.scn_win.x = (ctx->width[1] - ctx->valid_width[1])/2;
        layer_para.scn_win.y = (ctx->height[1] - ctx->valid_height[1])/2;
        layer_para.scn_win.width = ctx->valid_width[1];
        layer_para.scn_win.height = ctx->valid_height[1];
        
        arg[0] = 1;
        arg[1] = layer_hdl;
        arg[2] = (unsigned long)&layer_para;
        ret = ioctl(ctx->mFD_disp,DISP_CMD_LAYER_SET_PARA,(unsigned long)arg);

        LOGV("####display_setorientation,src[%d,%d,%d,%d] , scn[%d,%d,%d,%d],isenable_mp:%d\n",
            layer_para.src_win.x,layer_para.src_win.y,layer_para.src_win.width,layer_para.src_win.height,
            layer_para.scn_win.x,layer_para.scn_win.y,layer_para.scn_win.width,layer_para.scn_win.height,ctx->isenable_mp);
    }
    else if(!ctx->isenable_mp && ctx->mode == DISPLAY_MODE_DUALSAME)
    {
        unsigned int layer_hdl;
        __disp_layer_info_t layer_para;
        int                         ret = 0;
        unsigned long               arg[4];
        
        ioctl(ctx->mFD_fb[0], FBIOGET_LAYER_HDL_0, &layer_hdl);
        
        arg[0] = 0;
        arg[1] = layer_hdl;
        arg[2] = (unsigned long)&layer_para;
        ret = ioctl(ctx->mFD_disp,DISP_CMD_LAYER_GET_PARA,(unsigned long)arg);

        layer_para.mode = DISP_LAYER_WORK_MODE_SCALER;
        
        layer_para.src_win.x = 0;
        layer_para.src_win.width = ctx->app_width[0];
        layer_para.src_win.y = (layer_para.src_win.y + ctx->app_height[0] - 1) / ctx->app_height[0] * ctx->app_height[0];
        layer_para.src_win.height = ctx->app_height[0];


        layer_para.scn_win.x = (ctx->width[1] - ctx->valid_width[1])/2;
        layer_para.scn_win.y = (ctx->height[1] - ctx->valid_height[1])/2;
        layer_para.scn_win.width = ctx->valid_width[1];
        layer_para.scn_win.height = ctx->valid_height[1];


        ioctl(ctx->mFD_fb[1], FBIOGET_LAYER_HDL_1, &layer_hdl);

        arg[0] = 1;
        arg[1] = layer_hdl;
        arg[2] = (unsigned long)&layer_para;
        ret = ioctl(ctx->mFD_disp,DISP_CMD_LAYER_SET_PARA,(unsigned long)arg);

        LOGV("####display_setorientation,src:[%d,%d,%d,%d] , scn:[%d,%d,%d,%d],isenable_mp:%d\n",
            layer_para.src_win.x,layer_para.src_win.y,layer_para.src_win.width,layer_para.src_win.height,
            layer_para.scn_win.x,layer_para.scn_win.y,layer_para.scn_win.width,layer_para.scn_win.height,ctx->isenable_mp);
    }
    ctx->orientation = orientation;
    
    return 0;
}

static int display_setmode(struct display_device_t *dev,int mode,struct display_modepara_t *para)
{
    struct 	display_context_t*  ctx = (struct display_context_t*)dev;
    int							ret = 0;
    unsigned long 				arg[4];
    __disp_fb_create_para_t 	fb_para;
    int i = 0;

    LOGD("####display_setmode:%d,screen0_type:%d,screen0_format:%d,screen1_type:%d,screen1:format:%d\n", 
        mode,para->d0type,para->d0format,para->d1type,para->d1format);

    if(mode == DISPLAY_MODE_DUALSAME)
    {
        __disp_layer_info_t layer_info;
        unsigned int layer_hdl;
        __disp_rect_t scn_win;
        struct fb_fix_screeninfo    fix_fb1;

        if(mode == ctx->mode && para->d0type == ctx->out_type[0] && para->d0format == ctx->out_format[0] &&
            para->d1type == ctx->out_type[1] && para->d1format == ctx->out_format[1])
        {
            return -1;
        }
        if(ctx->out_type[0] == DISPLAY_DEVICE_HDMI && para->d1type == DISPLAY_DEVICE_HDMI)
        {
            return -1;
        }
        if(ctx->out_type[0] == DISPLAY_DEVICE_TV && para->d1type == DISPLAY_DEVICE_TV)
        {
            if((ctx->out_format[0] != DISPLAY_TVFORMAT_PAL &&  ctx->out_format[0] != DISPLAY_TVFORMAT_NTSC) && 
                (ctx->out_format[0] != DISPLAY_TVFORMAT_PAL &&  ctx->out_format[0] != DISPLAY_TVFORMAT_NTSC))
            {
                return -1;
            }
        }
        if(ctx->out_type[0] == DISPLAY_DEVICE_VGA && para->d1type == DISPLAY_DEVICE_TV)
        {
            if(para->d1format != DISPLAY_TVFORMAT_PAL &&  para->d1format != DISPLAY_TVFORMAT_NTSC)
            {   
                return -1;
            }
        }
        if(ctx->out_type[0] == DISPLAY_DEVICE_TV && para->d1type == DISPLAY_DEVICE_VGA)
        {
            if(ctx->out_format[0] != DISPLAY_TVFORMAT_PAL &&  ctx->out_format[0] != DISPLAY_TVFORMAT_NTSC)
            {
                return -1;
            }
        }
        

        if(ctx->mode != DISPLAY_MODE_SINGLE)
        {
            display_close_output(dev, 1);
            
            arg[0] = 1;
            ret = ioctl(ctx->mFD_disp,DISP_CMD_FB_RELEASE,(unsigned long)arg);
        }
        
        ctx->width[0] = display_getwidth(ctx,0,para->d0type,para->d0format);
        ctx->height[0] = display_getheight(ctx,0,para->d0type,para->d0format);
		
        ctx->width[1] = display_getwidth(ctx,1,para->d1type,para->d1format);
        ctx->height[1] = display_getheight(ctx,1,para->d1type,para->d1format);
        ctx->valid_width[0] = (ctx->area_percent[0] * ctx->width[0]) / 100;
        ctx->valid_height[0] = (ctx->area_percent[0] * ctx->height[0]) / 100;
        ctx->valid_width[1] = (ctx->area_percent[1] * ctx->width[1]) / 100;
        ctx->valid_height[1] = (ctx->area_percent[1] * ctx->height[1]) / 100;

        fb_para.fb_mode = FB_MODE_SCREEN1;
        fb_para.mode = DISP_LAYER_WORK_MODE_SCALER;//DISP_LAYER_WORK_MODE_NORMAL;
        fb_para.buffer_num = DISPLAYDISPATCH_MAXBUFNO;//3;
        unsigned int longsize = (ctx->width[0] > ctx->height[0]) ? ctx->width[0]:ctx->height[0];
        fb_para.width = longsize;//ctx->width[1];
        fb_para.height = longsize;//ctx->height[1];
        fb_para.output_width = ctx->valid_width[1];
        fb_para.output_height = ctx->valid_height[1];
        arg[0]                      = 1;
        arg[1]                      = (unsigned long)&fb_para;
        ret = ioctl(ctx->mFD_disp,DISP_CMD_FB_REQUEST,(unsigned long)arg);

        ioctl(ctx->mFD_fb[1], FBIOGET_LAYER_HDL_1, &layer_hdl);
        ctx->layer_hdl[1] = layer_hdl;
        
        scn_win.x = (ctx->width[1] - ctx->valid_width[1])/2;
        scn_win.width = ctx->valid_width[1];
        scn_win.y = (ctx->height[1] - ctx->valid_height[1])/2;
        scn_win.height = ctx->valid_height[1];
        arg[0] = 1;
        arg[1] = layer_hdl;
        arg[2] = (unsigned long)&scn_win;
        ret = ioctl(ctx->mFD_disp,DISP_CMD_LAYER_SET_SCN_WINDOW,(unsigned long)arg);

        ioctl(ctx->mFD_fb[1],FBIOGET_FSCREENINFO,&fix_fb1);
        ctx->fb1_mem_start = fix_fb1.smem_start;

        ctx->mode = mode;
        ctx->out_type[0] = para->d0type;
        ctx->out_format[0] = para->d0format;
        ctx->pixel_format [0] = para->d0pixelformat;
        ctx->out_type[1] = para->d1type;
        ctx->out_format[1] = para->d1format;
        ctx->pixel_format [1] = para->d1pixelformat;
        
        ctx->isenable_mp = ctx->width[0] < ctx->height[0];
        display_setorientation(dev,ctx->orientation);
        if(ctx->isenable_mp)
        {
            arg[0] = 0;
            arg[1] = 50;
            ioctl(ctx->mFD_disp,DISP_CMD_LCD_SET_FPS,(unsigned long)arg);
            display_copyfb(dev,0,0,1,0);
        }
        display_open_output(dev, 1, para->d1type, para->d1format);
    }
    else if(mode == DISPLAY_MODE_DUALSAME_TWO_VIDEO)
    {
        unsigned int layer_hdl;
        __disp_rect_t scn_win;

        if(ctx->out_type[0] == DISPLAY_DEVICE_HDMI && para->d1type == DISPLAY_DEVICE_HDMI)
        {
            return -1;
        }
        if(ctx->out_type[0] == DISPLAY_DEVICE_TV && para->d1type == DISPLAY_DEVICE_TV)
        {
            if((ctx->out_format[0] != DISPLAY_TVFORMAT_PAL &&  ctx->out_format[0] != DISPLAY_TVFORMAT_NTSC) && 
                (ctx->out_format[0] != DISPLAY_TVFORMAT_PAL &&  ctx->out_format[0] != DISPLAY_TVFORMAT_NTSC))
            {
                return -1;
            }
        }
        if(ctx->out_type[0] == DISPLAY_DEVICE_VGA && para->d1type == DISPLAY_DEVICE_TV)
        {
            if(para->d1format != DISPLAY_TVFORMAT_PAL &&  para->d1format != DISPLAY_TVFORMAT_NTSC)
            {   
                return -1;
            }
        }
        if(ctx->out_type[0] == DISPLAY_DEVICE_TV && para->d1type == DISPLAY_DEVICE_VGA)
        {
            if(ctx->out_format[0] != DISPLAY_TVFORMAT_PAL &&  ctx->out_format[0] != DISPLAY_TVFORMAT_NTSC)
            {
                return -1;
            }
        }

        if(ctx->mode != DISPLAY_MODE_SINGLE)
        {
            display_close_output(dev, 1);
            
            arg[0] = 1;
            ret = ioctl(ctx->mFD_disp,DISP_CMD_FB_RELEASE,(unsigned long)arg);
        }

        ctx->width[0] = display_getwidth(ctx,0,para->d0type,para->d0format);
        ctx->height[0] = display_getheight(ctx,0,para->d0type,para->d0format);
        ctx->width[1] = display_getwidth(ctx,1,para->d1type,para->d1format);
        ctx->height[1] = display_getheight(ctx,1,para->d1type,para->d1format);
        ctx->valid_width[0] = (ctx->area_percent[0] * ctx->width[0]) / 100;
        ctx->valid_height[0] = (ctx->area_percent[0] * ctx->height[0]) / 100;
        ctx->valid_width[1] = (ctx->area_percent[1] * ctx->width[1]) / 100;
        ctx->valid_height[1] = (ctx->area_percent[1] * ctx->height[1]) / 100;

        fb_para.fb_mode = FB_MODE_SCREEN1;
        fb_para.mode = DISP_LAYER_WORK_MODE_NORMAL;
        fb_para.buffer_num = 3;
        fb_para.width = ctx->valid_width[1];
        fb_para.height = ctx->valid_height[1];
        fb_para.output_width = ctx->valid_width[1];
        fb_para.output_height = ctx->valid_height[1];
        arg[0]                      = 1;
        arg[1]                      = (unsigned long)&fb_para;
        ret = ioctl(ctx->mFD_disp,DISP_CMD_FB_REQUEST,(unsigned long)arg);
        
        display_copyfb(dev,0,0,1,0);

        ioctl(ctx->mFD_fb[1], FBIOGET_LAYER_HDL_1, &layer_hdl);

        scn_win.x = (ctx->width[1] - ctx->valid_width[1])/2;
        scn_win.width = ctx->valid_width[1];
        scn_win.y = (ctx->height[1] - ctx->valid_height[1])/2;
        scn_win.height = ctx->valid_height[1];
        arg[0] = 1;
        arg[1] = layer_hdl;
        arg[2] = (unsigned long)&scn_win;
        ret = ioctl(ctx->mFD_disp,DISP_CMD_LAYER_SET_SCN_WINDOW,(unsigned long)arg);

        display_open_output(dev, 1, para->d1type, para->d1format);
    }
    else if(mode == DISPLAY_MODE_SINGLE)
    {
        if(ctx->mode != DISPLAY_MODE_SINGLE)
        {
            display_close_output(dev, 1);
            
            arg[0] = 1;
            ret = ioctl(ctx->mFD_disp,DISP_CMD_FB_RELEASE,(unsigned long)arg);
            if(ctx->isenable_mp)
            {
                arg[0] = 0;
                arg[1] = 60;
                ioctl(ctx->mFD_disp,DISP_CMD_LCD_SET_FPS,(unsigned long)arg);
                ctx->fb1_mem_start = 0;
                ctx->isenable_mp = false;
            }
            //arg[1] = ctx->layer_hdl[1];
            //ret = ioctl(ctx->mFD_disp,DISP_CMD_LAYER_RELEASE,(unsigned long)arg);
        }
    }    
    else if(mode == DISPLAY_MODE_SINGLE_VAR_FE)
    {
        if((para->d0type != ctx->out_type[0]) || (para->d0format != ctx->out_format[0]))
        {
            unsigned int layer_hdl;
            __disp_layer_info_t layer_para;
        
            display_close_output(dev, 0);
       
            ctx->width[0] = display_getwidth(ctx,0,para->d0type,para->d0format);
            ctx->height[0] = display_getheight(ctx,0,para->d0type,para->d0format);
            ctx->valid_width[0] = (ctx->area_percent[0] * ctx->width[0]) / 100;
            ctx->valid_height[0] = (ctx->area_percent[0] * ctx->height[0]) / 100;

            ioctl(ctx->mFD_fb[0], FBIOGET_LAYER_HDL_0, &layer_hdl);
        
            arg[0] = 0;
            arg[1] = layer_hdl;
            arg[2] = (unsigned long)&layer_para;
            ret = ioctl(ctx->mFD_disp,DISP_CMD_LAYER_GET_PARA,(unsigned long)arg);
            
            layer_para.mode = DISP_LAYER_WORK_MODE_SCALER;
            layer_para.scn_win.x = (ctx->width[0] - ctx->valid_width[0])/2;
            layer_para.scn_win.width = ctx->valid_width[0];
            layer_para.scn_win.y = (ctx->height[0] - ctx->valid_height[0])/2;
            layer_para.scn_win.height = ctx->valid_height[0];
            
            arg[0] = 0;
            arg[1] = layer_hdl;
            arg[2] = (unsigned long)&layer_para;
            ret = ioctl(ctx->mFD_disp,DISP_CMD_LAYER_SET_PARA,(unsigned long)arg);

            display_open_output(dev, 0,para->d0type,para->d0format);
        }
    }
    else if(mode == DISPLAY_MODE_SINGLE_VAR_BE)
    {
        if((para->d0type != ctx->out_type[0]) || (para->d0format != ctx->out_format[0]))
        {
            unsigned int layer_hdl;
            __disp_layer_info_t layer_para;
            
            display_close_output(dev, 0);
        
            ctx->width[0] = display_getwidth(ctx,0,para->d0type,para->d0format);
            ctx->height[0] = display_getheight(ctx,0,para->d0type,para->d0format);
            ctx->valid_width[0] = (ctx->area_percent[0] * ctx->width[0]) / 100;
            ctx->valid_height[0] = (ctx->area_percent[0] * ctx->height[0]) / 100;

            if(ctx->mode == DISPLAY_MODE_SINGLE_VAR_BE)
            {
                arg[0] = 1;
                ret = ioctl(ctx->mFD_disp,DISP_CMD_FB_RELEASE,(unsigned long)arg);
            }
            else
            {
                ioctl(ctx->mFD_fb[0], FBIOGET_LAYER_HDL_0, &layer_hdl);

                arg[0] = 0;
                arg[1] = layer_hdl;
                ioctl(ctx->mFD_disp,DISP_CMD_LAYER_CLOSE,(unsigned long)arg);
            }

            fb_para.fb_mode = FB_MODE_SCREEN0;
            fb_para.mode = DISP_LAYER_WORK_MODE_NORMAL;
            fb_para.buffer_num = 3;
            fb_para.width = ctx->valid_width[0];
            fb_para.height = ctx->valid_height[0];
            fb_para.output_width = ctx->valid_width[0];
            fb_para.output_height = ctx->valid_height[0];
            arg[0]                      = 1;
            arg[1]                      = (unsigned long)&fb_para;
            ret = ioctl(ctx->mFD_disp,DISP_CMD_FB_REQUEST,(unsigned long)arg);

            ioctl(ctx->mFD_fb[1], FBIOGET_LAYER_HDL_0, &layer_hdl);
            
            arg[0] = 0;
            arg[1] = layer_hdl;
            arg[2] = (unsigned long)&layer_para;
            ret = ioctl(ctx->mFD_disp,DISP_CMD_LAYER_GET_PARA,(unsigned long)arg);
            
            layer_para.scn_win.x = (ctx->width[0] - ctx->valid_width[0])/2;
            layer_para.scn_win.width = ctx->valid_width[0];
            layer_para.scn_win.y = (ctx->height[0] - ctx->valid_height[0])/2;
            layer_para.scn_win.height = ctx->valid_height[0];
            
            arg[0] = 0;
            arg[1] = layer_hdl;
            arg[2] = (unsigned long)&layer_para;
            ret = ioctl(ctx->mFD_disp,DISP_CMD_LAYER_SET_PARA,(unsigned long)arg);

            display_open_output(dev, 0,para->d0type,para->d0format);
        }
    }
    else if(mode == DISPLAY_MODE_SINGLE_FB_VAR)
    {
        if((para->d0type != ctx->out_type[0]) || (para->d0format != ctx->out_format[0]))
        {
            unsigned int layer_hdl;
            __disp_layer_info_t layer_para;

            arg[0] = 0;
            ret = ioctl(ctx->mFD_disp,DISP_CMD_FB_RELEASE,(unsigned long)arg);

            display_close_output(dev, 0);
        
            ctx->width[0] = display_getwidth(ctx,0,para->d0type,para->d0format);
            ctx->height[0] = display_getheight(ctx,0,para->d0type,para->d0format);
            ctx->valid_width[0] = (ctx->area_percent[0] * ctx->width[0]) / 100;
            ctx->valid_height[0] = (ctx->area_percent[0] * ctx->height[0]) / 100;

            fb_para.fb_mode = FB_MODE_SCREEN0;
            fb_para.mode = DISP_LAYER_WORK_MODE_NORMAL;
            fb_para.buffer_num = 2;
            fb_para.width = ctx->valid_width[0];
            fb_para.height = ctx->valid_height[0];
            fb_para.output_width = ctx->valid_width[0];
            fb_para.output_height = ctx->valid_height[0];
            arg[0]                      = 0;
            arg[1]                      = (unsigned long)&fb_para;
            ret = ioctl(ctx->mFD_disp,DISP_CMD_FB_REQUEST,(unsigned long)arg);

            ioctl(ctx->mFD_fb[0], FBIOGET_LAYER_HDL_0, &layer_hdl);
            
            arg[0] = 0;
            arg[1] = layer_hdl;
            arg[2] = (unsigned long)&layer_para;
            ret = ioctl(ctx->mFD_disp,DISP_CMD_LAYER_GET_PARA,(unsigned long)arg);
            
            layer_para.scn_win.x = (ctx->width[0] - ctx->valid_width[0])/2;
            layer_para.scn_win.width = ctx->valid_width[0];
            layer_para.scn_win.y = (ctx->height[0] - ctx->valid_height[0])/2;
            layer_para.scn_win.height = ctx->valid_height[0];
            
            arg[0] = 0;
            arg[1] = layer_hdl;
            arg[2] = (unsigned long)&layer_para;
            ret = ioctl(ctx->mFD_disp,DISP_CMD_LAYER_SET_PARA,(unsigned long)arg);

            display_open_output(dev, 0,para->d0type,para->d0format);
        }
    }
    else if(mode == DISPLAY_MODE_SINGLE_VAR_GPU)
    {
        if((para->d0type != ctx->out_type[0]) || (para->d0format != ctx->out_format[0]))
        {
            unsigned int layer_hdl;
            __disp_layer_info_t layer_para;
            
            display_close_output(dev, 0);
			usleep(1000*500);
        
            ctx->width[0] = display_getwidth(ctx,0,para->d0type,para->d0format);
            ctx->height[0] = display_getheight(ctx,0,para->d0type,para->d0format);
            ctx->valid_width[0] = (ctx->area_percent[0] * ctx->width[0]) / 100;
            ctx->valid_height[0] = (ctx->area_percent[0] * ctx->height[0]) / 100;
           
            ioctl(ctx->mFD_fb[0], FBIOGET_LAYER_HDL_0, &layer_hdl);
            
            arg[0] = 0;
            arg[1] = layer_hdl;
            arg[2] = (unsigned long)&layer_para;
            ret = ioctl(ctx->mFD_disp,DISP_CMD_LAYER_GET_PARA,(unsigned long)arg);
            
            layer_para.scn_win.x = (ctx->width[0] - ctx->valid_width[0])/2;
            layer_para.scn_win.y = (ctx->height[0] - ctx->valid_height[0])/2;
            layer_para.scn_win.width = ctx->valid_width[0];
            layer_para.scn_win.height = ctx->valid_height[0];
            
            arg[0] = 0;
            arg[1] = layer_hdl;
            arg[2] = (unsigned long)&layer_para;
            ret = ioctl(ctx->mFD_disp,DISP_CMD_LAYER_SET_PARA,(unsigned long)arg);

            ctx->mode = mode;
            ctx->out_type[0] = para->d0type;
            ctx->out_format[0] = para->d0format;
            ctx->pixel_format [0] = para->d0pixelformat;
            ctx->out_type[1] = para->d1type;
            ctx->out_format[1] = para->d1format;
            ctx->pixel_format [1] = para->d1pixelformat;
            display_setorientation(dev,ctx->orientation);

            display_open_output(dev, 0,para->d0type,para->d0format);
        }
    }
    
    ctx->mode = mode;
    ctx->out_type[0] = para->d0type;
    ctx->out_format[0] = para->d0format;
    ctx->pixel_format [0] = para->d0pixelformat;
    ctx->out_type[1] = para->d1type;
    ctx->out_format[1] = para->d1format;
    ctx->pixel_format [1] = para->d1pixelformat;

    return  1;
}
      
static int display_getmode(struct display_device_t *dev)
{   
    struct display_context_t* ctx = (struct display_context_t*)dev;
    
    return  ctx->mode;;
}

static int display_setoutputtype(struct display_device_t *dev,int displayno,int type,int format) 
{
    struct display_context_t* ctx = (struct display_context_t*)dev;
    struct display_modepara_t para;

    if(displayno == 0)
    {
        para.d0type = type;
        para.d0format = format;
        para.d1type = ctx->out_type[1];
        para.d1format = ctx->out_format[1];
    }
    else
    {
        para.d0type = ctx->out_type[0];
        para.d0format = ctx->out_format[0];
        para.d1type = type;
        para.d1format = format;
    }
    display_setmode(dev,ctx->mode,&para);
    return 0;
}

static int display_getoutputtype(struct display_device_t *dev,int displayno)
{
    struct display_context_t* ctx = (struct display_context_t*)dev;
    int                       ret;
    unsigned long args[4];

    args[0] = displayno;
    ret = ioctl(ctx->mFD_disp,DISP_CMD_GET_OUTPUT_TYPE,args);
    if(ret == DISP_OUTPUT_TYPE_LCD)
    {
        return  DISPLAY_DEVICE_LCD;
    }
    else if(ret == DISP_OUTPUT_TYPE_TV)
    {
        return  DISPLAY_DEVICE_TV;
    }
    else if(ret == DISP_OUTPUT_TYPE_HDMI)
    {
        return  DISPLAY_DEVICE_HDMI;
    }
    else if(ret == DISP_OUTPUT_TYPE_VGA)
    {
        return  DISPLAY_DEVICE_VGA;
    } 

    return  DISPLAY_DEVICE_NONE;
}

static int display_init(struct display_context_t* ctx)
{
    unsigned long arg[4];
    __disp_init_t init_para;
    unsigned int sel = 0, i = 0;
    char             node_name[20];


    ctx->mFD_disp = open("/dev/disp", O_RDWR, 0);
    if (ctx->mFD_disp < 0) 
    {
        LOGE("Error opening display driver, ret:%d, errno:%d", ctx->mFD_disp, errno);
        return -1;
    } 

    ctx->mFD_mp                     = open("/dev/g2d", O_RDWR, 0);
    if(ctx->mFD_mp < 0)
    {
        LOGE("Error opening g2d driver, ret:%d, errno:%d", ctx->mFD_mp, errno);
    }

    for(i=0; i<MAX_DISPLAY_NUM; i++)
    {
        sprintf(node_name, "/dev/graphics/fb%d", i);

    	ctx->mFD_fb[i]			= open(node_name,O_RDWR,0);
    	if(ctx->mFD_fb[i] <= 0)
    	{
            LOGE("Error opening fb%d driver, ret:%d, errno:%d",i,ctx->mFD_fb[i], errno);
            return -1;
        }
    }

    ctx->mFD_mem = open("/dev/sunxi_mem", O_RDWR, 0);
    if(ctx->mFD_mem < 0)
    {
        LOGE("Error opening sunxi_mem driver, ret:%d, errno:%d", ctx->mFD_mem, errno);
    }
    
    ctx->area_percent[0] = 100;
    ctx->area_percent[1] = 98;

    arg[0] = (unsigned long)&init_para;
    ioctl(ctx->mFD_disp,DISP_CMD_GET_DISP_INIT_PARA,(unsigned long)arg);

    ctx->mode = DISPLAY_MODE_SINGLE;
    
    if(init_para.b_init)
    {        
        for(sel=0; sel<2; sel++)
        {
            if(((sel == 0) && ((init_para.disp_mode == DISP_INIT_MODE_SCREEN0) || (init_para.disp_mode == DISP_INIT_MODE_TWO_DIFF_SCREEN)
                    || (init_para.disp_mode == DISP_INIT_MODE_TWO_SAME_SCREEN)))
            || ((sel == 1) && ((init_para.disp_mode == DISP_INIT_MODE_SCREEN1) || (init_para.disp_mode == DISP_INIT_MODE_TWO_DIFF_SCREEN)
                    || (init_para.disp_mode == DISP_INIT_MODE_TWO_SAME_SCREEN))))
            {
                if(init_para.output_type[sel] == DISP_OUTPUT_TYPE_LCD)
                {
                    ctx->out_type[sel] = DISPLAY_DEVICE_LCD;
                }
                else if(init_para.output_type[sel] == DISP_OUTPUT_TYPE_HDMI)
                {
                    ctx->out_type[sel] = DISPLAY_DEVICE_HDMI;
                }
                else if(init_para.output_type[sel] == DISP_OUTPUT_TYPE_TV)
                {
                    ctx->out_type[sel] = DISPLAY_DEVICE_TV;
                }
                else if(init_para.output_type[sel] == DISP_OUTPUT_TYPE_VGA)
                {
                    ctx->out_type[sel] = DISPLAY_DEVICE_VGA;
                }
                else
                {
                    ctx->out_type[sel] = DISPLAY_DEVICE_HDMI;
                }
                
                if(init_para.output_type[sel] == DISP_OUTPUT_TYPE_TV || init_para.output_type[sel] == DISP_OUTPUT_TYPE_HDMI)
                {
                    for(i=0; i<sizeof(g_tv_para)/sizeof(struct tv_para_t); i++)
                    {
                        if((init_para.tv_mode[sel] == g_tv_para[i].driver_mode) && (g_tv_para[i].type&0xd))
                        {
                            ctx->out_format[sel] = g_tv_para[i].mode;
                            break;
                        }
                    }
                    if(i==sizeof(g_tv_para)/sizeof(struct tv_para_t))
                    {
                        ctx->out_format[sel] = DISPLAY_TVFORMAT_720P_50HZ;
                    }
                }
                else if(init_para.output_type[sel] == DISP_OUTPUT_TYPE_VGA)
                {
                    for(i=0; i<sizeof(g_tv_para)/sizeof(struct tv_para_t); i++)
                    {
                        if((init_para.vga_mode[sel] == g_tv_para[i].driver_mode) && (g_tv_para[i].type&0x2))
                        {
                            ctx->out_format[sel] = g_tv_para[i].mode;
                            break;
                        }
                    }
                    if(i==sizeof(g_tv_para)/sizeof(struct tv_para_t))
                    {
                        ctx->out_format[sel] = DISPLAY_VGA_H1024_V768;
                    }
                }
                else if(init_para.output_type[sel] == DISP_OUTPUT_TYPE_LCD)
                {
                    arg[0] = sel;
                    ctx->lcd_width = ioctl(ctx->mFD_disp,DISP_CMD_SCN_GET_WIDTH,(unsigned long)arg);
                    ctx->lcd_height = ioctl(ctx->mFD_disp,DISP_CMD_SCN_GET_HEIGHT,(unsigned long)arg);
                }

                ctx->width[sel] = display_getwidth(ctx, sel, ctx->out_type[sel], ctx->out_format[sel]);
                ctx->height[sel] = display_getheight(ctx, sel, ctx->out_type[sel], ctx->out_format[sel]);
                ctx->valid_width[sel] = (ctx->area_percent[sel] * ctx->width[sel]) / 100;
                ctx->valid_height[sel] = (ctx->area_percent[sel] * ctx->height[sel]) / 100;
                ctx->app_width[sel] = display_getwidth(ctx, sel, ctx->out_type[sel], ctx->out_format[sel]);
                ctx->app_height[sel] = display_getheight(ctx, sel, ctx->out_type[sel], ctx->out_format[sel]);

                LOGD("####disp_init, mode:%d,out_type:%d,tv_mode:%d,app_width:%d,app_height:%d\n",
                    ctx->mode, ctx->out_type[sel],ctx->out_format[sel],ctx->app_width[sel],ctx->app_height[sel]);
            }
        }
    }

    return 0;
}

static int display_opendev(struct display_device_t *dev,int displayno)
{
    return 0;
}

static int display_closedev(struct display_device_t *dev,int displayno)
{
    return 0;    
}


static int close_display(struct hw_device_t *dev) 
{
    int    i;
    
    struct display_context_t* ctx = (struct display_context_t*)dev;
    if (ctx) 
    {
        if(ctx->mFD_disp)
        {
            close(ctx->mFD_disp);
        }

        if(ctx->mFD_mp)
        {
            close(ctx->mFD_mp);
        }

        for(i = 0;i < MAX_DISPLAY_NUM;i++)
        {
            if(ctx->mFD_fb[i])
            {
                close(ctx->mFD_fb[i]);
            }
        }
        
        free(ctx);
    }
    return 0;
}

static int open_display(const struct hw_module_t* module, const char* name,
        struct hw_device_t** device)
{
    int status = 0;
    display_context_t *ctx;
    
    ctx = (display_context_t *)malloc(sizeof(display_context_t));
    memset(ctx, 0, sizeof(*ctx));

    ctx->device.common.tag          	= HARDWARE_DEVICE_TAG;
    ctx->device.common.version      	= 1;
    ctx->device.common.module       	= const_cast<hw_module_t*>(module);
    ctx->device.common.close        	= close_display;
    ctx->device.opendisplay         	= display_opendev;
    ctx->device.closedisplay        	= display_closedev;
    ctx->device.copysrcfbtodstfb    	= display_copyfb;
    ctx->device.pandisplay          	= display_pandisplay;
    ctx->device.isenablemp          	= display_isenablemp;
    ctx->device.setdisplaymode      	= display_setmode;
    ctx->device.getdisplaymode			= display_getmode;
    ctx->device.changemode          	= display_setoutputtype;
    ctx->device.setdisplayparameter 	= display_setparameter;
    ctx->device.getdisplayparameter 	= display_getparameter;
    ctx->device.gethdmistatus       	= display_gethdmistatus;
    ctx->device.gettvdacstatus      	= display_gettvdacstatus;
    ctx->device.request_modelock    	= display_requestmodelock;
    ctx->device.release_modelock    	= display_releasemodelock;
    ctx->device.setmasterdisplay    	= display_setmasterdisplay;
    ctx->device.getmasterdisplay    	= display_getmasterdisplay;
    ctx->device.getdisplaybufid     	= display_getdisplaybufid;
    ctx->device.getmaxwidthdisplay  	= display_getmaxdisplayno;
    ctx->device.getdisplaycount  		= display_getdisplaycount;
    ctx->device.gethdmimaxmode			= display_gethdmimaxmode;
    ctx->device.issupporthdmimode		= display_issupporthdmimode;
    ctx->device.issupport3dmode	        = display_issupport3dmode;
    ctx->device.setdisplayareapercent	= display_setareapercent;
    ctx->device.getdisplayareapercent	= display_getareapercent;
    ctx->device.enabledisplaybacklight = display_enablebacklight;
	ctx->device.setdisplaybacklightmode = display_setbacklightmode;
	ctx->device.setdisplaybacklightdemomode = display_setbacklightdemomode;
	ctx->device.setdisplayenhancemode   = display_setenhancemode;
	ctx->device.getdisplayenhanceenable = display_getenhanceenable;
	ctx->device.setdisplayenhancedemomode= display_setenhancedemomode;
    ctx->device.setdisplaybrightness	= display_setbright;
    ctx->device.getdisplaybrightness	= display_getbright;
    ctx->device.setdisplaycontrast		= display_setcontrast;
    ctx->device.getdisplaycontrast		= display_getcontrast;
    ctx->device.setdisplaysaturation	= display_setsaturation;
    ctx->device.getdisplaysaturation	= display_getsaturation;
    ctx->device.setOrientation          = display_setorientation;
    ctx->device.requestdispbuf			= display_requestbuf;
    ctx->device.releasedispbuf			= display_releasebuf;
    ctx->device.convertfb				= display_convertfb;
    ctx->device.getdispbufaddr			= display_getdispbufaddr;
    
    display_init(ctx);

    if (status == 0) 
    {
        *device = &ctx->device.common;
    } 
    else 
    {
        close_display(&ctx->device.common);
    }

    if(mutex_inited == false)
    {
        pthread_mutex_init(&mode_lock, NULL);

		//display_globalinit(NULL);
		
        mutex_inited = true;
    }
    
    return status;
}


/*
 * Copyright (C) 2010 The Android Open Source Project
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

//#define LOG_NDEBUG 0

#include <hardware/hardware.h>

#include <fcntl.h>
#include <errno.h>

#include <cutils/log.h>
#include <cutils/atomic.h>

#include <hardware/hwcomposer.h>
#include <drv_display.h>
#include <fb.h>
#include <EGL/egl.h>
#include <hardware_legacy/uevent.h>
#include <sys/resource.h>
#include <stdlib.h>
#include <linux/netlink.h>
#include <sys/socket.h>
#include <poll.h>
#include "hwccomposer_priv.h"
#include <utils/Timers.h>

#define STATUS_LOG ALOGV
#define RECT_LOG ALOGV


/*****************************************************************************/
static int hwc_device_open(const struct hw_module_t* module, const char* name,
        struct hw_device_t** device);

static int hwc_set_mode(sun4i_hwc_context_t *ctx, e_hwc_mode_t mode);

static struct hw_module_methods_t hwc_module_methods = 
{
    open: hwc_device_open
};

hwc_module_t HAL_MODULE_INFO_SYM = 
{
    common: 
    {
        tag: HARDWARE_MODULE_TAG,
        version_major: 1,
        version_minor: 0,
        id: HWC_HARDWARE_MODULE_ID,
        name: "Sample hwcomposer module",
        author: "The Android Open Source Project",
        methods: &hwc_module_methods,
    }
};

static int hwc_video_open(sun4i_hwc_context_t *ctx, int screen_idx)
{
	unsigned long args[4]={0};

    ALOGD("#%s screen_idx:%d\n", __FUNCTION__, screen_idx);

    if(ctx->video_layerhdl[screen_idx])
    {        
        args[0]                         = screen_idx;
        args[1]                         = ctx->video_layerhdl[screen_idx];
        ioctl(ctx->dispfd, DISP_CMD_LAYER_OPEN,args);

        ctx->status[screen_idx] |= HWC_STATUS_OPENED;
        STATUS_LOG("#status[%d]=%d in %s", screen_idx, ctx->status[screen_idx], __FUNCTION__);
    }
    else
    {
        ALOGD("####video layer handle:%d in %s", ctx->video_layerhdl[screen_idx], __FUNCTION__);
    }

	return 0;
}

static int hwc_video_close(sun4i_hwc_context_t *ctx, int screen_idx)
{
	unsigned long args[4]={0};

    ALOGD("#%s screen_idx:%d\n", __FUNCTION__, screen_idx);

    if(ctx->video_layerhdl[screen_idx])
    {
        args[0]                         = screen_idx;
        args[1]                         = ctx->video_layerhdl[screen_idx];
        ioctl(ctx->dispfd, DISP_CMD_LAYER_CLOSE,args);

        ctx->status[screen_idx] &= (~HWC_STATUS_OPENED);
        STATUS_LOG("#status[%d]=%d in %s", screen_idx, ctx->status[screen_idx], __FUNCTION__);
    }
    else
    {
        ALOGD("####video layer handle:%d in %s", ctx->video_layerhdl[screen_idx], __FUNCTION__);
    }

	return 0;
}

static int hwc_video_request(sun4i_hwc_context_t *ctx, int screen_idx, layerinitpara_t *layer_init_para)
{
    __disp_layer_info_t         layer_info;
    unsigned long               args[4]={0};
    int                         ret;

    ALOGD("#%s screen_idx:%d\n", __FUNCTION__, screen_idx);

    args[0] = 0;
    args[1] = 2;
    ioctl(ctx->dispfd, DISP_CMD_SET_OVL_MODE, args);
    ctx->force_sgx |= HWC_FORCE_SGX_REASON_VIDEO;

    if(screen_idx == 0)
    {
        ioctl(ctx->mFD_fb[0], FBIOGET_LAYER_HDL_0, &ctx->ui_layerhdl[0]);
    }
    else
    {
        ctx->ui_layerhdl[1] = ctx->screen_para.ui_hdl[1];
    }

    if(ctx->video_layerhdl[screen_idx])
    {
        //should not be here, something wrong????
        args[0] = screen_idx;
        args[1] = ctx->video_layerhdl[screen_idx];
        ctx->video_layerhdl[screen_idx] = (uint32_t)ioctl(ctx->dispfd, DISP_CMD_LAYER_RELEASE,args);
        ctx->video_layerhdl[screen_idx] = 0;
    }
    
    args[0] = screen_idx;
    ctx->video_layerhdl[screen_idx] = (uint32_t)ioctl(ctx->dispfd, DISP_CMD_LAYER_REQUEST,args);
    if(ctx->video_layerhdl[screen_idx] == 0)
    {
        ALOGD("####request layer failed in %s!\n", __FUNCTION__);
        return -1;
    }

    memset(&layer_info, 0, sizeof(__disp_layer_info_t));
    if (layer_init_para->h < 720)
    {
        layer_info.fb.cs_mode = DISP_BT601;
    }
    else
    {
        layer_info.fb.cs_mode = DISP_BT709;
    }
    switch(layer_init_para->format)
    {
        case HWC_FORMAT_MBYUV420:
            layer_info.fb.format = DISP_FORMAT_YUV420;
            layer_info.fb.mode = DISP_MOD_MB_UV_COMBINED;
            layer_info.fb.seq = DISP_SEQ_UVUV;
            break;
        case HWC_FORMAT_MBYUV422:
            layer_info.fb.format = DISP_FORMAT_YUV422;
            layer_info.fb.mode = DISP_MOD_MB_UV_COMBINED;
            layer_info.fb.seq = DISP_SEQ_UVUV;
            break;
        case HWC_FORMAT_YUV420PLANAR:
            layer_info.fb.format = DISP_FORMAT_YUV420;
            layer_info.fb.mode = DISP_MOD_NON_MB_PLANAR;
            layer_info.fb.seq = DISP_SEQ_P3210;
            break;
        case HWC_FORMAT_RGBA_8888:
            layer_info.fb.format = DISP_FORMAT_ARGB8888;
            layer_info.fb.mode = DISP_MOD_NON_MB_PLANAR;
            layer_info.fb.seq = DISP_SEQ_P3210;
            break;
        case HWC_FORMAT_DEFAULT:
        default:
            layer_info.fb.format = DISP_FORMAT_YUV420;
            layer_info.fb.mode = DISP_MOD_NON_MB_UV_COMBINED;
            layer_info.fb.seq = DISP_SEQ_UVUV;
            break;
    }
    layer_info.fb.br_swap         = 0;
    layer_info.fb.addr[0]         = 0;
    layer_info.fb.addr[1]         = 0;
    layer_info.fb.addr[2]         = 0;
    layer_info.fb.size.width      = layer_init_para->w;
    layer_info.fb.size.height     = layer_init_para->h;
    layer_info.mode               = DISP_LAYER_WORK_MODE_SCALER;
    layer_info.alpha_en           = 1;
    layer_info.alpha_val          = 0xff;
    layer_info.pipe               = 1;
    layer_info.src_win.x          = 0;
    layer_info.src_win.y          = 0;
    layer_info.src_win.width      = 1;
    layer_info.src_win.height     = 1;
    layer_info.scn_win.x          = 0;
    layer_info.scn_win.y          = 0;
    layer_info.scn_win.width      = 1;
    layer_info.scn_win.height     = 1;

    args[0]                         = screen_idx;
    args[1]                         = ctx->video_layerhdl[screen_idx];
    args[2]                         = (unsigned long) (&layer_info);
    args[3]                         = 0;
    ioctl(ctx->dispfd, DISP_CMD_LAYER_SET_PARA, args);

    args[0]                         = screen_idx;
    args[1]                         = ctx->video_layerhdl[screen_idx];
    ioctl(ctx->dispfd, DISP_CMD_LAYER_BOTTOM, args);
    
    args[0]                         = screen_idx;
    args[1]                         = ctx->ui_layerhdl[screen_idx];
    args[2]                         = (unsigned long) (&layer_info);
    args[3]                         = 0;
    ret = ioctl(ctx->dispfd, DISP_CMD_LAYER_GET_PARA, args);
    if(ret < 0)
    {
        ALOGD("####DISP_CMD_LAYER_GET_PARA fail in %s, screen_idx:%d,hdl:%d\n", __FUNCTION__, screen_idx, ctx->ui_layerhdl[screen_idx]);
    }

    args[0]                         = screen_idx;
    args[1]                         = ctx->ui_layerhdl[screen_idx];
    ioctl(ctx->dispfd, DISP_CMD_LAYER_ALPHA_OFF, args);    
    
    args[0]                         = screen_idx;
    args[1]                         = ctx->video_layerhdl[screen_idx];
    ret = ioctl(ctx->dispfd, DISP_CMD_VIDEO_START, args);

    ctx->w = layer_init_para->w;
    ctx->h = layer_init_para->h;
    ctx->format = layer_init_para->format;
    ctx->cur_3d_format = (e_hwc_format_t)ctx->format;
    ctx->cur_3d_w = ctx->w;
    ctx->cur_3d_h = ctx->h;
    ctx->cur_3d_src = HWC_3D_SRC_MODE_NORMAL;
    ctx->cur_3d_out = HWC_3D_OUT_MODE_ORIGINAL;
    
    return 0;
}

static int hwc_video_release(sun4i_hwc_context_t *ctx, int screen_idx)
{
	unsigned long args[4]={0};
    __disp_output_type_t        cur_out_type;
    __disp_tv_mode_t            cur_hdmi_mode;

    ALOGD("#hwc_video_release screen_idx:%d,mode:%d\n", screen_idx,ctx->mode);

    if(ctx->video_layerhdl[screen_idx])
    {
        args[0]                         = screen_idx;
        args[1]                         = ctx->video_layerhdl[screen_idx];
        ioctl(ctx->dispfd, DISP_CMD_LAYER_CLOSE,args);

        args[0]                         = screen_idx;
        args[1]                         = ctx->video_layerhdl[screen_idx];
        ioctl(ctx->dispfd, DISP_CMD_VIDEO_STOP, args);

        usleep(20 * 1000);

        args[0]                         = screen_idx;
        args[1]                         = ctx->video_layerhdl[screen_idx];
        ioctl(ctx->dispfd, DISP_CMD_LAYER_RELEASE, args);
        ctx->status[0] &= HWC_STATUS_HAVE_VIDEO;
        ctx->status[1] &= HWC_STATUS_HAVE_VIDEO;
        STATUS_LOG("#status[0]=%d in %s", ctx->status[screen_idx], __FUNCTION__);
        STATUS_LOG("#status[1]=%d in %s", ctx->status[screen_idx], __FUNCTION__);

		args[0] 						= screen_idx;
		args[1] 						= ctx->ui_layerhdl[screen_idx];
		args[2] 						=0xff;
		ioctl(ctx->dispfd, DISP_CMD_LAYER_SET_ALPHA_VALUE, args);    

		args[0] 						= screen_idx;
		args[1] 						= ctx->ui_layerhdl[screen_idx];
		ioctl(ctx->dispfd, DISP_CMD_LAYER_ALPHA_ON, args);    

        args[0] = screen_idx;
        cur_out_type = (__disp_output_type_t)ioctl(ctx->dispfd,DISP_CMD_GET_OUTPUT_TYPE,(unsigned long)args);
        if(cur_out_type == DISP_OUTPUT_TYPE_HDMI)
        {
            args[0] = screen_idx;
            cur_hdmi_mode = (__disp_tv_mode_t)ioctl(ctx->dispfd,DISP_CMD_HDMI_GET_MODE,(unsigned long)args);
            if(cur_hdmi_mode == DISP_TV_MOD_1080P_24HZ_3D_FP || cur_hdmi_mode == DISP_TV_MOD_720P_50HZ_3D_FP || cur_hdmi_mode == DISP_TV_MOD_720P_60HZ_3D_FP)
            {
                __disp_layer_info_t         ui_layer_info;
                int ret = 0;
                
                args[0]                         = screen_idx;
                args[1]                         = ctx->ui_layerhdl[screen_idx];
                args[2]                         = (unsigned long) (&ui_layer_info);
                ret = ioctl(ctx->dispfd, DISP_CMD_LAYER_GET_PARA, args);
                if(ret < 0)
                {
                    ALOGD("####DISP_CMD_LAYER_GET_PARA fail in %s, screen_idx:%d,hdl:%d\n", __FUNCTION__, screen_idx, ctx->ui_layerhdl[screen_idx]);
                }
                
                ui_layer_info.scn_win.x = ctx->org_scn_win.x;
                ui_layer_info.scn_win.y = ctx->org_scn_win.y;
                ui_layer_info.scn_win.width = ctx->org_scn_win.width;
                ui_layer_info.scn_win.height = ctx->org_scn_win.height;
                
                args[0]                         = screen_idx;
                args[1]                         = ctx->ui_layerhdl[screen_idx];
                args[2]                         = (unsigned long) (&ui_layer_info);
                ioctl(ctx->dispfd, DISP_CMD_LAYER_SET_PARA, args);
                
                args[0] = screen_idx;
                ioctl(ctx->dispfd,DISP_CMD_HDMI_OFF,(unsigned long)args);
    
                args[0] = screen_idx;
                args[1] = ctx->org_hdmi_mode;
                ioctl(ctx->dispfd,DISP_CMD_HDMI_SET_MODE,(unsigned long)args);
                
                args[0] = screen_idx;
                ioctl(ctx->dispfd,DISP_CMD_HDMI_ON,(unsigned long)args);
            }
        }

        ctx->video_layerhdl[screen_idx] = 0;
    }

    args[0]                         = screen_idx;
    args[1]                         = ctx->ui_layerhdl[screen_idx];
    ioctl(ctx->dispfd, DISP_CMD_LAYER_ALPHA_ON, args);
    
    memset(&ctx->rect_in, 0, sizeof(hwc_rect_t));
    memset(&ctx->rect_out, 0, sizeof(hwc_rect_t));
    ctx->cur_3denable = 0;

    args[0] = 0;
    args[1] = 1;
    ioctl(ctx->dispfd, DISP_CMD_SET_OVL_MODE, args);
    ctx->force_sgx &= (~HWC_FORCE_SGX_REASON_VIDEO);
    
	return 0;
}


static int hwc_video_3d_mode(sun4i_hwc_context_t *ctx, int screen_idx, video3Dinfo_t *_3d_info)
{
    __disp_layer_info_t         layer_info;
    __disp_output_type_t        cur_out_type;
    __disp_tv_mode_t            cur_hdmi_mode;
    unsigned long               args[4]={0};
    int                         ret = -1;

    ALOGD("#%s, screen_idx:%d, src:%d, out:%d, w:%d, h:%d, format:0x%x\n", 
        __FUNCTION__, screen_idx, _3d_info->src_mode, _3d_info->display_mode, _3d_info->width, _3d_info->height, _3d_info->format);

    args[0] = screen_idx;
    cur_out_type = (__disp_output_type_t)ioctl(ctx->dispfd,DISP_CMD_GET_OUTPUT_TYPE,(unsigned long)args);
    if(cur_out_type == DISP_OUTPUT_TYPE_HDMI)
    {
        args[0] = screen_idx;
        cur_hdmi_mode = (__disp_tv_mode_t)ioctl(ctx->dispfd,DISP_CMD_HDMI_GET_MODE,(unsigned long)args);

        if(cur_hdmi_mode == DISP_TV_MOD_1080P_24HZ_3D_FP || cur_hdmi_mode == DISP_TV_MOD_720P_50HZ_3D_FP || cur_hdmi_mode == DISP_TV_MOD_720P_60HZ_3D_FP)
        {
            if((_3d_info->display_mode != HWC_3D_OUT_MODE_HDMI_3D_1080P24_FP) && (_3d_info->display_mode != HWC_3D_OUT_MODE_HDMI_3D_720P50_FP) 
                && (_3d_info->display_mode != HWC_3D_OUT_MODE_HDMI_3D_720P60_FP))
            {
                // 3d to 2d
                __disp_layer_info_t         ui_layer_info;
                
                args[0]                         = screen_idx;
                args[1]                         = ctx->ui_layerhdl[screen_idx];
                args[2]                         = (unsigned long) (&ui_layer_info);
                ret = ioctl(ctx->dispfd, DISP_CMD_LAYER_GET_PARA, args);
                if(ret < 0)
                {
                    ALOGD("####DISP_CMD_LAYER_GET_PARA fail in %s, screen_idx:%d,hdl:%d\n", __FUNCTION__, screen_idx, ctx->ui_layerhdl[screen_idx]);
                }

                ui_layer_info.scn_win.x = ctx->org_scn_win.x;
                ui_layer_info.scn_win.y = ctx->org_scn_win.y;
                ui_layer_info.scn_win.width = ctx->org_scn_win.width;
                ui_layer_info.scn_win.height = ctx->org_scn_win.height;
                
                args[0]                         = screen_idx;
                args[1]                         = ctx->ui_layerhdl[screen_idx];
                args[2]                         = (unsigned long) (&ui_layer_info);
                ret = ioctl(ctx->dispfd, DISP_CMD_LAYER_SET_PARA, args);
                
                args[0] = screen_idx;
                ret = ioctl(ctx->dispfd,DISP_CMD_HDMI_OFF,(unsigned long)args);

                args[0] = screen_idx;
                args[1] = ctx->org_hdmi_mode;
                ioctl(ctx->dispfd,DISP_CMD_HDMI_SET_MODE,(unsigned long)args);
                
                args[0] = screen_idx;
                ret = ioctl(ctx->dispfd,DISP_CMD_HDMI_ON,(unsigned long)args);
            }
        }
        else
        {
            if(_3d_info->display_mode == HWC_3D_OUT_MODE_HDMI_3D_1080P24_FP || _3d_info->display_mode == HWC_3D_OUT_MODE_HDMI_3D_720P50_FP 
                || _3d_info->display_mode == HWC_3D_OUT_MODE_HDMI_3D_720P60_FP)
            {
                // 2d to 3d
                ctx->org_hdmi_mode = cur_hdmi_mode;
                
                __disp_layer_info_t 		ui_layer_info;
                
                args[0]                         = screen_idx;
                args[1]                         = ctx->ui_layerhdl[screen_idx];
                args[2]                         = (unsigned long) (&ui_layer_info);
                ret = ioctl(ctx->dispfd, DISP_CMD_LAYER_GET_PARA, args);
                if(ret < 0)
                {
                    ALOGD("####DISP_CMD_LAYER_GET_PARA fail in %s, screen_idx:%d,hdl:%d\n", __FUNCTION__, screen_idx, ctx->ui_layerhdl[screen_idx]);
                }

                ctx->org_scn_win.x = ui_layer_info.scn_win.x;
                ctx->org_scn_win.y = ui_layer_info.scn_win.y;
                ctx->org_scn_win.width = ui_layer_info.scn_win.width;
                ctx->org_scn_win.height = ui_layer_info.scn_win.height;
                
                ui_layer_info.scn_win.x = 0;
                ui_layer_info.scn_win.y = 0;
                if(_3d_info->display_mode == HWC_3D_OUT_MODE_HDMI_3D_1080P24_FP)
                {
                    ui_layer_info.scn_win.width = 1920;
                    ui_layer_info.scn_win.height = 1080;
                }
                else
                {
                    ui_layer_info.scn_win.width = 1280;
                    ui_layer_info.scn_win.height = 720;
                }
                args[0]                         = screen_idx;
                args[1]                         = ctx->ui_layerhdl[screen_idx];
                args[2]                         = (unsigned long) (&ui_layer_info);
                ret = ioctl(ctx->dispfd, DISP_CMD_LAYER_SET_PARA, args);
                
                args[0] = screen_idx;
                ret = ioctl(ctx->dispfd,DISP_CMD_HDMI_OFF,(unsigned long)args);
                
                args[0] = screen_idx;
                if(_3d_info->display_mode == HWC_3D_OUT_MODE_HDMI_3D_1080P24_FP)
                {
                    args[1] = DISP_TV_MOD_1080P_24HZ_3D_FP;
                }
                else if(_3d_info->display_mode == HWC_3D_OUT_MODE_HDMI_3D_720P50_FP)
                {
                    args[1] = DISP_TV_MOD_720P_50HZ_3D_FP;
                }
                else if(_3d_info->display_mode == HWC_3D_OUT_MODE_HDMI_3D_720P60_FP)
                {
                    args[1] = DISP_TV_MOD_720P_60HZ_3D_FP;
                }
                ioctl(ctx->dispfd,DISP_CMD_HDMI_SET_MODE,(unsigned long)args);
                
                args[0] = screen_idx;
                ret = ioctl(ctx->dispfd,DISP_CMD_HDMI_ON,(unsigned long)args);
            }
        }
    }

    if(ctx->video_layerhdl[screen_idx])
    {
        args[0] = screen_idx;
        args[1] = ctx->video_layerhdl[screen_idx];
        args[2] = (unsigned long)&layer_info;
        ret = ioctl(ctx->dispfd, DISP_CMD_LAYER_GET_PARA, args);
        if(ret < 0)
        {
            ALOGD("####DISP_CMD_LAYER_GET_PARA fail in %s, screen_idx:%d,hdl:%d\n",__FUNCTION__, screen_idx, ctx->video_layerhdl[screen_idx]);
        }

        layer_info.fb.size.width = _3d_info->width;
        layer_info.fb.size.height = _3d_info->height;
        layer_info.src_win.x = 0;
        layer_info.src_win.y = 0;
        layer_info.src_win.width = _3d_info->width;
        layer_info.src_win.height = _3d_info->height;
        
        if(_3d_info->format == HWC_FORMAT_RGBA_8888)//·ÖÉ«
        {
            layer_info.fb.mode = DISP_MOD_NON_MB_PLANAR;
            layer_info.fb.format = DISP_FORMAT_ARGB8888;
            layer_info.fb.seq = DISP_SEQ_P3210;
        }
        else if(_3d_info->format == HWC_FORMAT_YUV420PLANAR)
        {
            layer_info.fb.mode = DISP_MOD_NON_MB_PLANAR;
            layer_info.fb.format = DISP_FORMAT_YUV420;
            layer_info.fb.seq = DISP_SEQ_P3210;
        }
        else if(_3d_info->format == HWC_FORMAT_MBYUV422)
        {
            layer_info.fb.mode = DISP_MOD_MB_UV_COMBINED;
            layer_info.fb.format = DISP_FORMAT_YUV422;
            layer_info.fb.seq = DISP_SEQ_UVUV;
        }
        else
        {
            layer_info.fb.mode = DISP_MOD_MB_UV_COMBINED;
            layer_info.fb.format = DISP_FORMAT_YUV420;
            layer_info.fb.seq = DISP_SEQ_UVUV;
        }

        if(_3d_info->src_mode == HWC_3D_SRC_MODE_NORMAL)
        {
            layer_info.fb.b_trd_src = 0;
        }
        else
        {
            layer_info.fb.b_trd_src = 1;
            layer_info.fb.trd_mode = (__disp_3d_src_mode_t)_3d_info->src_mode;
            layer_info.fb.trd_right_addr[0] = layer_info.fb.addr[0];
            layer_info.fb.trd_right_addr[1] = layer_info.fb.addr[1];
            layer_info.fb.trd_right_addr[2] = layer_info.fb.addr[2];
        }
        if(_3d_info->display_mode == HWC_3D_OUT_MODE_HDMI_3D_1080P24_FP || _3d_info->display_mode == HWC_3D_OUT_MODE_HDMI_3D_720P50_FP 
            || _3d_info->display_mode == HWC_3D_OUT_MODE_HDMI_3D_720P60_FP)
        {
            if(cur_out_type == DISP_OUTPUT_TYPE_HDMI)
            {
                layer_info.b_trd_out = 1;
                layer_info.out_trd_mode = DISP_3D_OUT_MODE_FP;
            }
            else
            {
                layer_info.fb.b_trd_src = 0;
                layer_info.b_trd_out = 0;
            }
        }
        else if(_3d_info->display_mode == HWC_3D_OUT_MODE_ORIGINAL || _3d_info->display_mode == HWC_3D_OUT_MODE_ANAGLAGH)
        {
            layer_info.fb.b_trd_src = 0;
            layer_info.b_trd_out = 0;
        }
        else if(_3d_info->display_mode == HWC_3D_OUT_MODE_LI || _3d_info->display_mode == HWC_3D_OUT_MODE_CI_1 || _3d_info->display_mode == HWC_3D_OUT_MODE_CI_2 || 
                _3d_info->display_mode == HWC_3D_OUT_MODE_CI_3 || _3d_info->display_mode == HWC_3D_OUT_MODE_CI_4)
        {
            layer_info.b_trd_out = 1;
            layer_info.out_trd_mode = (__disp_3d_out_mode_t)_3d_info->display_mode;
        }
        else
        {
            layer_info.b_trd_out = 0;
        }

        if(layer_info.b_trd_out)
        {
            unsigned int w,h;
            
            args[0] = screen_idx;
            w = ioctl(ctx->dispfd,DISP_CMD_SCN_GET_WIDTH,(unsigned long)args);
            h = ioctl(ctx->dispfd,DISP_CMD_SCN_GET_HEIGHT,(unsigned long)args);

            layer_info.scn_win.x = 0;
            layer_info.scn_win.y = 0;
            layer_info.scn_win.width = w;
            layer_info.scn_win.height = h;

            ctx->rect_out.left = 0;
            ctx->rect_out.top = 0;
            ctx->rect_out.right = w;
            ctx->rect_out.bottom = h;
        }
        else
        {
            layer_info.scn_win.x = ctx->rect_out_active[screen_idx].left;
            layer_info.scn_win.y = ctx->rect_out_active[screen_idx].top;
            layer_info.scn_win.width = ctx->rect_out_active[screen_idx].right - ctx->rect_out_active[screen_idx].left;
            layer_info.scn_win.height = ctx->rect_out_active[screen_idx].bottom - ctx->rect_out_active[screen_idx].top;
        }
        
        ALOGV("b_3d_src:%d, 3d_src_mode:%d, b_3d_out:%d, 3d_out_mode:%d", layer_info.fb.b_trd_src, layer_info.fb.trd_mode, layer_info.b_trd_out, layer_info.out_trd_mode);
        args[0] = screen_idx;
        args[1] = ctx->video_layerhdl[screen_idx];
        args[2] = (unsigned long)&layer_info;
        ioctl(ctx->dispfd, DISP_CMD_LAYER_SET_PARA, args);
    }
    else
    {
        ALOGD("####have not video in %s", __FUNCTION__);
    }

    ctx->cur_3d_w = _3d_info->width;
    ctx->cur_3d_h = _3d_info->height;
    ctx->cur_3d_format = _3d_info->format;
    ctx->cur_3d_src = _3d_info->src_mode;
    ctx->cur_3d_out = _3d_info->display_mode;
    ctx->cur_3denable = layer_info.b_trd_out;
    return 0;
}


static int hwc_video_set_frame_para(sun4i_hwc_context_t *ctx, int screen_idx, libhwclayerpara_t *overlaypara)
{
    __disp_video_fb_t      		video_info;
    unsigned long               args[4]={0};
    int                         ret;
        
	video_info.interlace         = (overlaypara->bProgressiveSrc?0:1);
	video_info.top_field_first   = overlaypara->bTopFieldFirst;
	video_info.addr[0]           = overlaypara->addr[0];
	video_info.addr[1]           = overlaypara->addr[1];
	video_info.addr[2]			 = overlaypara->addr[2];
	video_info.addr_right[0]     = overlaypara->addr_3d_right[0];
	video_info.addr_right[1]     = overlaypara->addr_3d_right[1];
	video_info.addr_right[2]	 = overlaypara->addr_3d_right[2];
	video_info.id                = overlaypara->number; 
	video_info.maf_valid         = overlaypara->maf_valid;
	video_info.pre_frame_valid   = overlaypara->pre_frame_valid;
	video_info.flag_addr         = overlaypara->flag_addr;
	video_info.flag_stride       = overlaypara->flag_stride;

	if((ctx->status[screen_idx] & HWC_STATUS_HAVE_FRAME) == 0)
	{
        __disp_layer_info_t         layer_info;

    	ALOGD("#first_frame ............");

    	args[0] 				= screen_idx;
    	args[1] 				= ctx->video_layerhdl[screen_idx];
    	args[2] 				= (unsigned long) (&layer_info);
    	args[3] 				= 0;
    	ret = ioctl(ctx->dispfd, DISP_CMD_LAYER_GET_PARA, args);
        if(ret < 0)
        {
            ALOGD("####DISP_CMD_LAYER_GET_PARA fail in %s, screen_idx:%d, hdl:%d\n",__FUNCTION__, screen_idx, ctx->video_layerhdl[screen_idx]);
        }

    	layer_info.fb.addr[0] 	= video_info.addr[0];
    	layer_info.fb.addr[1] 	= video_info.addr[1];
    	layer_info.fb.addr[2] 	= video_info.addr[2];
    	layer_info.fb.trd_right_addr[0] = video_info.addr_right[0];
    	layer_info.fb.trd_right_addr[1] = video_info.addr_right[1];
    	layer_info.fb.trd_right_addr[2] = video_info.addr_right[2];
    	args[0] 				= screen_idx;
    	args[1] 				= ctx->video_layerhdl[screen_idx];
    	args[2] 				= (unsigned long) (&layer_info);
    	args[3] 				= 0;
    	ret = ioctl(ctx->dispfd, DISP_CMD_LAYER_SET_PARA, args);

        ctx->status[screen_idx] |= HWC_STATUS_HAVE_FRAME;
        STATUS_LOG("#status[%d]=%d in %s", screen_idx, ctx->status[screen_idx], __FUNCTION__);
	}            

	args[0]					= screen_idx;
    args[1]                 = ctx->video_layerhdl[screen_idx];
	args[2]                 = (unsigned long)(&video_info);
	args[3]                 = 0;
	ret = ioctl(ctx->dispfd, DISP_CMD_VIDEO_SET_FB,args);

    memcpy(&ctx->cur_frame_para, overlaypara,sizeof(libhwclayerpara_t));

    
    return 0;
}

static int hwc_computer_rect(sun4i_hwc_context_t *ctx, int screen_idx, hwc_rect_t *rect_out, hwc_rect_t *rect_in)
{
    int		        ret;
    unsigned long   args[4]={0};
    unsigned int    screen_in_width;
    unsigned int    screen_in_height;
    unsigned int    temp_x,temp_y,temp_w,temp_h;
    int             x,y,w,h,mid_x,mid_y;
    int             force_full_screen = 0;

    if(rect_in->left >= rect_in->right || rect_in->top >= rect_in->bottom)
    {
        ALOGV("para error in hwc_computer_rect,(left:%d,right:%d,top:%d,bottom:%d)\n", rect_in->left, rect_in->right, rect_in->top, rect_in->bottom);
        return -1;
    }

    screen_in_width = ctx->screen_para.app_width[0];
    screen_in_height = ctx->screen_para.app_height[0];

    if(ctx->rotation_video)
    {
        screen_in_width = ctx->screen_para.app_height[0];
        screen_in_height = ctx->screen_para.app_width[0];
    }

    RECT_LOG("#1.0 in:[%d,%d,%d,%d] [%d,%d]\n", 
        rect_in->left, rect_in->top, rect_in->right - rect_in->left, rect_in->bottom-rect_in->top,screen_in_width, screen_in_height);

    
    if(ctx->cur_3denable == true)
    {
    	rect_out->left = 0;
    	rect_out->right = ctx->screen_para.width[screen_idx];
    	rect_out->top = 0;
    	rect_out->bottom = ctx->screen_para.height[screen_idx];
    	
    	return 0;
    }

    temp_x = rect_in->left;
    temp_w = rect_in->right - rect_in->left;
    temp_y = rect_in->top;
    temp_h = rect_in->bottom - rect_in->top;

	mid_x = temp_x + temp_w/2;
	mid_y = temp_y + temp_h/2;

	mid_x = mid_x * ctx->screen_para.valid_width[screen_idx] / screen_in_width;
	mid_y = mid_y * ctx->screen_para.valid_height[screen_idx] / screen_in_height;

    if(ctx->b_video_in_valid_area)
    {
        force_full_screen = 0;
    }
    else
    {
        if((rect_in->left==0 && rect_in->right==(screen_in_width-1)) || (rect_in->top==0 && rect_in->bottom==(screen_in_height-1)))
        {
            force_full_screen = 1;
        }
        else
        {
            force_full_screen = 0;
        }
    }

    if(force_full_screen)
    {
    	if((screen_in_width == ctx->screen_para.width[screen_idx]) && (screen_in_height == ctx->screen_para.height[screen_idx]))
    	{
    	    rect_out->left = rect_in->left;
    	    rect_out->right = rect_in->right;
    	    rect_out->top = rect_in->top;
    	    rect_out->bottom = rect_in->bottom;
    	    return 0;
    	}
    
   	 	mid_x += (ctx->screen_para.width[screen_idx] - ctx->screen_para.valid_width[screen_idx])/2;
   	 	mid_y += (ctx->screen_para.height[screen_idx] - ctx->screen_para.valid_height[screen_idx])/2;
		
    	if(mid_x * temp_h >= mid_y * temp_w)
    	{
    	    y = 0;
    	    h = mid_y * 2;
    	    w = h * temp_w / temp_h;
    	    x = mid_x - (w/2);
    	}
    	else
    	{
    	    x = 0;
    	    w = mid_x * 2;
    	    h = w * temp_h / temp_w;
    	    y = mid_y - (h/2);
    	}
	}
    else
    {
        w = temp_w * ctx->screen_para.valid_width[screen_idx] / screen_in_width;
        h = temp_h * ctx->screen_para.valid_height[screen_idx] / screen_in_height;
        x = mid_x - (w/2);
        y = mid_y - (h/2);
        x += (ctx->screen_para.width[screen_idx] - ctx->screen_para.valid_width[screen_idx])/2;
        y += (ctx->screen_para.height[screen_idx] - ctx->screen_para.valid_height[screen_idx])/2;
    }
	
	temp_x = x;
	temp_y = y;
	temp_w = w;
	temp_h = h;

    rect_out->left = temp_x;
    rect_out->right = temp_x + temp_w;
    rect_out->top = temp_y;
    rect_out->bottom = temp_y + temp_h;

	RECT_LOG("#1.1 out:[%d,%d,%d,%d] [%d,%d] [%d,%d]\n", temp_x, temp_y, temp_w, temp_h,
	    ctx->screen_para.width[screen_idx], ctx->screen_para.height[screen_idx], ctx->screen_para.valid_width[screen_idx], ctx->screen_para.valid_height[screen_idx]);

    return 0;
}

static int hwc_set_rect(hwc_composer_device_t *dev,hwc_layer_list_t* list)
{
    int 						ret = 0;
    sun4i_hwc_context_t   		*ctx = (sun4i_hwc_context_t *)dev;
    unsigned long               args[4]={0};
    int screen_idx;
    
    ctx->have_video_overlay = 0;
    
	for (size_t i=0 ; i<list->numHwLayers ; i++)         
    {       
        if((list->hwLayers[i].format == HWC_FORMAT_MBYUV420)
            ||(list->hwLayers[i].format == HWC_FORMAT_MBYUV422)
            ||(list->hwLayers[i].format == HWC_FORMAT_YUV420PLANAR)
            ||(list->hwLayers[i].format == HWC_FORMAT_DEFAULT))
        {
            ALOGV("#hwc_set_rect\n");
            
            hwc_rect_t croprect;
            hwc_rect_t displayframe_src, displayframe_dst;
            __disp_layer_info_t         layer_info;

            memcpy(&croprect, &list->hwLayers[i].sourceCrop, sizeof(hwc_rect_t));
            memcpy(&displayframe_src, &list->hwLayers[i].displayFrame, sizeof(hwc_rect_t));

            if(ctx->rotation_video)
            {
                displayframe_src.left = list->hwLayers[i].displayFrame.top;
                displayframe_src.top = list->hwLayers[i].displayFrame.left;
                displayframe_src.right = list->hwLayers[i].displayFrame.bottom;
                displayframe_src.bottom = list->hwLayers[i].displayFrame.right;
            }
            
            if(ctx->rect_in.left != croprect.left || ctx->rect_in.right != croprect.right 
             ||ctx->rect_in.top != croprect.top || ctx->rect_in.bottom != croprect.bottom
             ||ctx->rect_out.left != displayframe_src.left || ctx->rect_out.right != displayframe_src.right
             ||ctx->rect_out.top != displayframe_src.top || ctx->rect_out.bottom != displayframe_src.bottom)
            {
                for(screen_idx=0; screen_idx<2; screen_idx++)
                {
                    if(((screen_idx == 0) && (ctx->mode==HWC_MODE_SCREEN0 || ctx->mode==HWC_MODE_SCREEN0_AND_SCREEN1 || ctx->mode==HWC_MODE_SCREEN0_GPU))
                    || ((screen_idx == 1) && (ctx->mode==HWC_MODE_SCREEN1 || ctx->mode==HWC_MODE_SCREEN0_TO_SCREEN1 || ctx->mode==HWC_MODE_SCREEN0_AND_SCREEN1)))
                    {
                        if((!(ctx->cur_3denable)) && ctx->video_layerhdl[screen_idx])
                        {
                            int screen_in_width;
                            int screen_in_height;

                            screen_in_width                     = ctx->screen_para.app_width[0];
                            screen_in_height                    = ctx->screen_para.app_height[0];
                            if(ctx->rotation_video)
                            {
                                screen_in_width = ctx->screen_para.app_height[0];
                                screen_in_height = ctx->screen_para.app_width[0];
                            }

                            RECT_LOG("#0:src[%d,%d,%d,%d], dst[%d,%d,%d,%d]\n",
                                croprect.left, croprect.top, croprect.right-croprect.left, croprect.bottom-croprect.top,
                                displayframe_src.left, displayframe_src.top, displayframe_src.right-displayframe_src.left, displayframe_src.bottom-displayframe_src.top);
                            if(croprect.left < 0)
                            {
                            	croprect.left = 0;
                            }
                            if(croprect.right > (int)ctx->w)
                            {
                            	croprect.right = ctx->w;
                            }
                            if(croprect.top< 0)
                            {
                            	croprect.top= 0;
                            }
                            if(croprect.bottom > (int)ctx->h)
                            {
                            	croprect.bottom = ctx->h;
                            }
                            if(displayframe_src.left < 0)
                            {
                                croprect.left = croprect.left + ((0 - displayframe_src.left) * (croprect.right - croprect.left) / (displayframe_src.right - displayframe_src.left));
                                displayframe_src.left = 0;
                            }
                            if(displayframe_src.right > screen_in_width)
                            {
                                croprect.right = croprect.right - ((displayframe_src.right - screen_in_width) * (croprect.right - croprect.left) / (displayframe_src.right - displayframe_src.left));
                                displayframe_src.right = screen_in_width;
                            }
                            if(displayframe_src.top< 0)
                            {
                                croprect.top= croprect.top+ ((0 - displayframe_src.top) * (croprect.bottom- croprect.top) / (displayframe_src.bottom- displayframe_src.top));
                                displayframe_src.top= 0;
                            }
                            if(displayframe_src.bottom> screen_in_height)
                            {
                                croprect.bottom= croprect.bottom- ((displayframe_src.bottom- screen_in_height) * (croprect.bottom- croprect.top) / (displayframe_src.bottom- displayframe_src.top));
                                displayframe_src.bottom= screen_in_height;
                            }

                            RECT_LOG("#1:src[%d,%d,%d,%d], dst[%d,%d,%d,%d]\n",
                                croprect.left, croprect.top, croprect.right-croprect.left, croprect.bottom-croprect.top,
                                displayframe_src.left, displayframe_src.top, displayframe_src.right-displayframe_src.left, displayframe_src.bottom-displayframe_src.top);

                            ret = hwc_computer_rect(ctx,screen_idx, &displayframe_dst, &displayframe_src);
                            if(ret < 0)
                            {
                                return -1;
                            }

                            RECT_LOG("#2:src[%d,%d,%d,%d], dst[%d,%d,%d,%d]\n",
                                croprect.left, croprect.top, croprect.right-croprect.left, croprect.bottom-croprect.top,
                                displayframe_dst.left, displayframe_dst.top, displayframe_dst.right-displayframe_dst.left, displayframe_dst.bottom-displayframe_dst.top);

                        	args[0] 				= screen_idx;
                        	args[1] 				= ctx->video_layerhdl[screen_idx];
                        	args[2] 				= (unsigned long) (&layer_info);
                        	args[3] 				= 0;
                        	ret = ioctl(ctx->dispfd, DISP_CMD_LAYER_GET_PARA, args);
                        	if(ret < 0)
                        	{
                        	    ALOGD("####DISP_CMD_LAYER_GET_PARA fail in hwc_set_rect, screen_idx:%d,hdl:%d\n",screen_idx,ctx->video_layerhdl[screen_idx]);
                        	}

                        	layer_info.src_win.x = croprect.left;
                        	layer_info.src_win.y = croprect.top;
                        	layer_info.src_win.width = croprect.right - croprect.left;
                        	layer_info.src_win.height = croprect.bottom - croprect.top;
                            if(ctx->cur_3d_out == HWC_3D_OUT_MODE_ANAGLAGH)
                            {
                                if(ctx->cur_3d_src == HWC_3D_SRC_MODE_SSF || ctx->cur_3d_src == HWC_3D_SRC_MODE_SSH)
                                {
                                    layer_info.src_win.x /=2;
                                    layer_info.src_win.width /=2;
                                }
                                if(ctx->cur_3d_src == HWC_3D_SRC_MODE_TB)
                                {
                                    layer_info.src_win.y /=2;
                                    layer_info.src_win.height /=2;
                                }
                            }
                        	layer_info.scn_win.x = displayframe_dst.left;
                        	layer_info.scn_win.y = displayframe_dst.top;
                        	layer_info.scn_win.width = displayframe_dst.right - displayframe_dst.left;
                        	layer_info.scn_win.height = displayframe_dst.bottom - displayframe_dst.top;
                            
                        	args[0] 				= screen_idx;
                        	args[1] 				= ctx->video_layerhdl[screen_idx];
                        	args[2] 				= (unsigned long) (&layer_info);
                        	args[3] 				= 0;
                        	ioctl(ctx->dispfd, DISP_CMD_LAYER_SET_PARA, args);

                            ctx->rect_out_active[screen_idx].left = displayframe_dst.left;
                            ctx->rect_out_active[screen_idx].right = displayframe_dst.right;
                            ctx->rect_out_active[screen_idx].top = displayframe_dst.top;
                            ctx->rect_out_active[screen_idx].bottom = displayframe_dst.bottom;
                        }
                    }
                }            
                
                ctx->rect_in.left = list->hwLayers[i].sourceCrop.left;
                ctx->rect_in.right = list->hwLayers[i].sourceCrop.right;
                ctx->rect_in.top = list->hwLayers[i].sourceCrop.top;
                ctx->rect_in.bottom = list->hwLayers[i].sourceCrop.bottom;
                
                ctx->rect_out.left = list->hwLayers[i].displayFrame.left;
                ctx->rect_out.right = list->hwLayers[i].displayFrame.right;
                ctx->rect_out.top = list->hwLayers[i].displayFrame.top;
                ctx->rect_out.bottom = list->hwLayers[i].displayFrame.bottom;
                if(ctx->rotation_video)
                {
                    ctx->rect_out.left = list->hwLayers[i].displayFrame.top;
                    ctx->rect_out.right = list->hwLayers[i].displayFrame.bottom;
                    ctx->rect_out.top = list->hwLayers[i].displayFrame.left;
                    ctx->rect_out.bottom = list->hwLayers[i].displayFrame.right;
                }
            }
            ctx->have_video_overlay = 1;
            list->hwLayers[i].compositionType = HWC_OVERLAY;
        }
        else
        {
            list->hwLayers[i].compositionType = HWC_FRAMEBUFFER;
        }
    }     

    //close video layer if there is not overlay
    for(screen_idx=0; screen_idx<2; screen_idx++)
    {
        if(((screen_idx == 0) && (ctx->mode==HWC_MODE_SCREEN0 || ctx->mode==HWC_MODE_SCREEN0_AND_SCREEN1 || ctx->mode==HWC_MODE_SCREEN0_GPU))
            || ((screen_idx == 1) && (ctx->mode == HWC_MODE_SCREEN1 || ctx->mode==HWC_MODE_SCREEN0_TO_SCREEN1 || ctx->mode==HWC_MODE_SCREEN0_AND_SCREEN1)))
        {
            if((ctx->have_video_overlay == 0) && (ctx->status[screen_idx] & HWC_STATUS_OPENED))
            {
                hwc_video_close(ctx, screen_idx);
            }
            else if((ctx->have_video_overlay) && ((ctx->status[screen_idx] & (HWC_STATUS_OPENED | HWC_STATUS_HAVE_FRAME)) == HWC_STATUS_HAVE_FRAME))
            {
                hwc_video_open(ctx, screen_idx);
            }
        }
    }

    return ret;
}


static int hwc_set_init_para(sun4i_hwc_context_t *ctx,uint32_t value,int mode_change)
{
    layerinitpara_t				*layer_init_para = (layerinitpara_t *)value;
    unsigned int                screen_idx;
    unsigned long args[4]={0};
    int hdmi_connected;

    if(ctx->screen_para.app_width[0] < ctx->screen_para.app_height[0])
    {
        args[0] = 1;
        hdmi_connected = ioctl(ctx->dispfd,DISP_CMD_HDMI_GET_HPD_STATUS,(unsigned long)args);

        ALOGD("####%s,hdmi status:%d,mode:%d\n",__FUNCTION__, hdmi_connected,ctx->mode);
        if(1 == hdmi_connected && ctx->mode == HWC_MODE_SCREEN0)
        {
            hwc_set_mode(ctx, HWC_MODE_SCREEN0_TO_SCREEN1);
        }
        else if(0 == hdmi_connected && ctx->mode == HWC_MODE_SCREEN0_TO_SCREEN1)
        {
            hwc_set_mode(ctx, HWC_MODE_SCREEN0);
        }
    }
    
    ALOGD("####%s, mode:%d, w:%d, h:%d, format:0x%x\n",
        __FUNCTION__, ctx->mode, layer_init_para->w, layer_init_para->h, layer_init_para->format);
    
    for(screen_idx=0; screen_idx<2; screen_idx++)
    {
        if(((screen_idx == 0) && (ctx->mode==HWC_MODE_SCREEN0 || ctx->mode==HWC_MODE_SCREEN0_AND_SCREEN1 || ctx->mode==HWC_MODE_SCREEN0_GPU))
            || ((screen_idx == 1) && (ctx->mode==HWC_MODE_SCREEN1 || ctx->mode==HWC_MODE_SCREEN0_TO_SCREEN1 || ctx->mode==HWC_MODE_SCREEN0_AND_SCREEN1)))
        {
            hwc_video_request(ctx, screen_idx, layer_init_para);
        }
    }
    
    ctx->w = layer_init_para->w;
    ctx->h = layer_init_para->h;
    ctx->format = layer_init_para->format;
    memset(&ctx->rect_in, 0, sizeof(hwc_rect_t));
    memset(&ctx->rect_out, 0, sizeof(hwc_rect_t));
    ctx->cur_3denable = 0;
    ctx->status[0] = HWC_STATUS_HAVE_VIDEO;
    ctx->status[1] = HWC_STATUS_HAVE_VIDEO;
    STATUS_LOG("#status[0]=%d in %s", ctx->status[0], __FUNCTION__);
    STATUS_LOG("#status[1]=%d in %s", ctx->status[1], __FUNCTION__);

	return 0;
}

static int hwc_set_frame_para(sun4i_hwc_context_t *ctx,uint32_t value)
{
    libhwclayerpara_t           *overlaypara;
    int                         screen_idx;

    ALOGV("####hwc_set_frame_para\n");
    
    for(screen_idx=0; screen_idx<2; screen_idx++)
    {
        if(((screen_idx == 0) && (ctx->mode==HWC_MODE_SCREEN0 || ctx->mode==HWC_MODE_SCREEN0_AND_SCREEN1 || ctx->mode==HWC_MODE_SCREEN0_GPU))
            || ((screen_idx == 1) && (ctx->mode==HWC_MODE_SCREEN1 || ctx->mode==HWC_MODE_SCREEN0_TO_SCREEN1 || ctx->mode==HWC_MODE_SCREEN0_AND_SCREEN1)))
        {
            if(ctx->video_layerhdl[screen_idx])
            {
                overlaypara = (libhwclayerpara_t*)value;
                
                hwc_video_set_frame_para(ctx, screen_idx, overlaypara);
            }
        }
    }
    
    return 0;
}


static int hwc_get_frame_id(sun4i_hwc_context_t *ctx)
{
    int                         ret = -1;
    unsigned long               args[4]={0};

    if(ctx->mode==HWC_MODE_SCREEN0 || ctx->mode==HWC_MODE_SCREEN0_GPU)
    {
    	args[0] = 0;
    	args[1] = ctx->video_layerhdl[0];
    	ret = ioctl(ctx->dispfd, DISP_CMD_VIDEO_GET_FRAME_ID, args);
    }
    else if(ctx->mode==HWC_MODE_SCREEN0_TO_SCREEN1 || ctx->mode==HWC_MODE_SCREEN1)
    {
        args[0] = 1;
        args[1] = ctx->video_layerhdl[1];
        ret = ioctl(ctx->dispfd, DISP_CMD_VIDEO_GET_FRAME_ID, args);
    }
    else if(ctx->mode == HWC_MODE_SCREEN0_AND_SCREEN1)
    {
        int ret0,ret1;
        
    	args[0] = 0;
    	args[1] = ctx->video_layerhdl[0];
    	ret0 = ioctl(ctx->dispfd, DISP_CMD_VIDEO_GET_FRAME_ID, args);

        args[0] = 1;
        args[1] = ctx->video_layerhdl[1];
        ret1 = ioctl(ctx->dispfd, DISP_CMD_VIDEO_GET_FRAME_ID, args);

        ret = (ret0<ret1)?ret0:ret1;
    }

    if(ret <0)
    {
        ALOGV("####hwc_get_frame_id return -1,mode:%d\n",ctx->mode);
    }
    return ret;
}

static int hwc_set3dmode(sun4i_hwc_context_t *ctx,int para)
{
	video3Dinfo_t *_3d_info = (video3Dinfo_t *)para;
    unsigned int                screen_idx;

    ALOGV("####%s\n", __FUNCTION__);
    
    for(screen_idx=0; screen_idx<2; screen_idx++)
    {
        if(((screen_idx == 0) && (ctx->mode==HWC_MODE_SCREEN0 || ctx->mode==HWC_MODE_SCREEN0_AND_SCREEN1 || ctx->mode==HWC_MODE_SCREEN0_GPU))
            || ((screen_idx == 1) && (ctx->mode==HWC_MODE_SCREEN1 || ctx->mode==HWC_MODE_SCREEN0_TO_SCREEN1 || ctx->mode==HWC_MODE_SCREEN0_AND_SCREEN1)))
        {
            hwc_video_3d_mode(ctx, screen_idx, _3d_info);
        }
    }

    return 0;
}

static int hwc_set_3d_parallax(sun4i_hwc_context_t *ctx,uint32_t value)
{
    __disp_layer_info_t         layer_info;
    unsigned int                screen_idx;
    int                         ret = -1;
    unsigned long               args[4]={0};

    ALOGD("####%s value:%d\n", __FUNCTION__, value);
    
    for(screen_idx=0; screen_idx<2; screen_idx++)
    {
        if(((screen_idx == 0) && (ctx->mode==HWC_MODE_SCREEN0 || ctx->mode==HWC_MODE_SCREEN0_AND_SCREEN1 || ctx->mode==HWC_MODE_SCREEN0_GPU))
            || ((screen_idx == 1) && (ctx->mode==HWC_MODE_SCREEN1 || ctx->mode==HWC_MODE_SCREEN0_TO_SCREEN1 || ctx->mode==HWC_MODE_SCREEN0_AND_SCREEN1)))
        {
            if(ctx->video_layerhdl[screen_idx])
            {
                args[0] = screen_idx;
                args[1] = ctx->video_layerhdl[screen_idx];
                args[2] = (unsigned long)&layer_info;
                ret = ioctl(ctx->dispfd, DISP_CMD_LAYER_GET_PARA, args);
                if(ret < 0)
                {
                    ALOGD("####DISP_CMD_LAYER_GET_PARA fail in %s, screen_idx:%d, hdl:%d\n",__FUNCTION__, screen_idx, ctx->video_layerhdl[screen_idx]);
                }

                if(layer_info.fb.b_trd_src && (layer_info.fb.trd_mode==DISP_3D_SRC_MODE_SSF || layer_info.fb.trd_mode==DISP_3D_SRC_MODE_SSH) && layer_info.b_trd_out)
                {
                    args[0] = screen_idx;
                    args[1] = ctx->video_layerhdl[screen_idx];
                    args[2] = (unsigned long)&layer_info;
                    ret = ioctl(ctx->dispfd, DISP_CMD_LAYER_SET_PARA, args);
                }
            }
            else
            {
                ALOGD("####have not video in %s", __FUNCTION__);
            }
        }
    }
    
    return 0;
}

static int hwc_is_valid_format(int format)
{
    switch(format) 
    {
    case HAL_PIXEL_FORMAT_RGBA_8888:
    case HAL_PIXEL_FORMAT_RGBX_8888:
    case HAL_PIXEL_FORMAT_RGB_888:
    case HAL_PIXEL_FORMAT_RGB_565:
    case HAL_PIXEL_FORMAT_BGRA_8888:
    case HAL_PIXEL_FORMAT_RGBA_5551:
    case HAL_PIXEL_FORMAT_RGBA_4444:
        return 1;
    default:
        return 0;
    }
}

static int hwc_is_BLENDED(hwc_layer_t* psLayer)
{
	//return psLayer->blending != HWC_BLENDING_NONE;
	return ((psLayer->blending != HWC_BLENDING_NONE) || (psLayer->alpha<0XFF));
}

static int hwc_is_SCALED(hwc_layer_t *layer)
{
    int w = layer->sourceCrop.right - layer->sourceCrop.left;
    int h = layer->sourceCrop.bottom - layer->sourceCrop.top;

    if (layer->transform & HWC_TRANSFORM_ROT_90)
    {
        int tmp = w;
        w = h;
        h = tmp;
    }

    return ((layer->displayFrame.right - layer->displayFrame.left) != w) || ((layer->displayFrame.bottom - layer->displayFrame.top)!= h);
}

static int hwc_is_valid_layer(hwc_composer_device_t *dev,hwc_layer_t *layer,IMG_native_handle_t *handle)
{
    /* Skip layers are handled by SF */
    if ((layer->flags & HWC_SKIP_LAYER) || !handle)
    {
        return 0;
    }
    
    //ALOGD("##%x,%d,%d,%d", handle, handle->iFormat, handle->iWidth, handle->iHeight);

    if (!hwc_is_valid_format(handle->iFormat))
    {
        return 0;
    }
    
    if(hwc_is_SCALED(layer))
    {
        return 0;
    }

    if(layer->transform)
    {
        return 0;
    }

    return 1;
}

static void hwc_gather_layer_statistics(hwc_composer_device_t *dev,  hwc_layer_list_t *list)
{
    sun4i_hwc_context_t * ctx = (sun4i_hwc_context_t *)dev;
    unsigned int i;
    unsigned int pipe_num;

    /* Figure out how many layers we can support via dispc */
    for (i = 0; list && i < list->numHwLayers; i++) 
    {
        hwc_layer_t *layer = &list->hwLayers[i];
        IMG_native_handle_t *handle = (IMG_native_handle_t *)layer->handle;

        if (hwc_is_valid_layer(dev, layer, handle)) 
        {
            ctx->possible_overlay_layers++;
        }

        if(hwc_is_BLENDED(layer))
        {
            if(i != 0)
            {
                ctx->num_pipe++;
            }
        }
    }
}

static int hwc_can_dispc_render_all(hwc_composer_device_t *dev, hwc_layer_list_t* list)
{
    sun4i_hwc_context_t * ctx = (sun4i_hwc_context_t *)dev;
    
    return  (ctx->possible_overlay_layers <= ctx->max_hw_overlays &&
            ctx->possible_overlay_layers == list->numHwLayers &&
            ctx->num_pipe <= 2);
}

static void hwc_setup_layer(hwc_composer_device_t *dev, hwc_layer_t *layer, int zorder, int pipe)
{
    sun4i_hwc_context_t * ctx = (sun4i_hwc_context_t *)dev;
    IMG_native_handle_t *handle = (IMG_native_handle_t *)layer->handle;
    __disp_layer_info_t * layer_info = &(ctx->disp_data.layer_info[zorder]);

    memset(layer_info, 0, sizeof(__disp_layer_info_t));
    
    switch(handle->iFormat)
    {
        case HAL_PIXEL_FORMAT_RGBA_8888:
            layer_info->fb.mode = DISP_MOD_INTERLEAVED;
            layer_info->fb.format = DISP_FORMAT_ARGB8888;
            layer_info->fb.seq = DISP_SEQ_ARGB;
            layer_info->fb.br_swap = 1;
            break;
        case HAL_PIXEL_FORMAT_RGBX_8888:
            layer_info->fb.mode = DISP_MOD_INTERLEAVED;
            layer_info->fb.format = DISP_FORMAT_ARGB8888;
            layer_info->fb.seq = DISP_SEQ_ARGB;
            layer_info->fb.br_swap = 1;
            layer_info->alpha_en = 1;
            layer_info->alpha_val = 0xff;
            break;
        case HAL_PIXEL_FORMAT_RGB_888:
            layer_info->fb.mode = DISP_MOD_INTERLEAVED;
            layer_info->fb.format = DISP_FORMAT_RGB888;
            layer_info->fb.seq = DISP_SEQ_ARGB;
            layer_info->fb.br_swap = 1;
            break;
        case HAL_PIXEL_FORMAT_RGB_565:
            layer_info->fb.mode = DISP_MOD_INTERLEAVED;
            layer_info->fb.format = DISP_FORMAT_RGB565;
            layer_info->fb.seq = DISP_SEQ_ARGB;
            layer_info->fb.br_swap = 0;
            break;
        case HAL_PIXEL_FORMAT_BGRA_8888:
            layer_info->fb.mode = DISP_MOD_INTERLEAVED;
            layer_info->fb.format = DISP_FORMAT_ARGB8888;
            layer_info->fb.seq = DISP_SEQ_ARGB;
            layer_info->fb.br_swap = 0;
            break;
        case HAL_PIXEL_FORMAT_RGBA_5551:
            layer_info->fb.mode = DISP_MOD_INTERLEAVED;
            layer_info->fb.format = DISP_FORMAT_ARGB1555;
            layer_info->fb.seq = DISP_SEQ_ARGB;
            layer_info->fb.br_swap = 1;
            break;
        case HAL_PIXEL_FORMAT_RGBA_4444:
            layer_info->fb.mode = DISP_MOD_INTERLEAVED;
            layer_info->fb.format = DISP_FORMAT_ARGB4444;
            layer_info->fb.seq = DISP_SEQ_ARGB;
            layer_info->fb.br_swap = 1;
            break;
        default:
            ALOGE("not support format %d in %s", handle->iFormat, __FUNCTION__);
            break;
    }
	
	if(layer->alpha < 0xff)
    {
        layer_info->alpha_en = 1;
        layer_info->alpha_val = 0;//layer->alpha;
    }else if(zorder == 0 || hwc_is_BLENDED(layer)==0)
    {
        layer_info->alpha_en = 1;
        layer_info->alpha_val = 0xff;
    }

    if(layer->blending == HWC_BLENDING_PREMULT)
    {
        layer_info->fb.pre_multiply = 1;
    }

    layer_info->fb.size.width = (handle->iWidth+31)/32*32;
    layer_info->fb.size.height = handle->iHeight;

    layer_info->src_win.x = layer->sourceCrop.left;
    layer_info->src_win.y = layer->sourceCrop.top;
    layer_info->src_win.width = layer->sourceCrop.right - layer->sourceCrop.left;
    layer_info->src_win.height = layer->sourceCrop.bottom - layer->sourceCrop.top;

    layer_info->scn_win.x = layer->displayFrame.left;
    layer_info->scn_win.y = layer->displayFrame.top;
    layer_info->scn_win.width = layer->displayFrame.right - layer->displayFrame.left;
    layer_info->scn_win.height = layer->displayFrame.bottom - layer->displayFrame.top;

    layer_info->mode = DISP_LAYER_WORK_MODE_NORMAL;
    layer_info->pipe = pipe - 1;
    layer_info->prio = zorder;
}   

static int hwc_prepare(hwc_composer_device_t *dev, hwc_layer_list_t* list) 
{
    sun4i_hwc_context_t * ctx = (sun4i_hwc_context_t *)dev;
    int have_fb = 0, pipe = 1;
    unsigned long args[4]={0};
	hwc_rect_t fb_win;

    ALOGV("########hwc_prepare %d", list->numHwLayers);

	fb_win.left = 10000;
	fb_win.right = 0;
	fb_win.top = 10000;
	fb_win.bottom = 0;
	
    memset(ctx->buffers, 0, sizeof(buffer_handle_t) * 4);
	ctx->use_sgx = 1;
	ctx->post2_layers = 0;
    ctx->possible_overlay_layers = 0;
    ctx->num_pipe = 1;
    
	ctx->disp_data.use_sgx = 1;
	ctx->disp_data.post2_layers = 0;
	ctx->disp_data.fb_scn_win.x = 0;
	ctx->disp_data.fb_scn_win.y = 0;
	ctx->disp_data.fb_scn_win.width = ctx->screen_para.app_width[0];
	ctx->disp_data.fb_scn_win.height = ctx->screen_para.app_height[0];

    hwc_set_rect(dev,list);
    
    hwc_gather_layer_statistics(dev, list);

    if (list->numHwLayers<= 1)
    {
        ctx->force_sgx |= HWC_FORCE_SGX_REASON_ONE_LAYER;
    }
	else
	{
        ctx->force_sgx &= (~HWC_FORCE_SGX_REASON_ONE_LAYER);
    }
    
    if (hwc_can_dispc_render_all(dev, list)) 
    {
        ctx->use_sgx = 0;
        //ctx->force_sgx &= (~HWC_FORCE_SGX_REASON_CAN_NOT_RENDER_ALL);
    } 
    else 
    {
        ctx->use_sgx = 1;
        //ctx->force_sgx |= HWC_FORCE_SGX_REASON_CAN_NOT_RENDER_ALL;
    }
            
    pipe = ctx->use_sgx + 1;

    for (size_t i=0 ; i<list->numHwLayers ; i++) 
    {
        hwc_layer_t *layer = &list->hwLayers[i];
        IMG_native_handle_t *handle = (IMG_native_handle_t *)layer->handle;
        int add_pipe = 0;  
        
        bool can_render_layer = hwc_is_valid_layer(dev, layer, handle);
        bool trans_ovl_mid_fb = (hwc_is_BLENDED(layer) && have_fb);
        if(fb_win.left > layer->displayFrame.right || fb_win.bottom < layer->displayFrame.top || 
        	fb_win.right < layer->displayFrame.left || fb_win.top > layer->displayFrame.bottom)
		{
			trans_ovl_mid_fb = 0;
		}
		
        if(hwc_is_BLENDED(layer) && (i != 0))
        {
            add_pipe = 1;
        }
        if(!ctx->force_sgx && can_render_layer && !trans_ovl_mid_fb && ((pipe+add_pipe) <= 2) && (ctx->post2_layers < ctx->max_hw_overlays))
        {
            layer->compositionType = HWC_OVERLAY;

            if (ctx->use_sgx && !hwc_is_BLENDED(layer))
            {
                layer->hints |= HWC_HINT_CLEAR_FB;
            }

            if(hwc_is_BLENDED(layer) && (i != 0))
            {
                pipe++;
            }
            ctx->buffers[ctx->post2_layers] = layer->handle;
            hwc_setup_layer(dev, layer, ctx->post2_layers, pipe);

            ctx->post2_layers++;
        }
        else if(layer->compositionType != HWC_OVERLAY)
        {        	
        	if(layer->displayFrame.left < fb_win.left) 
        	{
        		fb_win.left = (layer->displayFrame.left<0)?0:layer->displayFrame.left;
        	}
        	if(layer->displayFrame.right > fb_win.right) 
        	{
        		fb_win.right = (layer->displayFrame.right>(int)ctx->screen_para.app_width[0])?ctx->screen_para.app_width[0]:layer->displayFrame.right;
        	}
        	if(layer->displayFrame.top < fb_win.top) 
        	{
        		fb_win.top = (layer->displayFrame.top<0)?0:layer->displayFrame.top;
        	}
        	if(layer->displayFrame.bottom > fb_win.bottom) 
        	{
        		fb_win.bottom = (layer->displayFrame.bottom>(int)ctx->screen_para.app_height[0])?ctx->screen_para.app_height[0]:layer->displayFrame.bottom;
        	}

			ctx->disp_data.fb_scn_win.x = fb_win.left;
			ctx->disp_data.fb_scn_win.y = fb_win.top;
			ctx->disp_data.fb_scn_win.width = fb_win.right - fb_win.left;
			ctx->disp_data.fb_scn_win.height = fb_win.bottom - fb_win.top;

            have_fb = 1;
        }
        
        if(handle)
        {
            ALOGV("##id:%d,type:%d, w:%d, h:%d, format:%d, blending:0x%x, usage:0x%x, flags:0x%x, transform:0x%x [%d,%d,%d,%d], [%d,%d,%d,%d]", 
            i, layer->compositionType, handle->iWidth,handle->iHeight,handle->iFormat, layer->blending,handle->usage, layer->flags, layer->transform,
            layer->sourceCrop.left,layer->sourceCrop.right,layer->sourceCrop.top,layer->sourceCrop.bottom,
            layer->displayFrame.left,layer->displayFrame.right,layer->displayFrame.top,layer->displayFrame.bottom);
        }
    }

	if(ctx->post2_layers < list->numHwLayers)
	{
		ctx->use_sgx = 1;
	}

	ctx->disp_data.post2_layers = ctx->post2_layers;
	ctx->disp_data.use_sgx = ctx->use_sgx;

    return 0;
}

static int hwc_set(hwc_composer_device_t *dev,
        hwc_display_t dpy,
        hwc_surface_t sur,
        hwc_layer_list_t* list)
{
    sun4i_hwc_context_t * ctx = (sun4i_hwc_context_t *)dev;
    int err = 0;
    unsigned long args[4]={0};

    ALOGV("###hwc_set use_sgx:%d post2_layers:%d", ctx->use_sgx, ctx->post2_layers);

    if(dpy && sur) 
    {	    
        if(ctx->force_sgx & HWC_FORCE_SGX_REASON_STILL0)
        {
            ctx->force_sgx &= (~HWC_FORCE_SGX_REASON_STILL0);
        }
        else if(ctx->force_sgx & HWC_FORCE_SGX_REASON_STILL1)
        {
            ctx->force_sgx &= (~HWC_FORCE_SGX_REASON_STILL1);
        }
        
        if(1)//ctx->use_sgx) 
        {
            EGLBoolean sucess = eglSwapBuffers((EGLDisplay)dpy, (EGLSurface)sur);
            if (!sucess) 
            {
                //ALOGD("####eglSwapBuffers fail in %s, ret:%d", __FUNCTION__, sucess);
                //return HWC_EGL_ERROR;
            }
        }

        err = ctx->fb_dev->Post2((framebuffer_device_t *)ctx->fb_dev,
                         ctx->buffers,
                         ctx->post2_layers+ctx->use_sgx,
                         &(ctx->disp_data), sizeof(setup_dispc_data_t));
        if (err)
        {
            ALOGE("####Post2 error");
        }
    }
    return 0;
}

static int hwc_set_mode(sun4i_hwc_context_t *ctx, e_hwc_mode_t mode)
{
    unsigned int screen_idx;

    ALOGD("####%s mode:%d\n", __FUNCTION__, mode);

    if(mode == ctx->mode)
    {
        ALOGV("####mode not change\n");
        return 0;
    }

	if(mode==HWC_MODE_SCREEN0_TO_SCREEN1 || mode==HWC_MODE_SCREEN0_AND_SCREEN1)
	{
		ctx->force_sgx |= HWC_FORCE_SGX_REASON_DUAL_SCREEN;
	}
	else
	{
		ctx->force_sgx &= (~HWC_FORCE_SGX_REASON_DUAL_SCREEN);
	}
    
    for(screen_idx=0; screen_idx<2; screen_idx++)
    {        
        if(((screen_idx == 0) && (mode==HWC_MODE_SCREEN0 || mode==HWC_MODE_SCREEN0_AND_SCREEN1 || mode==HWC_MODE_SCREEN0_GPU))
            || ((screen_idx == 1) && (mode==HWC_MODE_SCREEN1 || mode==HWC_MODE_SCREEN0_TO_SCREEN1 || mode==HWC_MODE_SCREEN0_AND_SCREEN1)))
        {
        }
        else if(ctx->video_layerhdl[screen_idx])
        {
            hwc_video_release(ctx, screen_idx);
        }
    }

    for(screen_idx=0; screen_idx<2; screen_idx++)
    {        
        if(((screen_idx == 0) && (mode==HWC_MODE_SCREEN0 || mode==HWC_MODE_SCREEN0_AND_SCREEN1 || mode==HWC_MODE_SCREEN0_GPU))
            || ((screen_idx == 1) && (mode==HWC_MODE_SCREEN1 || mode==HWC_MODE_SCREEN0_TO_SCREEN1 || mode==HWC_MODE_SCREEN0_AND_SCREEN1)))
        {
            if((ctx->screen_para.app_width[0] > ctx->screen_para.app_height[0]) && (ctx->status[screen_idx] & HWC_STATUS_HAVE_VIDEO) && (ctx->video_layerhdl[screen_idx] == 0))
            {
                layerinitpara_t layer_init_para;
                video3Dinfo_t _3d_info;
            
                layer_init_para.w = ctx->w;
                layer_init_para.h = ctx->h;
                layer_init_para.format = ctx->format;
                hwc_video_request(ctx, screen_idx, &layer_init_para);
            
                hwc_video_set_frame_para(ctx, screen_idx, &ctx->cur_frame_para);
                
                _3d_info.width = ctx->cur_3d_w;
                _3d_info.height = ctx->cur_3d_h;
                _3d_info.format = ctx->cur_3d_format;
                _3d_info.src_mode = ctx->cur_3d_src;
                _3d_info.display_mode = ctx->cur_3d_out;
                hwc_video_3d_mode(ctx, screen_idx, &_3d_info);
            }
        }
    }


    ctx->mode = mode;

    return 0;
}

static int hwc_set_screen_para(sun4i_hwc_context_t *ctx,uint32_t value)
{
    screen_para_t *screen_info = (screen_para_t *)value;
    unsigned long args[4]={0};
    int hdmi_connected;

    ALOGD("####hwc_set_screen_para,%d,%d,%d,%d,%d,%d",screen_info->app_width[0],screen_info->app_height[0],
        screen_info->width[0],screen_info->height[0],screen_info->valid_width[0],screen_info->valid_height[0]);

    memcpy(&ctx->screen_para,screen_info,sizeof(screen_para_t));

    args[0] = 1;
    hdmi_connected = ioctl(ctx->dispfd,DISP_CMD_HDMI_GET_HPD_STATUS,(unsigned long)args);

    ALOGD("####hwc_set_screen_para,hdmi status:%d\n", hdmi_connected);

    if(hdmi_connected == 1 && (screen_info->app_width[0] < screen_info->app_height[0]) &&
        (ctx->orientation == 1 || ctx->orientation == 3))
    {
        ctx->rotation_video = true;
    }
    else
    {
        ctx->rotation_video = false;
    }
    ALOGD("####hwc_set_screen_para,ctx->rotation_video:%d\n", ctx->rotation_video);    
    return 0;
}

static int hwc_set_show(sun4i_hwc_context_t *ctx,uint32_t enable)
{
    int screen_idx;

    ALOGD("####%s enable:%d", __FUNCTION__, enable);
    
    if(enable == 0)
    {    
        for(screen_idx=0; screen_idx<2; screen_idx++)
        {
            if(((screen_idx == 0) && (ctx->mode==HWC_MODE_SCREEN0 || ctx->mode==HWC_MODE_SCREEN0_AND_SCREEN1 || ctx->mode==HWC_MODE_SCREEN0_GPU))
                || ((screen_idx == 1) && (ctx->mode==HWC_MODE_SCREEN1 || ctx->mode==HWC_MODE_SCREEN0_TO_SCREEN1 || ctx->mode==HWC_MODE_SCREEN0_AND_SCREEN1)))
            {
                hwc_video_release(ctx, screen_idx);
            }
        }
        
        ctx->status[0] = 0;
        ctx->status[1] = 0;
        STATUS_LOG("#status[0]=%d in %s", ctx->status[0], __FUNCTION__);
        STATUS_LOG("#status[1]=%d in %s", ctx->status[1], __FUNCTION__);
    }
    else if((ctx->screen_para.app_width[0] < ctx->screen_para.app_height[0]) && 1 == enable)
    {
        for(screen_idx=0; screen_idx<2; screen_idx++)
        {
            if(((screen_idx == 0) && (ctx->mode==HWC_MODE_SCREEN0 || ctx->mode==HWC_MODE_SCREEN0_AND_SCREEN1 || ctx->mode==HWC_MODE_SCREEN0_GPU))
                || ((screen_idx == 1) && (ctx->mode == HWC_MODE_SCREEN1 || ctx->mode==HWC_MODE_SCREEN0_TO_SCREEN1 || ctx->mode==HWC_MODE_SCREEN0_AND_SCREEN1)))
            {
                if((ctx->status[screen_idx] & (HWC_STATUS_OPENED | HWC_STATUS_HAVE_FRAME)) == HWC_STATUS_HAVE_FRAME)
                {
                   hwc_video_open(ctx, screen_idx);
                }
            }
        }
    }
    return 0;
}

static int hwc_set_orientation(sun4i_hwc_context_t *ctx,uint32_t value)
{    
    ctx->orientation = value;
    ALOGD("###################hwc_set_orientation %d", value);
    return 0;
}

static int hwc_setparameter(hwc_composer_device_t *dev,uint32_t param,uint32_t value)
{
	int 						ret = 0;
    sun4i_hwc_context_t   		*ctx = (sun4i_hwc_context_t *)dev;
	
    if(param == HWC_LAYER_SETINITPARA)
    {
    	ret = hwc_set_init_para(ctx,value, 0);
    }
    else if(param == HWC_LAYER_SETFRAMEPARA)
    {
    	ret = hwc_set_frame_para(ctx,value);
    }
    else if(param == HWC_LAYER_GETCURFRAMEPARA)
    {
    	ret = hwc_get_frame_id(ctx);
    }
    else if(param == HWC_LAYER_SETMODE)
    {
        ret = hwc_set_mode(ctx, (e_hwc_mode_t)value);
    }
	else if(param == HWC_LAYER_SET3DMODE)
	{
	    ret = hwc_set3dmode(ctx,value);
	}
	else if(param == HWC_LAYER_SET_3D_PARALLAX)
	{
	    ret = hwc_set_3d_parallax(ctx,value);
	}
	else if(param == HWC_LAYER_SET_SCREEN_PARA)
	{
	    ret = hwc_set_screen_para(ctx,value);
	}
	else if(param == HWC_LAYER_SHOW)
	{
	    ret = hwc_set_show(ctx,value);
	}
    else if(param == HWC_LAYER_SET_ORIENTATION)
    {
        ret = hwc_set_orientation(ctx, value);
    }

    return ( ret );
}


static uint32_t hwc_getparameter(hwc_composer_device_t *dev,uint32_t param)
{
    return 0;
}

static int hwc_uevent(hwc_context_t *ctx)
{
	struct sockaddr_nl snl;
	const int buffersize = 16*1024;
	int retval;
	int hotplug_sock;

	memset(&snl, 0x0, sizeof(snl));
	snl.nl_family = AF_NETLINK;
	snl.nl_pid = getpid();
	snl.nl_groups = 0xffffffff;

	hotplug_sock = socket(PF_NETLINK, SOCK_DGRAM, NETLINK_KOBJECT_UEVENT);
	if (hotplug_sock == -1) {
		ALOGD("####socket is failed in %s error:%d %s\n", __FUNCTION__, errno, strerror(errno));
		return -1;
	}

	setsockopt(hotplug_sock, SOL_SOCKET, SO_RCVBUFFORCE, &buffersize, sizeof(buffersize));

	retval = bind(hotplug_sock, (struct sockaddr *)&snl, sizeof(struct sockaddr_nl));

	if (retval < 0) {
		ALOGD("####bind is failed in %s error:%d %s\n", __FUNCTION__, errno, strerror(errno));
		close(hotplug_sock);
		return -1;
	}

    while(true) {
		char buf[4096*2] = {0};
        struct pollfd fds;
        int err;
    
        fds.fd = hotplug_sock;
        fds.events = POLLIN;
        fds.revents = 0;
        
        err = poll(&fds, 1, 256);
        
        if(err == 0)
        {
            if(!ctx->force_sgx)
            {
                ctx->force_sgx |= (HWC_FORCE_SGX_REASON_STILL0 | HWC_FORCE_SGX_REASON_STILL1);
                ctx->procs->invalidate(ctx->procs);
                ALOGV("#########invalidate, ctx->force_sgx:%x", ctx->force_sgx);
                continue;
            }
        }

        if(err > 0 && fds.events == POLLIN)
        {
    		int count = recv(hotplug_sock, &buf, sizeof(buf),0);
    		if(count > 0)
    		{
        		ALOGV("####received %s", buf);

                bool vsync = !strcmp(buf, "change@/devices/platform/disp");
                if(vsync && ctx->vsync_en)
                {
                    uint64_t timestamp = 0;
                    unsigned int display_id = -1;
                    const char *s = buf;
                
                    if(!ctx->procs || !ctx->procs->vsync)
                       return 0;

                    s += strlen(s) + 1;
                    while(s)
                    {
                        if (!strncmp(s, "VSYNC0=", strlen("VSYNC0=")))
                        {
                            timestamp = strtoull(s + strlen("VSYNC0="), NULL, 0);
                            ALOGV("#### %s display0 timestamp:%lld", s,timestamp);
                            display_id = 0;
                        }
                        else if (!strncmp(s, "VSYNC1=", strlen("VSYNC1=")))
                        {
                            timestamp = strtoull(s + strlen("VSYNC1="), NULL, 0);
                            ALOGV("#### %s display1 timestamp:%lld", s,timestamp);
                            display_id = 1;
                        }

                        s += strlen(s) + 1;
                        if(s - buf >= count)
                        {
                            break;
                        }
                    }

                    if(display_id == 0)//only handle display 0 vsync event now
                    {
                        ctx->procs->vsync(ctx->procs, display_id, timestamp);
                    }
                }
            }
        }
    }

	return 0;
}

static void *hwc_vsync_thread(void *data)
{
    hwc_context_t *ctx = (hwc_context_t *)(data);

    setpriority(PRIO_PROCESS, 0, HAL_PRIORITY_URGENT_DISPLAY);

	hwc_uevent(ctx);

    return NULL;
}

static int hwc_eventControl(struct hwc_composer_device* dev,
        int event, int enabled)
{
    struct hwc_context_t* ctx = (struct hwc_context_t*)dev;
    unsigned long               arg[4]={0};

    switch (event) {
    case HWC_EVENT_VSYNC:
        arg[0] = 0;
        arg[1] = enabled;
        ioctl(ctx->dispfd,DISP_CMD_VSYNC_EVENT_EN,(unsigned long)arg);
        ctx->vsync_en = enabled;
        return 0;
    }

    return -EINVAL;
}

static const struct hwc_methods hwc_methods = {
    eventControl: hwc_eventControl
};

static void hwc_register_procs(struct hwc_composer_device* dev,
        hwc_procs_t const* procs)
{
    struct hwc_context_t* ctx = (struct hwc_context_t*)dev;
    ctx->procs = const_cast<hwc_procs_t *>(procs);
}


static int hwc_open_fb_hal(IMG_framebuffer_device_public_t **fb_dev)
{
    const struct hw_module_t *psModule;
    IMG_gralloc_module_public_t *psGrallocModule;
    int err;

    err = hw_get_module(GRALLOC_HARDWARE_MODULE_ID, &psModule);
    psGrallocModule = (IMG_gralloc_module_public_t *) psModule;

    if(err)
        goto err_out;

    if (strcmp(psGrallocModule->base.common.author, "Imagination Technologies")) {
        err = -EINVAL;
        goto err_out;
    }

    *fb_dev = psGrallocModule->psFrameBufferDevice;

    return 0;

err_out:
    ALOGE("Composer HAL failed to load compatible Graphics HAL");
    return err;
}

static int hwc_init(sun4i_hwc_context_t *ctx)
{
	unsigned long               arg[4]={0};
    __disp_init_t init_para;

    ctx->dispfd = open("/dev/disp", O_RDWR);
    if (ctx->dispfd < 0)
    {
        ALOGE("Failed to open disp device, ret:%d, errno: %d\n", ctx->dispfd, errno);
        return  -1;
    }
    
    ctx->mFD_fb[0] = open("/dev/graphics/fb0", O_RDWR);
    if (ctx->mFD_fb[0] < 0)
    {
        ALOGE("Failed to open fb0 device, ret:%d, errno:%d\n", ctx->mFD_fb[0], errno);
        return  -1;
    }
    
    ctx->mFD_fb[1] = open("/dev/graphics/fb1", O_RDWR);
    if (ctx->mFD_fb[1] < 0)
    {
        ALOGE("Failed to open fb1 device, ret:%d, errno:%d\n", ctx->mFD_fb[1], errno);
        return  -1;
    }

	arg[0] = 0;
	ctx->screen_para.app_width[0] = ioctl(ctx->dispfd,DISP_CMD_SCN_GET_WIDTH,(unsigned long)arg);
	ctx->screen_para.app_height[0] = ioctl(ctx->dispfd,DISP_CMD_SCN_GET_HEIGHT,(unsigned long)arg);

    ctx->mode = HWC_MODE_SCREEN0;

	ctx->b_video_in_valid_area = 1;
	ctx->max_hw_overlays = 3;

	hwc_open_fb_hal(&ctx->fb_dev);
	ctx->fb_dev->bBypassPost = 1;

	ctx->buffers = (buffer_handle_t*)malloc(sizeof(buffer_handle_t) * 4);

    return 0;
}

static int hwc_device_close(struct hw_device_t *dev)
{
    return 0;
}

static int hwc_device_open(const struct hw_module_t* module, const char* name,
        struct hw_device_t** device)
{
    int status = -EINVAL;
    if (!strcmp(name, HWC_HARDWARE_COMPOSER)) 
    {
        sun4i_hwc_context_t *dev;
        dev = (sun4i_hwc_context_t*)malloc(sizeof(*dev));

        /* initialize our state here */
        memset(dev, 0, sizeof(*dev));

        /* initialize the procs */
        dev->device.common.tag      = HARDWARE_DEVICE_TAG;
        dev->device.common.version  = HWC_DEVICE_API_VERSION_0_3;
        dev->device.common.module   = const_cast<hw_module_t*>(module);
        dev->device.common.close    = hwc_device_close;

        dev->device.prepare         = hwc_prepare;
        dev->device.set             = hwc_set;
        dev->device.setparameter    = hwc_setparameter;
        dev->device.getparameter    = hwc_getparameter;
        dev->device.registerProcs   = hwc_register_procs;
        dev->device.methods         = &hwc_methods;

        *device = &dev->device.common;
        status = 0;

        hwc_init(dev);

        if(dev->device.common.version == HWC_DEVICE_API_VERSION_0_3)
        {
            status = pthread_create(&dev->vsync_thread, NULL, hwc_vsync_thread, dev);
            if (status) {
                ALOGE("%s::pthread_create() failed : %s", __func__, strerror(status));
                status = -status;
            }
        }
    }
    return status;
}


#ifndef __HWCOMPOSER_PRIV_H__
#define __HWCOMPOSER_PRIV_H__

#include <hardware/hardware.h>

#include <fcntl.h>
#include <errno.h>

#include <cutils/log.h>
#include <cutils/atomic.h>

#include <hardware/hwcomposer.h>

#include <EGL/egl.h>
#include <hardware/hal_public.h>

enum
{
    HWC_STATUS_HAVE_FRAME       = 1,
    HWC_STATUS_COMPOSITED       = 2,//not used
    HWC_STATUS_OPENED           = 4,
    HWC_STATUS_HAVE_VIDEO       = 8,
};

enum
{
	HWC_FORCE_SGX_REASON_VIDEO 						= 0x00000001,
	HWC_FORCE_SGX_REASON_DUAL_SCREEN 				= 0x00000002,
	HWC_FORCE_SGX_REASON_ONE_LAYER 					= 0x00000004,
	HWC_FORCE_SGX_REASON_CAN_NOT_RENDER_ALL 		= 0x00000008,
	HWC_FORCE_SGX_REASON_STILL0                     = 0x00000010,
	HWC_FORCE_SGX_REASON_STILL1                     = 0x00000020,
};

typedef struct hwc_context_t 
{
    hwc_composer_device_t 	device;
    hwc_procs_t 			*procs;
    int						dispfd;
    int                     mFD_fb[2];
    e_hwc_mode_t            mode;
    screen_para_t           screen_para;
    bool					b_video_in_valid_area;

    uint32_t                ui_layerhdl[2];
    uint32_t                video_layerhdl[2];
    uint32_t                status[2];
    uint32_t		        w;
    uint32_t		        h;
    uint32_t		        format;
    bool					cur_3denable;
    e_hwc_3d_src_mode_t     cur_3d_src;
    e_hwc_3d_out_mode_t     cur_3d_out;
    uint32_t                cur_3d_w;
    uint32_t                cur_3d_h;
    e_hwc_format_t          cur_3d_format;
    hwc_rect_t              rect_in;
    hwc_rect_t              rect_out;
    hwc_rect_t              rect_out_active[2];
    __disp_tv_mode_t        org_hdmi_mode;
    __disp_rect_t           org_scn_win;
    libhwclayerpara_t       cur_frame_para;
    
    bool                    have_video_overlay;
    uint32_t                num_pipe;
    uint32_t                force_sgx;
    uint32_t                possible_overlay_layers;
    uint32_t                max_hw_overlays;
    bool                    use_sgx;
    IMG_framebuffer_device_public_t *fb_dev;
    uint32_t                post2_layers;
    buffer_handle_t         *buffers;
    setup_dispc_data_t      disp_data;//pass to 3rd party dc

    bool                    vsync_en;
    pthread_t               vsync_thread;
    uint32_t                orientation;
    bool                    rotation_video;
}sun4i_hwc_context_t;

#endif

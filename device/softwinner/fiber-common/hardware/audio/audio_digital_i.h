/*
 * Copyright (C) 2011 The Android Open Source Project
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

#ifndef  AUDIO_DIGITAL_I_H
#define  AUDIO_DIGITAL_I_H

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------

/* ALSA cards for A1X */
#define CARD_A1X_CODEC		0
#define CARD_A1X_HDMI		1
#define CARD_A1X_SPDIF		2
#define CARD_A1X_I2S		3
#define CARD_A1X_PCM		4
#define CARD_A1X_DEFAULT	CARD_A1X_CODEC

/* ALSA ports for A10 */
#define PORT_CODEC			0
#define PORT_PCM			0
#define PORT_HDMI			0
#define PORT_SPDIF			0
#define PORT_I2S			0
#define PORT_MODEM			0

#define SAMPLING_RATE_8K	8000
#define SAMPLING_RATE_11K	11025
#define SAMPLING_RATE_44K	44100
#define SAMPLING_RATE_48K	48000

/* constraint imposed by ABE: all period sizes must be multiples of 24 */
#define ABE_BASE_FRAME_COUNT 24
/* number of base blocks in a short period (low latency) */
//#define SHORT_PERIOD_MULTIPLIER 44  /* 22 ms */
#define SHORT_PERIOD_MULTIPLIER 44  /* 40 ms */
/* number of frames per short period (low latency) */
#define SHORT_PERIOD_SIZE (ABE_BASE_FRAME_COUNT * SHORT_PERIOD_MULTIPLIER)
/* number of short periods in a long period (low power) */
//#define LONG_PERIOD_MULTIPLIER 14  /* 308 ms */
#define LONG_PERIOD_MULTIPLIER 6  /* 240 ms */
/* number of frames per long period (low power) */
#define LONG_PERIOD_SIZE (SHORT_PERIOD_SIZE * LONG_PERIOD_MULTIPLIER)
/* number of pseudo periods for playback */
#define PLAYBACK_PERIOD_COUNT 4
/* number of periods for capture */
#define CAPTURE_PERIOD_COUNT 4
/* minimum sleep time in out_write() when write threshold is not reached */
#define MIN_WRITE_SLEEP_US 5000

#define RESAMPLER_BUFFER_FRAMES (SHORT_PERIOD_SIZE * 2)
#define RESAMPLER_BUFFER_SIZE (4 * RESAMPLER_BUFFER_FRAMES)

/* android default out sampling rate*/
#define DEFAULT_OUT_SAMPLING_RATE SAMPLING_RATE_44K

/* audio codec default sampling rate*/
#define MM_SAMPLING_RATE SAMPLING_RATE_44K
//#define MM_SAMPLING_RATE  SAMPLING_RATE_48K

/*wifi display buffer size*/
#define AF_BUFFER_SIZE 1024 * 80

enum tty_modes {
    TTY_MODE_OFF,
    TTY_MODE_VCO,
    TTY_MODE_HCO,
    TTY_MODE_FULL
};

struct route_setting
{
    char *ctl_name;
    int intval;
    char *strval;
};


//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------

//#define  SUNXI_DIGITAL_TEST       //调试
#define  START_ZERO_BUFFER          //传输开始前，发起零包传输

#define  SUNXI_DIGITAL_TEST_FILE_SIZE   16777216  //16M
#define  START_ZERO_BUFFER_LEN          4

typedef void *(* voice_thread)(void *param);

struct sunxi_pcm_channel{
    struct pcm * dev;

    struct pcm_config config;
    void *buf;
    unsigned int buf_size;

    unsigned char run;          //flag, 通道正在传输数据
};

struct sunxi_pcm_dev{
    unsigned int card;   /* 设备编号 */

    struct sunxi_pcm_channel in;
    struct sunxi_pcm_channel out;
};

struct sunxi_audio_play{
    sem_t sem;

    pthread_t pid;
    unsigned char thread_run;    //flag, 线程运行标志
    unsigned char play;          //flag, 正在传输标志

    voice_thread func;

#ifdef  SUNXI_DIGITAL_TEST
    FILE *fp;
    unsigned int len;
#endif
};

struct sunxi_digital_audio{
    enum digital_audio_method method;
    char valid;

    struct sunxi_pcm_dev iis;
    struct sunxi_pcm_dev pcm;
    struct sunxi_pcm_dev codec;

    struct sunxi_audio_play up;
    struct sunxi_audio_play down;
};

#endif   //AUDIO_DIGITAL_I_H


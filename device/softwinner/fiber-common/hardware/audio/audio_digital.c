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

#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <pthread.h>
#include <semaphore.h>
#include <alloca.h>
#include <sys/time.h>

#define LOG_TAG "audio_digital"
#define LOG_NDEBUG 0

#include <cutils/log.h>
#include <cutils/properties.h>

#include <tinyalsa/asoundlib.h>

#include "audio_digital.h"
#include "audio_digital_i.h"

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------

#if 1
#define F_LOG ALOGI("%s:%d:\n", __func__, __LINE__);
#else
#define F_LOG {};
#endif

static int stop_audio_play = 0;
static int close_audio_device = 0;
static struct sunxi_digital_audio g_digital_audio;
extern enum digital_audio_method digital_method;

//-----------------------------------------------------------------------------
//  digital audio test
//-----------------------------------------------------------------------------

#ifdef  SUNXI_DIGITAL_TEST

static int sunxi_record(struct sunxi_audio_play *play, void *buf, unsigned int size)
{
    int ret = 0;

    play->len += size;
    if(play->len > SUNXI_DIGITAL_TEST_FILE_SIZE){
        return -1;
    }

    fwrite(buf, 1, size, play->fp);

    return 0;
}

static int sunxi_record_init(struct sunxi_audio_play *play, char *file_name)
{
    FILE *fp = 0;
    char name[128];

    memset(name, 0, 128);
    sprintf(name, "/data/camera/%s", file_name);

    play->fp = fopen(name, "wb+");
    if(play->fp == NULL){
        ALOGE("err: fopen(%s) failed", file_name);
    }

    return 0;
}

static void sunxi_record_exit(struct sunxi_audio_play *play)
{
    play->len = 0;

    if(play->fp){
        fclose(play->fp);
        play->fp = NULL;
    }

    return ;
}

#endif

//-----------------------------------------------------------------------------
//  audio device
//-----------------------------------------------------------------------------

/* pcm in, iis out */
static int sunxi_pcm_to_iis(struct sunxi_pcm_dev *pcm, struct sunxi_pcm_dev *iis)
{
    int size3 = 0;
    int ret   =0;

    F_LOG

    size3 = (pcm->in.buf_size > iis->out.buf_size) ? pcm->in.buf_size : iis->out.buf_size;

#ifdef  SUNXI_DIGITAL_TEST
    sunxi_record_init(&g_digital_audio.up, "pcm_to_iis.pcm");
#endif

#ifdef  START_ZERO_BUFFER
    /* 消除开头杂音 */
    memset(pcm->in.buf, 0, START_ZERO_BUFFER_LEN);
    pcm_write(iis->out.dev, pcm->in.buf, START_ZERO_BUFFER_LEN);
#endif

    while (!stop_audio_play) {
        ret = pcm_read_ex(pcm->in.dev, pcm->in.buf, size3);
        if (ret != 0) {
            ALOGE("err: read pcm err:%s, ret=%d", strerror(errno), ret);
            break;
        }

		if(stop_audio_play){
			ALOGI("stop_audio_play, pcm_to_iis stoped\n");
			break;
		}


#ifdef  SUNXI_DIGITAL_TEST
        sunxi_record(&g_digital_audio.up, pcm->in.buf, size3);
#endif
        iis->out.run = 1;
        ret = pcm_write(iis->out.dev, pcm->in.buf, size3);
        if (ret != 0) {
            ALOGE("err: write iis err:%s, ret=%d", strerror(errno), ret);
            break;
        }
        iis->out.run = 0;
    }

#ifdef  SUNXI_DIGITAL_TEST
    sunxi_record_exit(&g_digital_audio.up);
#endif

    F_LOG

    return 0;
}

/* pcm out, iis in */
static int sunxi_iis_to_pcm(struct sunxi_pcm_dev *iis, struct sunxi_pcm_dev *pcm)
{
    int size3 = 0;
    int ret   =0;

    F_LOG

    size3 = (pcm->out.buf_size > iis->in.buf_size) ? pcm->out.buf_size : iis->in.buf_size;

#ifdef  SUNXI_DIGITAL_TEST
    sunxi_record_init(&g_digital_audio.down, "iis_to_pcm.pcm");
#endif

#ifdef  START_ZERO_BUFFER
    /* 消除开头杂音 */
    memset(iis->in.buf, 0, START_ZERO_BUFFER_LEN);
    pcm_write(pcm->out.dev, iis->in.buf, START_ZERO_BUFFER_LEN);
#endif

    while (!stop_audio_play) {
        ret = pcm_read_ex(iis->in.dev, iis->in.buf, size3);
        if (ret != 0) {
            ALOGE("err: read iis err:%s, ret=%d", strerror(errno), ret);
            break;
        }

		if(stop_audio_play){
			ALOGI("stop_audio_play, iis_to_pcm stoped\n");
			break;
		}

#ifdef  SUNXI_DIGITAL_TEST
        sunxi_record(&g_digital_audio.down, iis->in.buf, size3);
#endif

        pcm->out.run = 1;
        ret = pcm_write(pcm->out.dev, iis->in.buf, size3);
        if (ret != 0) {
            ALOGE("err: write pcm err:%s, ret=%d", strerror(errno), ret);
            break;
        }
        pcm->out.run = 0;
    }

#ifdef  SUNXI_DIGITAL_TEST
    sunxi_record_exit(&g_digital_audio.down);
#endif

    F_LOG

    return 0;
}

static int sunxi_iis_to_codec(struct sunxi_pcm_dev *iis, struct sunxi_pcm_dev *codec)
{
    return 0;
}

static int sunxi_codec_to_iis(struct sunxi_pcm_dev *codec, struct sunxi_pcm_dev *iis)
{
    return 0;
}

static int sunxi_pcm_to_codec(struct sunxi_pcm_dev *pcm, struct sunxi_pcm_dev *codec)
{
    int size3 = 0;
    int ret   =0;

    F_LOG

    size3 = (pcm->in.buf_size > codec->out.buf_size) ? pcm->in.buf_size : codec->out.buf_size;

#ifdef  SUNXI_DIGITAL_TEST
        sunxi_record_init(&g_digital_audio.up, "pcm_to_codec.pcm");
#endif

    while (!stop_audio_play) {
        ret = pcm_read_ex(pcm->in.dev, pcm->in.buf, size3);
        if (ret != 0) {
            ALOGE("err: read pcm err:%s, ret=%d", strerror(errno), ret);
            break;
        }

        if(stop_audio_play){
            ALOGI("stop_audio_play, pcm_to_codec stoped\n");
            break;
        }


#ifdef  SUNXI_DIGITAL_TEST
        sunxi_record(&g_digital_audio.up, pcm->in.buf, size3);
#endif
        codec->out.run = 1;
        ret = pcm_write(codec->out.dev, pcm->in.buf, size3);
        if (ret != 0) {
            ALOGE("err: write codec err:%s, ret=%d", strerror(errno), ret);
            break;
        }
        codec->out.run = 0;
    }

#ifdef  SUNXI_DIGITAL_TEST
    sunxi_record_exit(&g_digital_audio.up);
#endif

    F_LOG

    return 0;
}

static int sunxi_codec_to_pcm(struct sunxi_pcm_dev *codec, struct sunxi_pcm_dev *pcm)
{
    int size3 = 0;
    int ret   =0;

    F_LOG

    size3 = (pcm->out.buf_size > codec->in.buf_size) ? pcm->out.buf_size : codec->in.buf_size;

#ifdef  SUNXI_DIGITAL_TEST
    sunxi_record_init(&g_digital_audio.down, "codec_to_pcm.pcm");
#endif

#ifdef  START_ZERO_BUFFER
    /* 消除开头杂音 */
    memset(codec->in.buf, 0, START_ZERO_BUFFER_LEN);
    pcm_write(pcm->out.dev, codec->in.buf, START_ZERO_BUFFER_LEN);
#endif

    while (!stop_audio_play) {
        ret = pcm_read_ex(codec->in.dev, codec->in.buf, size3);
        if (ret != 0) {
            ALOGE("err: read codec err:%s, ret=%d", strerror(errno), ret);
            break;
        }

        if(stop_audio_play){
            ALOGI("stop_audio_play, codec_to_pcm stoped\n");
            break;
        }

#ifdef  SUNXI_DIGITAL_TEST
        sunxi_record(&g_digital_audio.down, codec->in.buf, size3);
#endif

        pcm->out.run = 1;
        ret = pcm_write(pcm->out.dev, codec->in.buf, size3);
        if (ret != 0) {
            ALOGE("err: write pcm err:%s, ret=%d", strerror(errno), ret);
            break;
        }
        pcm->out.run = 0;
    }

#ifdef  SUNXI_DIGITAL_TEST
    sunxi_record_exit(&g_digital_audio.down);
#endif

    F_LOG

    return 0;
}

static int sunxi_iis_open(struct sunxi_pcm_dev *iis)
{
    F_LOG

    /* in */
    iis->in.config.channels            = 1;
    iis->in.config.rate                = 8000;
    iis->in.config.period_size         = 512;
    iis->in.config.period_count        = 4;
    iis->in.config.format              = PCM_FORMAT_S16_LE;
    iis->in.config.start_threshold     = 0;
    iis->in.config.stop_threshold      = 0;
    iis->in.config.silence_threshold   = 0;

    iis->in.dev = pcm_open(iis->card, PORT_I2S, PCM_IN, &iis->in.config);
    if (!pcm_is_ready(iis->in.dev)) {
        ALOGE("err: Unable to open iis in device (%s)", pcm_get_error(iis->in.dev));
        goto in_dev_open_failed;
    }

    /* out */
    iis->out.config.channels            = 1;
    iis->out.config.rate                = 8000;
    iis->out.config.period_size         = 512;
    iis->out.config.period_count        = 4;
    iis->out.config.format              = PCM_FORMAT_S16_LE;
    iis->out.config.start_threshold     = 0;
    iis->out.config.stop_threshold      = 0;
    iis->out.config.silence_threshold   = 0;

    iis->out.dev = pcm_open(iis->card, PORT_I2S, PCM_OUT, &iis->out.config);
    if (!pcm_is_ready(iis->out.dev)) {
        ALOGE("err: Unable to open iis out device (%s)", pcm_get_error(iis->out.dev));
        goto out_dev_open_failed;
    }

    return 0;

out_dev_open_failed:
    pcm_close(iis->in.dev);

in_dev_open_failed:

    return -1;
}

static int sunxi_iis_close(struct sunxi_pcm_dev *iis)
{
    F_LOG

    if(iis->in.dev){
        pcm_close(iis->in.dev);
        iis->in.dev = NULL;
    }

    if(iis->out.dev){
        pcm_close(iis->out.dev);
        iis->out.dev = NULL;
    }

    return 0;
}

static int sunxi_pcm_open(struct sunxi_pcm_dev *pcm)
{
    F_LOG

    /* in */
    pcm->in.config.channels             = 1;
    pcm->in.config.rate                 = 8000;
    pcm->in.config.period_size          = 512;
    pcm->in.config.period_count         = 4;
    pcm->in.config.format               = PCM_FORMAT_S16_LE;
    pcm->in.config.start_threshold      = 0;
    pcm->in.config.stop_threshold       = 0;
    pcm->in.config.silence_threshold    = 0;

    pcm->in.dev = pcm_open(pcm->card, PORT_PCM, PCM_IN, &pcm->in.config);
    if (!pcm_is_ready(pcm->in.dev)) {
        ALOGE("err: Unable to open pcm in device (%s)", pcm_get_error(pcm->in.dev));
        goto in_dev_open_failed;
    }

    /* out */
    pcm->out.config.channels            = 1;
    pcm->out.config.rate                = 8000;
    pcm->out.config.period_size         = 512;
    pcm->out.config.period_count        = 4;
    pcm->out.config.format              = PCM_FORMAT_S16_LE;
    pcm->out.config.start_threshold     = 0;
    pcm->out.config.stop_threshold      = 0;
    pcm->out.config.silence_threshold   = 0;

    pcm->out.dev = pcm_open(pcm->card , PORT_PCM, PCM_OUT, &pcm->out.config);
    if (!pcm_is_ready(pcm->out.dev)) {
        ALOGE("err: Unable to open pcm out device (%s)", pcm_get_error(pcm->out.dev));
        goto out_dev_open_failed;
    }

    return 0;

out_dev_open_failed:
    pcm_close(pcm->in.dev);

in_dev_open_failed:

    return -1;
}

static int sunxi_pcm_close(struct sunxi_pcm_dev *pcm)
{
    F_LOG

    if(pcm->in.dev){
        pcm_close(pcm->in.dev);
        pcm->in.dev = NULL;
    }

    if(pcm->out.dev){
        pcm_close(pcm->out.dev);
        pcm->out.dev = NULL;
    }

    return 0;
}

static int sunxi_codec_open(struct sunxi_pcm_dev *codec)
{
    F_LOG

    /* in */
    codec->in.config.channels             = 1;
    codec->in.config.rate                 = 8000;
    codec->in.config.period_size          = 512;
    codec->in.config.period_count         = 4;
    codec->in.config.format               = PCM_FORMAT_S16_LE;
    codec->in.config.start_threshold      = 0;
    codec->in.config.stop_threshold       = 0;
    codec->in.config.silence_threshold    = 0;

    codec->in.dev = pcm_open(codec->card, PORT_CODEC, PCM_IN, &codec->in.config);
    if (!pcm_is_ready(codec->in.dev)) {
        ALOGE("err: Unable to open codec in device (%s)", pcm_get_error(codec->in.dev));
        goto in_dev_open_failed;
    }

    /* out */
    codec->out.config.channels            = 1;
    codec->out.config.rate                = 8000;
    codec->out.config.period_size         = 512;
    codec->out.config.period_count        = 4;
    codec->out.config.format              = PCM_FORMAT_S16_LE;
    codec->out.config.start_threshold     = 0;
    codec->out.config.stop_threshold      = 0;
    codec->out.config.silence_threshold   = 0;

    codec->out.dev = pcm_open(codec->card, PORT_CODEC, PCM_OUT, &codec->out.config);
    if (!pcm_is_ready(codec->out.dev)) {
        ALOGE("err: Unable to open codec out device (%s)", pcm_get_error(codec->out.dev));
        goto out_dev_open_failed;
    }

    return 0;

out_dev_open_failed:
    pcm_close(codec->in.dev);

in_dev_open_failed:

    return -1;
}

static int sunxi_codec_close(struct sunxi_pcm_dev *codec)
{
    F_LOG

    if(codec->in.dev){
        pcm_close(codec->in.dev);
        codec->in.dev = NULL;
    }

    if(codec->out.dev){
        pcm_close(codec->out.dev);
        codec->out.dev = NULL;
    }

    return 0;
}

static int sunxi_iis_init(struct sunxi_pcm_dev *iis)
{
    int ret = 0;

    ret = pcm_get_node_number("sndi2s");
    if (ret < 0) {
        ALOGE("err: get sndi2s node number failed ");
        goto pcm_get_node_number_failed;
    }

    iis->card = ret;
    ALOGI("iis->card=%d\n", iis->card);

    ret = sunxi_iis_open(iis);
    if (ret != 0) {
        ALOGE("err: Unable to open iis in device (%s)", pcm_get_error(iis->in.dev));
        goto sunxi_iis_open_failed;
    }

    /* in */
    iis->in.buf_size = pcm_get_buffer_size(iis->in.dev);
    iis->in.buf = (void *)calloc(1, iis->in.buf_size);
    if (iis->in.buf == NULL) {
        ALOGE("Unable to iis in allocate %d bytes", iis->in.buf_size);
        goto in_buff_malloc_failed;
    }
    memset(iis->in.buf, 0, iis->in.buf_size);

    /* out */
    iis->out.buf_size = pcm_get_buffer_size(iis->out.dev);
    iis->out.buf = (void *)calloc(1, iis->out.buf_size);
    if (iis->out.buf == NULL) {
        ALOGE("Unable to iis out allocate %d bytes", iis->out.buf_size);
        goto out_buff_malloc_failed;
    }
    memset(iis->out.buf, 0, iis->out.buf_size);

    return 0;

out_buff_malloc_failed:
    free(iis->in.buf);

in_buff_malloc_failed:
    sunxi_iis_close(iis);

sunxi_iis_open_failed:
pcm_get_node_number_failed:

    return -1;
}

static int sunxi_iis_exit(struct sunxi_pcm_dev *iis)
{
    if(iis->in.buf){
        free(iis->in.buf);
        iis->in.buf = NULL;
    }

    if(iis->out.buf){
        free(iis->out.buf);
        iis->out.buf = NULL;
    }

    sunxi_iis_close(iis);

    return 0;
}

static int sunxi_pcm_init(struct sunxi_pcm_dev *pcm)
{
    int ret = 0;

    ret = pcm_get_node_number("sndpcm");
    if (ret < 0) {
        ALOGE("err: get sndpcm node number failed ");
        goto pcm_get_node_number_failed;
    }

    pcm->card = ret;
    ALOGI("pcm->card=%d\n", pcm->card);

    ret = sunxi_pcm_open(pcm);
    if (ret != 0) {
        ALOGE("err: sunxi_pcm_open failed");
        goto in_dev_open_failed;
    }

    /* in */
    pcm->in.buf_size = pcm_get_buffer_size(pcm->in.dev);
    pcm->in.buf = (void *)calloc(1, pcm->in.buf_size);
    if (pcm->in.buf == NULL) {
        ALOGE("Unable to pcm in allocate %d bytes", pcm->in.buf_size);
        goto in_buff_malloc_failed;
    }
    memset(pcm->in.buf, 0, pcm->in.buf_size);

    /* out */
    pcm->out.buf_size = pcm_get_buffer_size(pcm->out.dev);
    pcm->out.buf = (void *)calloc(1, pcm->out.buf_size);
    if (pcm->out.buf == NULL) {
        ALOGE("Unable to pcm out allocate %d bytes", pcm->out.buf_size);
        goto out_buff_malloc_failed;
    }
    memset(pcm->out.buf, 0, pcm->out.buf_size);

    return 0;

out_buff_malloc_failed:
    free(pcm->in.buf);

in_buff_malloc_failed:
    sunxi_pcm_close(pcm);

in_dev_open_failed:
pcm_get_node_number_failed:

    return -1;
}

static int sunxi_pcm_exit(struct sunxi_pcm_dev *pcm)
{
    if(pcm->in.buf){
        free(pcm->in.buf);
        pcm->in.buf = NULL;
    }

    if(pcm->out.buf){
        free(pcm->out.buf);
        pcm->out.buf = NULL;
    }

    sunxi_pcm_close(pcm);

    return 0;
}

static int sunxi_codec_init(struct sunxi_pcm_dev *codec)
{
    int ret = 0;

    ret = pcm_get_node_number("audiocodec");
    if (ret < 0) {
        ALOGE("err: get audiocodec node number failed ");
        goto pcm_get_node_number_failed;
    }

    codec->card = ret;
    ALOGI("codec->card=%d\n", codec->card);

    ret = sunxi_codec_open(codec);
    if (ret != 0) {
        ALOGE("err: Unable to open codec in device (%s)", pcm_get_error(codec->in.dev));
        goto sunxi_codec_open_failed;
    }

    /* in */
    codec->in.buf_size = pcm_get_buffer_size(codec->in.dev);
    codec->in.buf = (void *)calloc(1, codec->in.buf_size);
    if (codec->in.buf == NULL) {
        ALOGE("Unable to codec in allocate %d bytes", codec->in.buf_size);
        goto in_buff_malloc_failed;
    }
    memset(codec->in.buf, 0, codec->in.buf_size);

    /* out */
    codec->out.buf_size = pcm_get_buffer_size(codec->out.dev);
    codec->out.buf = (void *)calloc(1, codec->out.buf_size);
    if (codec->out.buf == NULL) {
        ALOGE("Unable to codec out allocate %d bytes", codec->out.buf_size);
        goto out_buff_malloc_failed;
    }
    memset(codec->out.buf, 0, codec->out.buf_size);

    return 0;

out_buff_malloc_failed:
    free(codec->in.buf);

in_buff_malloc_failed:
    sunxi_codec_close(codec);

sunxi_codec_open_failed:
pcm_get_node_number_failed:

    return -1;
}

static int sunxi_codec_exit(struct sunxi_pcm_dev *codec)
{
    if(codec->in.buf){
        free(codec->in.buf);
        codec->in.buf = NULL;
    }

    if(codec->out.buf){
        free(codec->out.buf);
        codec->out.buf = NULL;
    }

    sunxi_codec_close(codec);

    return 0;
}

//-----------------------------------------------------------------------------
//  method
//-----------------------------------------------------------------------------

/* BP -> AP -> (bluetooth/headset/speaker) */
static void *sunxi_voice_down_thread(void *param)
{
    struct sunxi_digital_audio *digital = (struct sunxi_digital_audio *)param;
    int ret = 0;

    F_LOG

    while(digital->down.thread_run){
		ALOGD("sunxi_voice_down_thread sleep\n");
        sem_wait(&digital->down.sem);
        ALOGD("sunxi_voice_down_thread wakeup\n");

		if(!stop_audio_play){
            /* audio play */
            digital->down.play = 1;
            if(digital->method == DIGITAL_AUDIO_METHOD_1){
                sunxi_iis_to_pcm(&digital->iis, &digital->pcm);
            }else if(digital->method == DIGITAL_AUDIO_METHOD_2){
                sunxi_codec_to_pcm(&digital->codec, &digital->pcm);
            }else if(digital->method == DIGITAL_AUDIO_METHOD_3){
                sunxi_iis_to_codec(&digital->iis, &digital->codec);
            }
            digital->down.play = 0;
		}

        /* audio play end, close audio device */
        if(close_audio_device){
			while(digital->down.play || digital->up.play){
				ALOGD("wait for play stop, up=%d, down=%d\n", digital->up.play, digital->down.play);
				usleep(1000);
			}

            /* close device */
            if(digital->method == DIGITAL_AUDIO_METHOD_1){
                /* iis & pcm */
                sunxi_iis_close(&digital->iis);
                sunxi_pcm_close(&digital->pcm);
            }else if(digital->method == DIGITAL_AUDIO_METHOD_2){
                /* codec & pcm */
                sunxi_iis_close(&digital->codec);
                sunxi_pcm_close(&digital->pcm);
            }else if(digital->method == DIGITAL_AUDIO_METHOD_3){
                /* codec & iis */
                sunxi_iis_close(&digital->codec);
                sunxi_pcm_close(&digital->iis);
            }
        }
    }

    ALOGD("sunxi_voice_down_thread end\n");

    return param;
}

/* (bluetooth/mic) -> AP -> BP */
static void *sunxi_voice_up_thread(void *param)
{
    struct sunxi_digital_audio *digital = (struct sunxi_digital_audio *)param;
    int ret = 0;

    F_LOG

    while(digital->up.thread_run){
		ALOGD("sunxi_voice_up_thread sleep\n");
        sem_wait(&digital->up.sem);
        ALOGD("sunxi_voice_up_thread wakeup\n");

        if(!stop_audio_play){
            /* audio play */
            digital->up.play = 1;
            if(digital->method == DIGITAL_AUDIO_METHOD_1){
                sunxi_pcm_to_iis(&digital->pcm, &digital->iis);
            }else if(digital->method == DIGITAL_AUDIO_METHOD_2){
                sunxi_pcm_to_codec(&digital->pcm, &digital->codec);
            }else if(digital->method == DIGITAL_AUDIO_METHOD_3){
                sunxi_codec_to_iis(&digital->codec, &digital->iis);
            }
            digital->up.play = 0;
		}
    }

    ALOGD("sunxi_voice_up_thread end\n");

    return param;
}

static int sunxi_audio_play_start(struct sunxi_digital_audio *digital)
{
    int ret = 0;

    stop_audio_play = 0;

    if(stop_audio_play || digital->up.play || digital->down.play){
		ALOGE("wrn, voice can not start, stop_audio_play=%d, digital->up.play=%d, digital->down.play=%d",
		      stop_audio_play, digital->up.play, digital->down.play);
		return 0;
    }

    /* open device */
    if(digital->method == DIGITAL_AUDIO_METHOD_1){
        /* iis & pcm */
        ret = sunxi_iis_open(&digital->iis);
        if(ret){
            ALOGE("err: down sunxi_iis_open failed, ret=%d\n", ret);
            return -1;
        }

        ret = sunxi_pcm_open(&digital->pcm);
        if(ret){
            ALOGE("err: down sunxi_pcm_open failed, ret=%d\n", ret);
            sunxi_iis_close(&digital->iis);
            return -1;
        }
    }else if(digital->method == DIGITAL_AUDIO_METHOD_2){
        /* codec & pcm */
        ret = sunxi_codec_open(&digital->codec);
        if(ret){
            ALOGE("err: down sunxi_codec_open failed, ret=%d\n", ret);
            return -1;
        }

        ret = sunxi_pcm_open(&digital->pcm);
        if(ret){
            ALOGE("err: down sunxi_pcm_open failed, ret=%d\n", ret);
            sunxi_codec_close(&digital->codec);
            return -1;
        }
    }else if(digital->method == DIGITAL_AUDIO_METHOD_3){
        /* codec & iis */
        ret = sunxi_codec_open(&digital->codec);
        if(ret){
            ALOGE("err: down sunxi_codec_open failed, ret=%d\n", ret);
            return -1;
        }

        ret = sunxi_iis_open(&digital->iis);
        if(ret){
            ALOGE("err: down sunxi_iis_open failed, ret=%d\n", ret);
            sunxi_codec_close(&digital->codec);
            return -1;
        }
    }

    /* audio play */
    sem_post(&digital->up.sem);
    sem_post(&digital->down.sem);

    return 0;
}

int sunxi_audio_play_stop(struct sunxi_digital_audio *digital)
{
	F_LOG

    stop_audio_play = 1;

    /*
     * 以下两种情况下，需特殊处理 :
     * 1、对方挂断电话时，等待最后一笔PCM数据发送给蓝牙后，关闭通道。
     * 2、己方挂断电话时，等待最后一笔iis数据发送给模组后，关闭通道。
     *
     */
    if(digital->down.play || digital->up.play){
        char iis_stop = 0;
        char pcm1_stop = 0;
        char codec_stop = 0;

        /*
         * delay 1s for pcm_read complete.
         * if iis or pcm unfinished, stop it.
         *
         */
        usleep(1000 * 1000);

        if(digital->method == DIGITAL_AUDIO_METHOD_1){
            iis_stop = 1;
            pcm1_stop = 1;
        }else if(digital->method == DIGITAL_AUDIO_METHOD_2){
            codec_stop = 1;
            pcm1_stop = 1;
        }else if(digital->method == DIGITAL_AUDIO_METHOD_3){
            iis_stop = 1;
            codec_stop = 1;
        }

        if(iis_stop){
            pcm_stop(digital->iis.in.dev);
            pcm_stop(digital->iis.out.dev);
        }

        if(pcm1_stop){
            pcm_stop(digital->pcm.in.dev);

            while(digital->pcm.out.run){
                ALOGD("wait for last pcm data complete\n");
                usleep(1000);
            }
        }

        if(codec_stop){
            pcm_stop(digital->codec.in.dev);
            pcm_stop(digital->codec.out.dev);
        }
    }

    /* stop audio play, and close audio device */
    close_audio_device = 1;
    sem_post(&digital->up.sem);
    sem_post(&digital->down.sem);

    return 0;
}

static int sunxi_audio_play_init(struct sunxi_audio_play *play, voice_thread func, void *parg)
{
	int ret;
    pthread_attr_t attr;

    play->thread_run = 1;
    play->play = 0;
    play->func = func;

    ret = sem_init(&play->sem, 0, 0);
    if (ret) {
        ALOGE("err: sem_init failed, ret=%d\n", ret);
        goto sem_init_failed;
    }

    ret = pthread_attr_init (&attr);
    if (ret != 0) {
        ALOGE("err: pthread_attr_init failed err=%s", strerror(ret));
        goto pthread_attr_init_failed;
    }

    ret = pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
    if (ret != 0) {
        ALOGE("err: pthread_attr_setdetachstate failed err=%s", strerror(ret));
        goto pthread_attr_setdetachstate_failed;
    }

	ret = pthread_create(&play->pid, &attr, play->func, parg);
	if (ret) {
		ALOGE("err: pthread_create failed, ret=%d\n", ret);
		goto pthread_create_failed;
	}

    return 0;

pthread_create_failed:
pthread_attr_setdetachstate_failed:
pthread_attr_init_failed:

    sem_destroy(&play->sem);

sem_init_failed:

    return -1;
}

static int sunxi_audio_play_exit(struct sunxi_audio_play *play)
{
    void *retval;

    play->thread_run = 0;
    sem_post(&play->sem);

/*
    if(pthread_join(play->pid, &retval)) {
        ALOGE("err: pthread_join failed\n");
    }
*/

    sem_destroy(&play->sem);

    return 0;
}

/* method1 : (BP)--iis--(AP)--pcm--(BT) */
static int sunxi_audio_method1_init(struct sunxi_digital_audio *digital)
{
    int ret = 0;

    /* iis init */
    ret = sunxi_iis_init(&digital->iis);
    if(ret != 0){
        ALOGE("err: sunxi_iis_init failed, ret=%d\n", ret);
        goto iis_init_failed;
    }

    /* pcm init */
    ret = sunxi_pcm_init(&digital->pcm);
    if(ret != 0){
        ALOGE("err: sunxi_pcm_init failed, ret=%d\n", ret);
        goto pcm_init_failed;
    }

    /* voice up init */
    ret = sunxi_audio_play_init(&digital->up, sunxi_voice_up_thread, digital);
    if(ret != 0){
        ALOGE("err: voice up init failed, ret=%d\n", ret);
        goto voice_up_init_failed;
    }

    /* voice down init */
    ret = sunxi_audio_play_init(&digital->down, sunxi_voice_down_thread, digital);
    if(ret != 0){
        ALOGE("err: voice down init failed, ret=%d\n", ret);
        goto voice_down_init_failed;
    }

    /* close iis & pcm */
    sunxi_iis_close(&digital->iis);
    sunxi_pcm_close(&digital->pcm);

    return 0;

voice_down_init_failed:
    sunxi_audio_play_exit(&digital->up);

voice_up_init_failed:
    sunxi_pcm_exit(&digital->pcm);

pcm_init_failed:
    sunxi_iis_exit(&digital->iis);

iis_init_failed:

    return -1;
}

static int sunxi_audio_method1_exit(struct sunxi_digital_audio *digital)
{
    sunxi_audio_play_exit(&digital->up);
    sunxi_audio_play_exit(&digital->down);
    sunxi_pcm_exit(&digital->pcm);
    sunxi_iis_exit(&digital->iis);

    return 0;
}

/* method2 : (BP)--phonein/phoneout--(AP)--pcm--(BT) */
static int sunxi_audio_method2_init(struct sunxi_digital_audio *digital)
{
    int ret = 0;

    /* codec init */
    ret = sunxi_codec_init(&digital->codec);
    if(ret != 0){
        ALOGE("err: sunxi_codec_init failed, ret=%d\n", ret);
        goto codec_init_failed;
    }

    /* pcm init */
    ret = sunxi_pcm_init(&digital->pcm);
    if(ret != 0){
        ALOGE("err: sunxi_pcm_init failed, ret=%d\n", ret);
        goto pcm_init_failed;
    }

    /* voice up init */
    ret = sunxi_audio_play_init(&digital->up, sunxi_voice_up_thread, digital);
    if(ret != 0){
        ALOGE("err: voice up init failed, ret=%d\n", ret);
        goto voice_up_init_failed;
    }

    /* voice down init */
    ret = sunxi_audio_play_init(&digital->down, sunxi_voice_down_thread, digital);
    if(ret != 0){
        ALOGE("err: voice down init failed, ret=%d\n", ret);
        goto voice_down_init_failed;
    }

    /* close codec & pcm */
    sunxi_codec_close(&digital->codec);
    sunxi_pcm_close(&digital->pcm);

    return 0;

voice_down_init_failed:
    sunxi_audio_play_exit(&digital->up);

voice_up_init_failed:
    sunxi_pcm_exit(&digital->pcm);

pcm_init_failed:
    sunxi_codec_exit(&digital->codec);

codec_init_failed:

    return -1;
}

static int sunxi_audio_method2_exit(struct sunxi_digital_audio *digital)
{
    sunxi_audio_play_exit(&digital->up);
    sunxi_audio_play_exit(&digital->down);
    sunxi_pcm_exit(&digital->pcm);
    sunxi_codec_exit(&digital->codec);

    return 0;
}

/* method3 : (BP)--iis--(AP)--lineout--(speaker/handset/earphone) */
static int sunxi_audio_method3_init(struct sunxi_digital_audio *digital)
{
    return 0;
}

static int sunxi_audio_method3_exit(struct sunxi_digital_audio *digital)
{
    return 0;
}

//-----------------------------------------------------------------------------
//  digital_audio
//-----------------------------------------------------------------------------

int sunxi_digital_audio_start(void)
{
    struct sunxi_digital_audio *digital = &g_digital_audio;

    if(!digital->valid){
        return -1;
    }

    return sunxi_audio_play_start(digital);
}

int sunxi_digital_audio_stop(void)
{
    struct sunxi_digital_audio *digital = &g_digital_audio;

    if(!digital->valid){
        return -1;
    }

    return sunxi_audio_play_stop(digital);
}

int sunxi_digital_audio_init(void)
{
    struct sunxi_digital_audio *digital = &g_digital_audio;
    char method[10];
    int ret = 0;

    F_LOG

    /* get audio method */
    memset(digital, 0, sizeof(struct sunxi_digital_audio));
    ret = property_get("ro.sw.audio.method", method, "0");
    if(ret <= 0){
        digital->method = DIGITAL_AUDIO_METHOD_UNKOWN;
		ALOGE("wrn: get ro.sw.audio.method failed\n");
		digital->valid = 0;
        return -1;
    }

    if(!strcmp(method, "1")){
        digital->method = DIGITAL_AUDIO_METHOD_1;
        ret = sunxi_audio_method1_init(digital);
        if(ret != 0){
            ALOGE("err: sunxi_audio_method1_init failed\n");
            digital->valid = 0;
            return -1;
        }
    }else if(!strcmp(method, "2")){
        digital->method = DIGITAL_AUDIO_METHOD_2;
        ret = sunxi_audio_method2_init(digital);
        if(ret != 0){
            ALOGE("err: sunxi_audio_method2_init failed\n");
            digital->valid = 0;
            return -1;
        }
    }else if(!strcmp(method, "3")){
        digital->method = DIGITAL_AUDIO_METHOD_3;
        sunxi_audio_method3_init(digital);
        if(ret != 0){
            ALOGE("err: sunxi_audio_method3_init failed\n");
            digital->valid = 0;
            return -1;
        }
    }else{
		ALOGE("wrn: ro.sw.audio.method is unkown, %s\n", method);
        digital->method = DIGITAL_AUDIO_METHOD_UNKOWN;
        digital->valid = 0;
        return -1;
    }

    digital->valid = 1;
    digital_method = digital->method;

    ALOGI("digital->method=%d, digital->valid=%d\n", digital->method, digital->valid);

    return 0;
}

int sunxi_digital_audio_exit(void)
{
    struct sunxi_digital_audio *digital = &g_digital_audio;

    if(!digital->valid){
        return -1;
    }

    digital_method = 0;

    switch(digital->method){
        case DIGITAL_AUDIO_METHOD_1:
            sunxi_audio_method1_exit(digital);
        break;

        case DIGITAL_AUDIO_METHOD_2:
            sunxi_audio_method2_exit(digital);
        break;

        case DIGITAL_AUDIO_METHOD_3:
            sunxi_audio_method3_exit(digital);
        break;

        default:
            //sunxi_audio_method1_exit(digital);
            ALOGE("wrn: unkown method(%d)\n", digital->method);
    }

    return 0;
}



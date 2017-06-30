//
// Created by gutou on 2017/4/20.
//

#include "xl_audio_player.h"
#include "../xl_container/xl_frame_queue.h"
#include "../xl_container/xl_frame_pool.h"
#include "../xl_utils/xl_clock.h"
#include <SLES/OpenSLES.h>
#include <SLES/OpenSLES_Android.h>
#include <assert.h>
#include <pthread.h>

// engine interfaces
static SLObjectItf engineObject = NULL;
static SLEngineItf engineEngine;

// output mix interfaces
static SLObjectItf outputMixObject = NULL;

// buffer queue player interfaces
static SLObjectItf bqPlayerObject = NULL;
static SLPlayItf bqPlayerPlay;
static SLAndroidSimpleBufferQueueItf bqPlayerBufferQueue;
static SLVolumeItf bqPlayerVolume;


static int64_t get_delta_time(xl_audio_player_context * ctx) {
    SLmillisecond sec;
    (*bqPlayerPlay)->GetPosition(bqPlayerPlay, &sec);
    if (sec > ctx->play_pos) {
        return (int64_t) (sec - ctx->play_pos) * 1000;
    }
    return 0;
}

static int get_audio_frame(xl_play_data * pd) {
    xl_audio_player_context * ctx = pd->audio_player_ctx;
    if (pd->status == IDEL || pd->status == PAUSED || pd->status == BUFFER_EMPTY) {
        return -1;
    }
    pd->audio_frame = xl_frame_queue_get(pd->audio_frame_queue);
    // buffer empty ==> return -1
    // eos          ==> return -1
    if (pd->audio_frame == NULL) {
        // 如果没有视频流  就从这里发结束信号
        if (pd->eof && ((pd->av_track_flags & XL_HAS_VIDEO_FLAG) == 0) || pd->just_audio) {
            pd->send_message(pd, xl_message_stop);
        }
        return -1;
    }
    // seek
    // get next frame
    while (pd->audio_frame == &pd->audio_frame_queue->flush_frame) {
        // 如果没有视频流  就从这里重置seek标记
        if((pd->av_track_flags & XL_HAS_VIDEO_FLAG) == 0){
            pd->seeking = 0;
        }
        return get_audio_frame(pd);
    }

    ctx->frame_size = av_samples_get_buffer_size(NULL, pd->audio_frame->channels, pd->audio_frame->nb_samples,
                                                     AV_SAMPLE_FMT_S16, 1);
    // filter will rewrite the frame's pts.  use  ptk_dts instead.
    int64_t time_stamp = av_rescale_q(pd->audio_frame->pkt_dts,
                                      pd->format_context->streams[pd->audio_index]->time_base,
                                      AV_TIME_BASE_Q);
    if(ctx->buffer_size < ctx->frame_size){
        ctx->buffer_size = ctx->frame_size;
        if(ctx->buffer == NULL){
            ctx->buffer = malloc((size_t) ctx->buffer_size);
        }else{
            ctx->buffer = realloc(ctx->buffer, (size_t) ctx->buffer_size);
        }
    }
    if(ctx->frame_size > 0){
        memcpy(ctx->buffer, pd->audio_frame->data[0], (size_t) ctx->frame_size);
    }
    xl_frame_pool_unref_frame(pd->audio_frame_pool, pd->audio_frame);
    xl_clock_set(pd->audio_clock, time_stamp);
    return 0;
}

static void bqPlayerCallback(SLAndroidSimpleBufferQueueItf bq, void *context) {
    xl_play_data * pd = context;
    xl_audio_player_context * ctx = pd->audio_player_ctx;
    pthread_mutex_lock(ctx->lock);
    assert(bq == bqPlayerBufferQueue);
    if (-1 == get_audio_frame(pd)) {
        (*bqPlayerPlay)->SetPlayState(bqPlayerPlay, SL_PLAYSTATE_PAUSED);
        pthread_mutex_unlock(ctx->lock);
        return;
    }
    // for streaming playback, replace this test by logic to find and fill the next buffer
    if (NULL != ctx->buffer && 0 != ctx->frame_size) {
        SLresult result;
        // enqueue another buffer
        result = (*bqPlayerBufferQueue)->Enqueue(bqPlayerBufferQueue, ctx->buffer,
                                                 (SLuint32) ctx->frame_size);
        (*bqPlayerPlay)->GetPosition(bqPlayerPlay, &ctx->play_pos);
        (void) result;
    }
    pthread_mutex_unlock(ctx->lock);
}

void xl_audio_play(xl_play_data * pd) {
    SLresult result = 0;
    (*bqPlayerPlay)->GetPlayState(bqPlayerPlay, &result);
    if(result == SL_PLAYSTATE_PAUSED){
        (*bqPlayerPlay)->SetPlayState(bqPlayerPlay, SL_PLAYSTATE_PLAYING);
        bqPlayerCallback(bqPlayerBufferQueue, pd);
    }
}

void xl_audio_player_shutdown();

void xl_audio_player_release(xl_audio_player_context * ctx);

void xl_audio_player_create(int rate, int channel, xl_play_data * pd);

xl_audio_player_context *xl_audio_engine_create() {
    SLresult result;
    // create engine
    result = slCreateEngine(&engineObject, 0, NULL, 0, NULL, NULL);
    assert(SL_RESULT_SUCCESS == result);
    (void) result;
    // realize the engine
    result = (*engineObject)->Realize(engineObject, SL_BOOLEAN_FALSE);
    assert(SL_RESULT_SUCCESS == result);
    (void) result;
    // get the engine interface, which is needed in order to create other objects
    result = (*engineObject)->GetInterface(engineObject, SL_IID_ENGINE, &engineEngine);
    assert(SL_RESULT_SUCCESS == result);
    (void) result;
    // create output mix, with environmental reverb specified as a non-required interface
    result = (*engineEngine)->CreateOutputMix(engineEngine, &outputMixObject, 0, NULL, NULL);
    assert(SL_RESULT_SUCCESS == result);
    (void) result;
    // realize the output mix
    result = (*outputMixObject)->Realize(outputMixObject, SL_BOOLEAN_FALSE);
    assert(SL_RESULT_SUCCESS == result);
    (void) result;
    // ignore unsuccessful result codes for environmental reverb, as it is optional for this example

    xl_audio_player_context * ctx = malloc(sizeof(xl_audio_player_context));
    ctx->play = xl_audio_play;
    ctx->shutdown = xl_audio_player_shutdown;
    ctx->release = xl_audio_player_release;
    ctx->player_create = xl_audio_player_create;
    ctx->get_delta_time = get_delta_time;
    ctx->lock = (pthread_mutex_t *) malloc(sizeof(pthread_mutex_t));
    pthread_mutex_init(ctx->lock, NULL);
    return ctx;
}

void xl_audio_player_create(int rate, int channel, xl_play_data * pd) {
    SLresult result;
    // configure audio source
    SLDataLocator_AndroidSimpleBufferQueue loc_bufq = {SL_DATALOCATOR_ANDROIDSIMPLEBUFFERQUEUE, 2};
    /**
     *  SLuint32 		formatType;
	 *  SLuint32 		numChannels;
	 *  SLuint32 		samplesPerSec;
	 *  SLuint32 		bitsPerSample;
	 *  SLuint32 		containerSize;
	 *  SLuint32 		channelMask;
	 *  SLuint32		endianness;
     */
    SLDataFormat_PCM format_pcm = {SL_DATAFORMAT_PCM, 1, SL_SAMPLINGRATE_8,
                                   SL_PCMSAMPLEFORMAT_FIXED_16, SL_PCMSAMPLEFORMAT_FIXED_16,
                                   SL_SPEAKER_FRONT_CENTER, SL_BYTEORDER_LITTLEENDIAN};
    format_pcm.numChannels = (SLuint32) channel;
    format_pcm.samplesPerSec = (SLuint32) (rate * 1000);
    if (channel == 2) {
        format_pcm.channelMask = SL_SPEAKER_FRONT_LEFT | SL_SPEAKER_FRONT_RIGHT;
    } else {
        format_pcm.channelMask = SL_SPEAKER_FRONT_CENTER;
    }
    SLDataSource audioSrc = {&loc_bufq, &format_pcm};

    // configure audio sink
    SLDataLocator_OutputMix loc_outmix = {SL_DATALOCATOR_OUTPUTMIX, outputMixObject};
    SLDataSink audioSnk = {&loc_outmix, NULL};

    /*
     * create audio player:
     *     fast audio does not support when SL_IID_EFFECTSEND is required, skip it
     *     for fast audio case
     */
    const SLInterfaceID ids[2] = {SL_IID_ANDROIDSIMPLEBUFFERQUEUE, SL_IID_VOLUME};
    const SLboolean req[2] = {SL_BOOLEAN_TRUE, SL_BOOLEAN_TRUE};

    result = (*engineEngine)->CreateAudioPlayer(engineEngine, &bqPlayerObject, &audioSrc, &audioSnk,
                                                2, ids, req);
    assert(SL_RESULT_SUCCESS == result);
    (void) result;

    // realize the player
    result = (*bqPlayerObject)->Realize(bqPlayerObject, SL_BOOLEAN_FALSE);
    assert(SL_RESULT_SUCCESS == result);
    (void) result;

    // get the play interface
    result = (*bqPlayerObject)->GetInterface(bqPlayerObject, SL_IID_PLAY, &bqPlayerPlay);
    assert(SL_RESULT_SUCCESS == result);
    (void) result;

    // get the buffer queue interface
    result = (*bqPlayerObject)->GetInterface(bqPlayerObject, SL_IID_ANDROIDSIMPLEBUFFERQUEUE,
                                             &bqPlayerBufferQueue);
    assert(SL_RESULT_SUCCESS == result);
    (void) result;

    // register callback on the buffer queue
    result = (*bqPlayerBufferQueue)->RegisterCallback(bqPlayerBufferQueue, bqPlayerCallback, pd);
    assert(SL_RESULT_SUCCESS == result);
    (void) result;

    // get the volume interface
    result = (*bqPlayerObject)->GetInterface(bqPlayerObject, SL_IID_VOLUME, &bqPlayerVolume);
    assert(SL_RESULT_SUCCESS == result);
    (void) result;
    // get the playbackRate interface
//    result = (*bqPlayerObject)->GetInterface(bqPlayerObject, SL_IID_PLAYBACKRATE, &playbackRateItf);
//    assert(SL_RESULT_SUCCESS == result);
//    (void) result;
    result = (*bqPlayerPlay)->SetPlayState(bqPlayerPlay, SL_PLAYSTATE_PAUSED);
    assert(SL_RESULT_SUCCESS == result);
    (void) result;
}

void xl_audio_player_shutdown() {
    if(bqPlayerPlay != NULL){
        (*bqPlayerPlay)->SetPlayState(bqPlayerPlay, SL_PLAYSTATE_STOPPED);
    }
}

void xl_audio_player_release(xl_audio_player_context * ctx) {
    // destroy buffer queue audio player object, and invalidate all associated interfaces
    if (bqPlayerObject != NULL) {
        (*bqPlayerObject)->Destroy(bqPlayerObject);
        bqPlayerObject = NULL;
        bqPlayerPlay = NULL;
        bqPlayerBufferQueue = NULL;
        bqPlayerVolume = NULL;
    }
    // destroy output mix object, and invalidate all associated interfaces
    if (outputMixObject != NULL) {
        (*outputMixObject)->Destroy(outputMixObject);
        outputMixObject = NULL;
    }

    // destroy engine object, and invalidate all associated interfaces
    if (engineObject != NULL) {
        (*engineObject)->Destroy(engineObject);
        engineObject = NULL;
        engineEngine = NULL;
    }

    if (ctx->lock != NULL) {
        pthread_mutex_destroy(ctx->lock);
        free(ctx->lock);
    }
    if(ctx->buffer != NULL){
        free(ctx->buffer);
    }
    free(ctx);
}
#include "AudioALSAPlaybackHandlerVoice.h"

#include "SpeechDriverFactory.h"
#include "SpeechBGSPlayer.h"


#define LOG_TAG "AudioALSAPlaybackHandlerVoice"

namespace android
{

AudioALSAPlaybackHandlerVoice::AudioALSAPlaybackHandlerVoice(const stream_attribute_t *stream_attribute_source) :
    AudioALSAPlaybackHandlerBase(stream_attribute_source),
    mBGSPlayer(BGSPlayer::GetInstance())
 /*lenovo-sw fuxm1 2014-07-13 add for leaveway mode begain*/	
    ,mAudioALSAStreamManager(AudioALSAStreamManager::getInstance())
/*lenovo-sw fuxm1 2014-07-13 add for leaveway mode begain*/	
{
    ALOGD("%s()", __FUNCTION__);
    mPlaybackHandlerType = PLAYBACK_HANDLER_VOICE;
}


AudioALSAPlaybackHandlerVoice::~AudioALSAPlaybackHandlerVoice()
{
    ALOGD("%s()", __FUNCTION__);
}


status_t AudioALSAPlaybackHandlerVoice::open()
{
    ALOGD("+%s(), audio_mode = %d, u8BGSUlGain = %d, u8BGSDlGain = %d", __FUNCTION__, mStreamAttributeSource->audio_mode, mStreamAttributeSource->u8BGSUlGain, mStreamAttributeSource->u8BGSDlGain);
/*lenovo-sw fuxm1 2014-07-13 add for leaveway mode begain*/	
    bool mLeaveWayOn = false;
/*lenovo-sw fuxm1 2014-07-13 add for leaveway mode end*/
    // debug pcm dump
    OpenPCMDump(LOG_TAG);


    // HW attribute config // TODO(Harvey): query this
    mStreamAttributeTarget.audio_format = AUDIO_FORMAT_PCM_16_BIT;
    mStreamAttributeTarget.audio_channel_mask = mStreamAttributeSource->audio_channel_mask; // same as source stream
    mStreamAttributeTarget.num_channels = android_audio_legacy::AudioSystem::popCount(mStreamAttributeTarget.audio_channel_mask);
    mStreamAttributeTarget.sample_rate = mStreamAttributeSource->sample_rate; // same as source stream
    mStreamAttributeTarget.u8BGSDlGain = mStreamAttributeSource->u8BGSDlGain;
    mStreamAttributeTarget.u8BGSUlGain = mStreamAttributeSource->u8BGSUlGain;

    // bit conversion
    initBitConverter();


    // open background sound
    mBGSPlayer->mBGSMutex.lock();

    mBGSPlayer->CreateBGSPlayBuffer(
        mStreamAttributeTarget.sample_rate,
        mStreamAttributeTarget.num_channels,
        mStreamAttributeTarget.audio_format);

//chris add for debug
   ALOGD("we already get here!!!!!");
	
/*lenovo-sw fuxm1 2014-07-13 add for leaveway mode begain*/
/* lenovo-sw zhangrc2 add 2s delay to ensure getmode is ok  begin 2014-09-17 */
//   usleep(2000*1000);
/* lenovo-sw zhangrc2 add 2s delay to ensure getmode is ok  end 2014-09-17 */
   mLeaveWayOn = mAudioALSAStreamManager->getLeaveWayMode();
   ALOGD("%s(),isLeaveWayOn = %d", __FUNCTION__,mLeaveWayOn);
   
   if (mLeaveWayOn)
        mBGSPlayer->Open(SpeechDriverFactory::GetInstance()->GetSpeechDriver(), 0xFF, 0xFF); // TODO(Harvey): BGS gain setting?
   else
        mBGSPlayer->Open(SpeechDriverFactory::GetInstance()->GetSpeechDriver(), 0x0, 0xFF); // TODO(Harvey): BGS gain setting?
   //mBGSPlayer->Open(SpeechDriverFactory::GetInstance()->GetSpeechDriver(), mStreamAttributeTarget.u8BGSUlGain, mStreamAttributeTarget.u8BGSDlGain);
 /*lenovo-sw fuxm1 2014-07-13 add for leaveway mode end*/
    

    mBGSPlayer->mBGSMutex.unlock();

    ALOGD("-%s()", __FUNCTION__);
    return NO_ERROR;
}


status_t AudioALSAPlaybackHandlerVoice::close()
{
    ALOGD("+%s()", __FUNCTION__);

    // close background sound
    mBGSPlayer->mBGSMutex.lock();

    mBGSPlayer->Close();

    mBGSPlayer->DestroyBGSPlayBuffer();

    mBGSPlayer->mBGSMutex.unlock();


    // bit conversion
    deinitBitConverter();


    // debug pcm dump
    ClosePCMDump();


    ALOGD("-%s()", __FUNCTION__);
    return NO_ERROR;
}


status_t AudioALSAPlaybackHandlerVoice::routing(const audio_devices_t output_devices)
{
    return INVALID_OPERATION;
}


ssize_t AudioALSAPlaybackHandlerVoice::write(const void *buffer, size_t bytes)
{
    ALOGV("%s()", __FUNCTION__);

    // const -> to non const
    void *pBuffer = const_cast<void *>(buffer);
    ASSERT(pBuffer != NULL);

    // bit conversion
    void *pBufferAfterBitConvertion = NULL;
    uint32_t bytesAfterBitConvertion = 0;
    doBitConversion(pBuffer, bytes, &pBufferAfterBitConvertion, &bytesAfterBitConvertion);


    // write data to background sound
    WritePcmDumpData(pBufferAfterBitConvertion, bytesAfterBitConvertion);
    uint32_t u4WrittenBytes = BGSPlayer::GetInstance()->Write(pBufferAfterBitConvertion, bytesAfterBitConvertion);
    if (u4WrittenBytes != bytesAfterBitConvertion) // TODO: 16/32
    {
        ALOGE("%s(), BGSPlayer::GetInstance()->Write() error, u4WrittenBytes(%u) != bytesAfterBitConvertion(%u)", __FUNCTION__, u4WrittenBytes, bytesAfterBitConvertion);
    }

    return bytes;
}


} // end of namespace android

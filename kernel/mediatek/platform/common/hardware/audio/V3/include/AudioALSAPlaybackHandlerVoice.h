#ifndef ANDROID_AUDIO_ALSA_PLAYBACK_HANDLER_VOICE_H
#define ANDROID_AUDIO_ALSA_PLAYBACK_HANDLER_VOICE_H

#include "AudioALSAPlaybackHandlerBase.h"
/*lenovo-sw fuxm1 2014-07-13 add for leaveway mode begain*/
#include  "AudioALSAStreamManager.h"
/*lenovo-sw fuxm1 2014-07-13 add for leaveway mode end*/

namespace android
{

class BGSPlayer;
/*lenovo-sw fuxm1 2014-07-13 add for leaveway mode begain*/
class AudioALSAStreamManager;
/*lenovo-sw fuxm1 2014-07-13 add for leaveway mode end*/

class AudioALSAPlaybackHandlerVoice : public AudioALSAPlaybackHandlerBase
{
    public:
        AudioALSAPlaybackHandlerVoice(const stream_attribute_t *stream_attribute_source);
        virtual ~AudioALSAPlaybackHandlerVoice();


        /**
         * open/close audio hardware
         */
        virtual status_t open();
        virtual status_t close();
        virtual status_t routing(const audio_devices_t output_devices);


        /**
         * write data to audio hardware
         */
        virtual ssize_t  write(const void *buffer, size_t bytes);



    private:
        BGSPlayer *mBGSPlayer;
/*lenovo-sw fuxm1 2014-07-13 add for leaveway mode begain*/
   //chirs add:
    /*in this way we can add other class functions resource to local class, and cross process
      to access. this is useful for parametes set/get*/
	  AudioALSAStreamManager *mAudioALSAStreamManager;
        //bool isLeaveWayOn;
/*lenovo-sw fuxm1 2014-07-13 add for leaveway mode end*/
};

} // end namespace android

#endif // end of ANDROID_AUDIO_ALSA_PLAYBACK_HANDLER_VOICE_H

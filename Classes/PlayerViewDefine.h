
#ifndef PLAYER_VIEW_DEFINE
#define PLAYER_VIEW_DEFINE

#define RETURN_STATUS_IF_ERROR(status) {EnumPlayerStatus x = status; if (x != ePlayerStatusOk) {return x;}}
#define RETURN_IF_ERROR(status) {EnumPlayerStatus x = status; if (x != ePlayerStatusOk) {return;}}

typedef enum EnumPlayerStatus
{
    ePlayerStatusOk = 0,
    ePlayerStatusError,
    ePlayerStatusNotImp,
    ePlayerStatusNotEnoughMemory,
    ePlayerStatusCanNotOpenFile,
    ePlayerStatusNoMediaStream,
    ePlayerStatusCanNotInitialAudioDevice,
    ePlayerStatusCanNotInitialVideoDevice,
}EnumPlayerStatus;

typedef enum EnumAspectRatio
{
    eAspectRadioOriginal = 0, // 1:1
    eAspectRadio4_3,
    eAspectRadio16_9,
    eAspectRadioFullScreen,
}EnumAspectRatio;

typedef enum EnumPlaySpeed
{
    ePlaySpeedHalf = -1,
    ePlaySpeedNormal,
    ePlaySpeedDouble,
    ePlaySpeedForth,
}EnumPlaySpeed;


#endif
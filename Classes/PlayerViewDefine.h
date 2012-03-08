
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
    ePlayerStatusBadSubTitleFileName,
    ePlayerStatusBadSubTitleFile,
}EnumPlayerStatus;

typedef enum EnumAspectRatio
{
    eAspectRatioOriginal = 0, // 1:1
    eAspectRatio4_3,
    eAspectRatio16_9,

    eAspectRatioEnd,
    eAspectRatioFullScreen = eAspectRatioEnd,
}EnumAspectRatio;

typedef enum EnumPlaySpeed
{
    ePlaySpeedHalf = -1,
    ePlaySpeedNormal,
    ePlaySpeedDouble,
    ePlaySpeedForth,
}EnumPlaySpeed;


#endif
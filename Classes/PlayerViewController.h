//
//  ViewPlayerController.h
//  Player
//
//  Create by xiaoyi liao  on 11-2-26.
//  Copyright 2011 . All rights reserved.
//

#import <UIKit/UIKit.h>

typedef enum EnumPlayerStatus
{
    ePlayerStatusOk = 0,
    ePlayerStatusError,
    ePlayerStatusNotImp,
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


@protocol PlayerViewControllerDelegate

@end

@interface PlayerViewController : UIViewController 

+(PlayerViewController*) retainPlayerViewController;
+(void) releasePlayerViewController;

- (EnumPlayerStatus) open:(NSString*)strFileName; 
- (EnumPlayerStatus) play;
- (EnumPlayerStatus) pause;
- (EnumPlayerStatus) close;
- (EnumPlayerStatus) setAspectRatio:(EnumAspectRatio)eAspectRatio;
- (EnumPlayerStatus) seek:(unsigned long)iSeekPos;
- (EnumPlayerStatus) setPlaySpeed:(EnumPlaySpeed)ePlaySpeed;
- (unsigned long) getDuration;
- (unsigned long) getPlayingTime;


@end

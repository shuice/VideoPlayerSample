#import "PlayerViewController.h"
#import "PlayerViewControllerImp.h"
#import "UserDefaultHelper.h"
@implementation PlayerViewController

+(PlayerViewController*) sharedPlayer
{
    static PlayerViewControllerImp* playerViewControllerImp = nil;
    if (playerViewControllerImp == nil)
    {
        [UserDefaultHelper init];
        playerViewControllerImp = [[PlayerViewControllerImp alloc] init];
        [[UIApplication sharedApplication] setStatusBarHidden:YES];
        [playerViewControllerImp view];
    }
    return playerViewControllerImp;
}

- (EnumPlayerStatus) open:(NSString*)strFileName
              andDelegate:(id<PlayerViewControllerDelegate>)delegate; 
{
    return ePlayerStatusNotImp;
}

- (EnumPlayerStatus) setSubTitle:(NSString*)strSubTitleName andCodePage:(int)iCodePage
{
    return ePlayerStatusNotImp;
}

- (EnumPlayerStatus) play
{
    return ePlayerStatusNotImp;
}
- (EnumPlayerStatus) pause;
{
    return ePlayerStatusNotImp;
}
- (EnumPlayerStatus) close
{
    return ePlayerStatusNotImp;
}

- (unsigned long) getDuration
{
    return ePlayerStatusNotImp;
}
- (unsigned long) getPlayingTime
{
    return ePlayerStatusNotImp;
}

- (EnumPlayerStatus) seek:(unsigned long)iSeekPos
{
    return ePlayerStatusNotImp;
}

- (EnumPlayerStatus) setPlaySpeed:(EnumPlaySpeed)ePlaySpeed
{
    return ePlayerStatusNotImp;
}

- (EnumPlaySpeed) getPlaySpeed
{
    return ePlaySpeedNormal;
}

- (EnumPlayerStatus) setAspectRatio:(EnumAspectRatio)eAspectRatio
{
    return ePlayerStatusNotImp;
}
@end
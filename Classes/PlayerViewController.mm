#import "PlayerViewController.h"
#import "PlayerViewControllerImp.h"

static PlayerViewControllerImp* g_playerViewControllerImp = nil;
static int g_iRefrenceCount = 0;
@implementation PlayerViewController

+(PlayerViewController*) initPlayer
{
    if (g_iRefrenceCount < 0)
    {
        g_iRefrenceCount = 0;
    }
    
    if (g_playerViewControllerImp == nil)
    {
        g_playerViewControllerImp = [[PlayerViewControllerImp alloc] init];
        [g_playerViewControllerImp view];
    }
    g_iRefrenceCount ++;
    return g_playerViewControllerImp;
}
+(void) deallocPlayer
{
    g_iRefrenceCount --;
    if (g_iRefrenceCount == 0)
    {
        [g_playerViewControllerImp release];
        g_playerViewControllerImp = nil;
    }
    
    if (g_iRefrenceCount < 0)
    {
        g_iRefrenceCount = 0;
    }
}

- (EnumPlayerStatus) open:(NSString*)strFileName
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
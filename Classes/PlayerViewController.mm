#import "PlayerViewController.h"
#import "PlayerViewControllerImp.h"

static PlayerViewControllerImp* g_playerViewControllerImp = nil;
static int iRefrenceCount = 0;
@implementation PlayerViewController

+(PlayerViewController*) retainPlayerViewController
{
    if (g_playerViewControllerImp == nil)
    {
        g_playerViewControllerImp = [[PlayerViewControllerImp alloc] init];
    }
    iRefrenceCount ++;
    return g_playerViewControllerImp;
}
+(void) releasePlayerViewController
{
    iRefrenceCount --;
    if (iRefrenceCount == 0)
    {
        [g_playerViewControllerImp release];
        g_playerViewControllerImp = nil;
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
@end
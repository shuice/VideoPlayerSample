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



@protocol PlayerViewControllerDelegate

@end

@interface PlayerViewController : UIViewController 

+(PlayerViewController*) retainPlayerViewController;
+(void) releasePlayerViewController;

- (EnumPlayerStatus) open:(NSString*)strFileName; 
- (EnumPlayerStatus) play;
- (EnumPlayerStatus) pause;
- (EnumPlayerStatus) close;

- (unsigned long) getDuration;
- (unsigned long) getPlayingTime;


@end

//
//  ViewPlayerController.h
//  Player
//
//  Create by xiaoyi liao  on 11-2-26.
//  Copyright 2011 . All rights reserved.
//

#import <UIKit/UIKit.h>
#import "PlayerViewDefine.h"

@protocol PlayerViewControllerDelegate
@required
- (void) playFinish; // restore ui after pla finish
@end

@interface PlayerViewController : UIViewController 

+(PlayerViewController*) sharedPlayer;

- (EnumPlayerStatus) open:(NSString*)strFileName
              andDelegate:(id<PlayerViewControllerDelegate>)delegate; 
- (EnumPlayerStatus) setSubTitle:(NSString*)strSubTitleName andCodePage:(int)iCodePage;
- (EnumPlayerStatus) play;
- (EnumPlayerStatus) pause;
- (EnumPlayerStatus) close;
- (EnumPlayerStatus) setAspectRatio:(EnumAspectRatio)eAspectRatio;
- (EnumPlayerStatus) seek:(unsigned long)iSeekPos;
- (EnumPlayerStatus) setPlaySpeed:(EnumPlaySpeed)ePlaySpeed;
- (EnumPlaySpeed) getPlaySpeed;
- (unsigned long) getDuration;
- (unsigned long) getPlayingTime;


@end

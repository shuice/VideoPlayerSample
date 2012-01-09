//
//  PlayerAppDelegate.h
//  Player
//
//  Create by xiaoyi liao  on 11-2-17.
//  Copyright 2011 . All rights reserved.
//

#import <UIKit/UIKit.h>
#import "common1.h"
#import "ViewPlayerController.h"
@class MediaType;
@class ViewPlayerController;
@interface PlayerAppDelegate : NSObject <UIApplicationDelegate> {
    UIWindow *window;
	ViewPlayerController* viewPlayerController;
	IBOutlet UINavigationController* uiNaviController;
}

@property (nonatomic, retain) IBOutlet UIWindow *window;
@property (nonatomic, retain) ViewPlayerController* viewPlayerController;
@property (nonatomic, retain) UINavigationController* uiNaviController;

//-(void) PlayLocal:(const char*)pszFileName;
-(void) Stop;

-(void) PlayLocal:(string)strFileName
	  ePlayerType:(EnumPlayerType)ePlayerType
	   strSrtPath:(string)strSrtPath
		iCodePage:(int)iCodePage;


@end


//
//  PlayerAppDelegate.m
//  Player
//
//  Created by  on 11-2-17.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import "PlayerAppDelegate.h"
#include <sys/stat.h>
#import "Common1.h"
#include "Common.h"
#import "PlayerViewControllerImp.h"


@implementation UINavigationBar (CustomImage)  
- (void)drawRect:(CGRect)rect {   
	UIImage *imageMainNavBg = [UIImage imageNamed: @"Main_Nav_bg.png"];
    [imageMainNavBg drawInRect:CGRectMake(0, 0, self.frame.size.width, self.frame.size.height)];  
}  
@end  

@implementation UITableView (CustomImage)  
- (void)drawRect:(CGRect)rect {  
	UIImage *imageMainBg = [UIImage imageNamed: @"Main_bg.png"];
    [imageMainBg drawInRect:CGRectMake(0, 0, self.frame.size.width, self.frame.size.height)]; 
}  
@end

@implementation PlayerAppDelegate

@synthesize window;
@synthesize uiNaviController;
@synthesize viewPlayerController;

#pragma mark -
#pragma mark Application lifecycle






//NSLog([[UIDevice currentDevice] name]); // Name of the phone as named by user 
//NSLog([[UIDevice currentDevice] uniqueIdentifier]); // A GUID like string 
//NSLog([[UIDevice currentDevice] systemName]); // "iPhone OS" 
//NSLog([[UIDevice currentDevice] systemVersion]); // "2.2.1" 
//NSLog([[UIDevice currentDevice] model]); // "iPhone" on both devices 
//NSLog([[UIDevice currentDevice] localizedModel]); // "iPhone" on both devices 
float version = [[[UIDevice currentDevice] systemVersion] floatValue]; 
- (BOOL)application:(UIApplication *)application didFinishLaunchingWithOptions:(NSDictionary *)launchOptions {    
    // Override point for customization after application launch.
	// create Media folder
	{
		NSArray *paths = NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDomainMask, YES);
		NSString *documentDirectory = [paths objectAtIndex:0];
		
		NSString *writeableDBPath = [documentDirectory stringByAppendingPathComponent:@"Media"];
		mkdir([writeableDBPath UTF8String], 0777);
		
		NSString *tmpPath = [documentDirectory stringByAppendingPathComponent:@"Tmp"];
		mkdir([tmpPath UTF8String], 0777);
		
		vector<string> vectFiles;
		string strTmpPath = [tmpPath UTF8String];
		GetFilesInFolder(strTmpPath, vectFiles);
		int iFilesCount = vectFiles.size();
		if (iFilesCount > 200) // too much files
		{
			for (int iFileIndex = 0; iFileIndex < 100; iFileIndex ++)
			{
				remove((strTmpPath + "/" + vectFiles[iFileIndex]).c_str());
			}
		}
	}
	
	viewPlayerController = [[PlayerViewControllerImp alloc] 
							initWithNibName:@"ViewPlayerController" bundle:nil];
	if (viewPlayerController == nil)
	{
		return NO;
	}
	
	[window addSubview:uiNaviController.view];
	[window bringSubviewToFront:uiNaviController.view];
	[window makeKeyAndVisible];
    return YES;
}


- (void)applicationWillResignActive:(UIApplication *)application {
    /*
     Sent when the application is about to move from active to inactive state. This can occur for certain types of temporary interruptions (such as an incoming phone call or SMS message) or when the user quits the application and it begins the transition to the background state.
     Use this method to pause ongoing tasks, disable timers, and throttle down OpenGL ES frame rates. Games should use this method to pause the game.
     */
	[viewPlayerController Pause];
}


- (void)applicationDidEnterBackground:(UIApplication *)application {
    /*
     Use this method to release shared resources, save user data, invalidate timers, and store enough application state information to restore your application to its current state in case it is terminated later. 
     If your application supports background execution, called instead of applicationWillTerminate: when the user quits.
     */

}


- (void)applicationWillEnterForeground:(UIApplication *)application {
    /*
     Called as part of  transition from the background to the inactive state: here you can undo many of the changes made on entering the background.
     */
}


- (void)applicationDidBecomeActive:(UIApplication *)application {
    /*
     Restart any tasks that were paused (or not yet started) while the application was inactive. If the application was previously in the background, optionally refresh the user interface.
     */
}


- (void)applicationWillTerminate:(UIApplication *)application {
    /*
     Called when the application is about to terminate.
     See also applicationDidEnterBackground:.
     */
	}


#pragma mark -
#pragma mark Memory management

- (void)applicationDidReceiveMemoryWarning:(UIApplication *)application {
    /*
     Free up as much memory as possible by purging cached data objects that can be recreated (or reloaded from disk) later.
     */
}


- (void)dealloc {
	[uiNaviController release];
	[viewPlayerController release];
    [window release];
    [super dealloc];
}

-(void) PlayLocal:(string)pszFileName
   ePlayerType:(EnumPlayerType)ePlayerType
	   strSrtPath:(string)strSrtPath
		iCodePage:(int)iCodePage;
{	
	[[UIApplication sharedApplication] setIdleTimerDisabled:YES];
	[[UIApplication sharedApplication] setStatusBarHidden:YES];
	[uiNaviController.view removeFromSuperview];
	[window addSubview:viewPlayerController.view];
	[window bringSubviewToFront:viewPlayerController.view];
	[window makeKeyAndVisible];
	
	viewPlayerController.m_ePlayerType = ePlayerType;
	viewPlayerController.m_strSrtPath = strSrtPath;
	viewPlayerController.m_iCodePage = iCodePage;
	[viewPlayerController StartPlay:pszFileName];
	
	
}

-(void) Stop
{
	[[UIApplication sharedApplication] setIdleTimerDisabled:NO];	
	[[UIApplication sharedApplication] setStatusBarHidden:NO];
	[viewPlayerController.view removeFromSuperview];
	[window addSubview:uiNaviController.view];
	[window bringSubviewToFront:uiNaviController.view];
	[window makeKeyAndVisible];
}

@end


//
//  ViewPlayerController.mm
//  Player
//
//  Create by xiaoyi liao  on 11-2-26.
//  Copyright 2011 . All rights reserved.
//

#import "ViewPlayerController.h"
#import "PlayerAppDelegate.h"
#include "LocalPlayer.h"
#import "Common.h"
#import <sys/time.h>


@implementation ViewPlayerController

@synthesize viewControlProgress;
@synthesize viewControlSound;
@synthesize playerView;
@synthesize m_strFileName;
@synthesize buttonPlay;
@synthesize buttonPause;
@synthesize buttonBackward;
@synthesize buttonForward;
@synthesize buttonPlaySmall;
@synthesize buttonPauseSmall;
@synthesize buttonBackwardSmall;
@synthesize buttonForwardSmall;
@synthesize labelPlayed;
@synthesize labelLeft;
@synthesize nsTimer;
@synthesize uiSliderProgress;
@synthesize uiSliderSound;
@synthesize uiImageViewAudio;
@synthesize uiImageViewLightBackward;
@synthesize uiImageViewLightPlay;
@synthesize uiImageViewLightForward;
@synthesize m_pLocalPlayer;
@synthesize m_iControlLife;
@synthesize m_iDuration;
@synthesize m_iCurrentTime;
@synthesize m_bIsReadEndOfFile;
@synthesize socketCommand;
@synthesize socketCache;
@synthesize m_iInNetSeeking;
@synthesize m_ePlayerType;
@synthesize m_strSrtPath;
@synthesize m_iCodePage;
// 5 seconds
#define AUTO_CONTROL_HIDDEN_TIME 150
#define ALWAYS_SHOW_TIME 0x7FFFFFFF


/*
 // The designated initializer.  Override if you create the controller programmatically and want to perform customization that is not appropriate for viewDidLoad.
- (id)initWithNibName:(NSString *)nibNameOrNil bundle:(NSBundle *)nibBundleOrNil {
    if ((self = [super initWithNibName:nibNameOrNil bundle:nibBundleOrNil])) {
        // Custom initialization
    }
    return self;
}
*/


// Implement viewDidLoad to do additional setup after loading the view, typically from a nib.
- (void)viewDidLoad {
	float fVersion = atof([[[UIDevice currentDevice] systemVersion] UTF8String]);
	if (fVersion >= 4.0)
	{
		buttonPlaySmall.alpha = 0;
		buttonPauseSmall.alpha = 0;
		buttonBackwardSmall.alpha = 0;
		buttonForwardSmall.alpha = 0;
	}
	else
	{
		buttonPlay.alpha = 0;
		buttonPause.alpha = 0;
		buttonBackward.alpha = 0;
		buttonForward.alpha = 0;
	}
	[self SetPauseVisiable:NO];
    [super viewDidLoad];
}



// Override to allow orientations other than the default portrait orientation.
-(BOOL)shouldAutorotateToInterfaceOrientation:(UIInterfaceOrientation)toInterfaceOrientation
{
	if ((toInterfaceOrientation == UIInterfaceOrientationLandscapeRight) 
		|| (toInterfaceOrientation == UIInterfaceOrientationLandscapeLeft))
	{
		return YES;
	}
	return NO;	
}

- (void)didReceiveMemoryWarning {
    // Releases the view if it doesn't have a superview.
    [super didReceiveMemoryWarning];
    
    // Release any cached data, images, etc that aren't in use.
}

- (void)viewDidUnload {
    [super viewDidUnload];
    // Release any retained subviews of the main view.
    // e.g. self.myOutlet = nil;
}


- (void)dealloc {
	[playerView release];
	[viewControlProgress release];
	[viewControlSound release];
	[buttonPlay release];
	[buttonPause release];
	[buttonBackward release];
	[buttonForward release];
	[buttonPlaySmall release];
	[buttonPauseSmall release];
	[buttonBackwardSmall release];
	[buttonForwardSmall release];
	[labelPlayed release];
	[labelLeft release];
	[uiSliderProgress release];
	[uiSliderSound release];
	
	[uiImageViewLightBackward release];
	[uiImageViewLightPlay release];
	[uiImageViewLightForward release];
	[uiImageViewAudio release];
	[nsTimer release];
    [super dealloc];
}

-(string) GetSrtFileName
{
	if (m_ePlayerType == ePlayerTypeLocal)
	{
		return m_strSrtPath;
	}
    return "";
}


-(BOOL) Open
{
	// init all controls
	labelPlayed.text = @"";
	labelLeft.text = @"";
	playerView.uiLabel.text = @"";
	uiSliderProgress.value = 0.0;
	m_bInSeek = NO;
	try 
	{
		m_pLocalPlayer = new CLocalPlayer;
		if (m_pLocalPlayer == NULL)
		{
			return NO;
		}
		{
			
			m_pLocalPlayer->m_strSubTitleLocal = [self GetSrtFileName];
			m_pLocalPlayer->m_iCodePage = m_iCodePage;
		}
		m_pLocalPlayer->m_pCurrentTime = &m_iCurrentTime;
		m_pLocalPlayer->m_pDuration = &m_iDuration;
		m_pLocalPlayer->m_pIsReadEndOfFile= &m_bIsReadEndOfFile;
		m_pLocalPlayer->m_pInNetSeeking = &m_iInNetSeeking;
		m_pLocalPlayer->socketCache = self.socketCache;
		m_pLocalPlayer->socketCommand = self.socketCommand;
		m_pLocalPlayer->m_ePlayerType = m_ePlayerType;
		m_bIsReadEndOfFile = false;	
		
		m_pLocalPlayer->Open(m_strFileName, (int)playerView);
	}
	catch (CPlayerException* pPlayerException)
	{
		ShowAlartMessage(pPlayerException->m_strDescription);
		delete pPlayerException;
		
		try 
		{
			m_pLocalPlayer->Close();
		}
		catch (CPlayerException* pPlayerException)
		{
			delete pPlayerException;
		}
		return NO;
	}
	m_pLocalPlayer->SetVolume(uiSliderSound.value * 100); 
	
	[self SetPauseVisiable:NO];
	[self SetPlayVisiable:YES];
	
	[self SetControlVisiable:YES];
	return YES;
}


-(void) Close
{
	if (YES == [nsTimer isValid])
	{
		[nsTimer invalidate];
	}
	if (m_pLocalPlayer != NULL)
	{
		try 
		{
			m_pLocalPlayer->Close();
		}
		catch (CPlayerException* e)
		{
			//ShowAlartMessage(e->m_strDescription);
			delete e;
		}
		
		delete m_pLocalPlayer;
		m_pLocalPlayer = NULL;
	}
	[self SetPlayVisiable:YES];
	[self SetPauseVisiable:NO];
	labelPlayed.text = @"";
	labelLeft.text = @"";
	uiSliderProgress.value = 0.0;
	[playerView eraseBackground];
	[playerView destroyTexturebuffer];
	
	// FIXME close timer
}

-(void) Exit
{
	[self Close];
	PlayerAppDelegate* pPlayerAppdelegate = [[UIApplication sharedApplication] delegate];
	[pPlayerAppdelegate Stop];	
}

-(void) buttonExit:(id)sender
{
	[self Exit];
}

-(void) buttonPlay:(id)sender
{
	if (m_pLocalPlayer == NULL)
	{
		NSLog(@"buttonPre but m_pLocalPlayer == NULL");
		return;
	}
	m_iControlLife = m_pLocalPlayer->HasVideo() ? AUTO_CONTROL_HIDDEN_TIME : ALWAYS_SHOW_TIME;
	if (m_pLocalPlayer->IsPaused())
	{
		m_pLocalPlayer->Play();
		[self SetPauseVisiable:YES];
		[self SetPlayVisiable:NO];
	}
	

}
-(void) buttonPause:(id)sender
{
	[self Pause];

}

-(void) buttonPre:(id)sender
{
	m_iControlLife = AUTO_CONTROL_HIDDEN_TIME;
	if (m_pLocalPlayer == NULL)
	{
		NSLog(@"buttonPre but m_pLocalPlayer == NULL");
		return;
	}
	m_pLocalPlayer->SeekBySecond(m_iCurrentTime - 30);
}

-(void) buttonNext:(id)sender
{
	if (m_pLocalPlayer == NULL)
	{
		NSLog(@"buttonPre but m_pLocalPlayer == NULL");
		return;
	}
	m_iControlLife = m_pLocalPlayer->HasVideo() ? AUTO_CONTROL_HIDDEN_TIME : ALWAYS_SHOW_TIME;
	m_pLocalPlayer->SeekBySecond(m_iCurrentTime + 30);
}

-(void) buttonPlaySmall:(id)sender
{
	[self buttonPlay:nil];
}
-(void) buttonPauseSmall:(id)sender
{
	[self buttonPause:nil];
}
-(void) buttonPreSmall:(id)sender
{
	[self buttonPre:nil];
}
-(void) buttonNextSmall:(id)sender
{
	[self buttonNext:nil];
}

-(void) OnTouchInPos:(id)sender
{
	m_bInSeek = YES;
}

-(void) OnTouchOutPos:(id)sender
{
	if (m_bInSeek == YES)
	{
		if (m_pLocalPlayer != NULL)
		{
			m_pLocalPlayer->SeekBySecond(m_iSeekPos);
		}
		
	}
	m_bInSeek = NO;
}



-(void) sliderPosChangeProgress:(id)sender
{
	if (m_pLocalPlayer == NULL)
	{
		NSLog(@"buttonPre but m_pLocalPlayer == NULL");
		return;
	}
	m_iControlLife = m_pLocalPlayer->HasVideo() ? AUTO_CONTROL_HIDDEN_TIME : ALWAYS_SHOW_TIME;
	m_iSeekPos = m_iDuration * 1.0 * uiSliderProgress.value;
	m_bInSeek = YES;
	
}

-(void) sliderPosChangeSound:(id)sender
{
	if (m_pLocalPlayer == NULL)
	{
		NSLog(@"buttonPre but m_pLocalPlayer == NULL");
		return;
	}
	m_iControlLife = m_pLocalPlayer->HasVideo() ? AUTO_CONTROL_HIDDEN_TIME : ALWAYS_SHOW_TIME;
	UISlider* uiSlider = (UISlider*)sender;
	m_pLocalPlayer->SetVolume(uiSlider.value * 100);
}

-(void)autoHiddenControl:(NSTimer*)timer
{
	[self SetControlVisiable:NO];
}

-(BOOL) isControlVisiable
{
	int iAlpha = viewControlSound.alpha;
	if (iAlpha == 0)
	{
		return NO;
	}
	return YES;
}

-(void)handleTimer:(NSTimer*)timer
{
	if (m_bIsReadEndOfFile)
	{
		[self Close];
		PlayerAppDelegate* pPlayerAppdelegate = [[UIApplication sharedApplication] delegate];
		[pPlayerAppdelegate Stop];	
	}
	// check control visiable
	if ([self isControlVisiable] == YES)
	{
		m_iControlLife --;
		if (m_iControlLife <= 0)
		{
			m_iControlLife = 0;
			[self SetControlVisiable:NO];
		}
	}
	// refresh media
	if (m_pLocalPlayer != NULL && m_pLocalPlayer->HasVideo()) 
	{
		if (!m_pLocalPlayer->IsPaused())
		{
			[playerView handleTimer];
		}
	}
	
	//return;
	if ((m_bInSeek == YES) || (m_iInNetSeeking == 1))
	{
		m_iCurrentTime = uiSliderProgress.value * m_iDuration;
	}
	else
	{
		float fValue = m_iCurrentTime * 1.0 / m_iDuration;
		uiSliderProgress.value = fValue;
	}


	char szTime[20] = {0};
	// update progress
	{
		NSString* nsString = [[NSString alloc] initWithUTF8String:
							 FormatTime(m_iCurrentTime, true, szTime, sizeof(szTime))];
		labelPlayed.text = nsString;
		[nsString release];
	}
	{
		NSString* nsString = [[NSString alloc] initWithUTF8String:
							  FormatTime(m_iDuration - m_iCurrentTime, false, szTime, sizeof(szTime))];
		labelLeft.text = nsString;
		[nsString release];
	}	
	
}

void ShowAlartMessage(string strMessage)
{
	NSString* nsString = [[NSString alloc] initWithUTF8String:strMessage.c_str()];
	UIAlertView* alert = [[UIAlertView alloc] initWithTitle:@"信息" 
													message:nsString
												   delegate:nil
										  cancelButtonTitle:@"ok"
										  otherButtonTitles:nil];
	[alert show];
	[alert release];
}




-(void) SetControlVisiable:(BOOL)bVisiable
{
	viewControlSound.alpha = (bVisiable == YES) ? 100 : 0;
	viewControlProgress.alpha = (bVisiable == YES) ? 100 : 0;
	[[UIApplication sharedApplication] setStatusBarHidden:!bVisiable];
}

-(void) onTouchPlayerView:(id)sender
{
	if (([self isControlVisiable] == YES) && m_pLocalPlayer->HasVideo())
	{
		[self SetControlVisiable:NO];
	}
	else 
	{
		[self SetControlVisiable:YES];
		m_iControlLife = m_pLocalPlayer->HasVideo() ? AUTO_CONTROL_HIDDEN_TIME : ALWAYS_SHOW_TIME;
	}

}

-(void) StartPlay:(string)strFileName
{
	[playerView eraseBackground];
	m_strFileName = strFileName;
	if (YES != [self Open])
	{
		ShowAlartMessage("打开文件失败");
		[self Exit];
		return;
	}
	
	bool bHasVideo = m_pLocalPlayer->HasVideo();
	uiImageViewAudio.alpha = bHasVideo ? 0 : 100;
	m_iControlLife = bHasVideo ? AUTO_CONTROL_HIDDEN_TIME : ALWAYS_SHOW_TIME;
	m_pLocalPlayer->Start();
	[self SetPauseVisiable:YES];
	[self SetPlayVisiable:NO];

	[self SetControlVisiable:YES];
	
	nsTimer = [NSTimer scheduledTimerWithTimeInterval:1.0/25.0 
												   target:self
												 selector:@selector(handleTimer:)
												 userInfo:nil
												  repeats:YES];
	
	//timer
}
-(void) Pause
{
	m_iControlLife = ALWAYS_SHOW_TIME;
	[self SetControlVisiable:YES];
	if (m_pLocalPlayer == NULL)
	{
		NSLog(@"buttonPre but m_pLocalPlayer == NULL");
		return;
	}
	if (!m_pLocalPlayer->IsPaused())
	{
		m_pLocalPlayer->Pause();
		[self SetPauseVisiable:NO];
		[self SetPlayVisiable:YES];
	}
}

- (void) SetPauseVisiable:(BOOL) bVisiable
{
	float fVersion = atof([[[UIDevice currentDevice] systemVersion] UTF8String]);
	if (fVersion >= 4.0)
	{
		buttonPause.alpha = (bVisiable == YES) ? 100 : NO;
	}
	else
	{
		buttonPauseSmall.alpha = (bVisiable == YES) ? 100 : NO;
	}
}

- (void) SetPlayVisiable:(BOOL) bVisiable
{
	float fVersion = atof([[[UIDevice currentDevice] systemVersion] UTF8String]);
	if (fVersion >= 4.0)
	{
		buttonPlay.alpha = (bVisiable == YES) ? 100 : NO;
	}
	else
	{
		buttonPlaySmall.alpha = (bVisiable == YES) ? 100 : NO;
	}
}
@end


//
//  ViewPlayerController.mm
//  Player
//
//  Create by xiaoyi liao  on 11-2-26.
//  Copyright 2011 . All rights reserved.
//

#import "PlayerViewControllerImp.h"
#include "LocalPlayer.h"
#import "Common.h"
#import <sys/time.h>
#import <QuartzCore/CALayer.h>

@interface PlayerViewControllerImp(method)
    - (void) refreshChangeAspectButton;
    -(EnumPlayerStatus) Open;
    -(BOOL) isControlVisiable;
- (void) SetPlayVisiable:(BOOL) bVisiable;
- (void) SetPauseVisiable:(BOOL) bVisiable;
-(void) SetControlVisiable:(BOOL)bVisiable;
- (void)setSubTitlePos;
@end


@implementation PlayerViewControllerImp

@synthesize viewControlProgress;
@synthesize viewControlSound;
@synthesize playerView;
@synthesize controlControls;
@synthesize m_strFileName;
@synthesize buttonPlay;
@synthesize buttonPause;
@synthesize buttonBackward;
@synthesize buttonForward;
@synthesize labelPlayed;
@synthesize labelLeft;
@synthesize nsTimer;
@synthesize uiSliderProgress;
@synthesize uiSliderSound;
@synthesize m_pLocalPlayer;
@synthesize m_iControlLife;
@synthesize m_strSrtPath;
@synthesize m_iCodePage;
@synthesize buttonChangeAspect;
@synthesize imageViewControlProgress;
@synthesize imageViewControlSound;
@synthesize uiLabelSubTitle;
// 5 seconds
#define AUTO_CONTROL_HIDDEN_TIME 150
#define ALWAYS_SHOW_TIME 0x7FFFFFFF

- (void) insertSubViews
{
    self.playerView = [[[PlayerView alloc] initWithFrame:CGRectMake(0.0f, 0.0f, PLAYER_FRAME_WIDTH, PLAYER_FRAME_HEIGHT)] autorelease];
    self.controlControls = [[[UIControl alloc] initWithFrame:[playerView bounds]] autorelease];
    [controlControls addTarget:self action:@selector(onTouchUpInsideControlControls:) forControlEvents:UIControlEventTouchUpInside];
    
    playerView.playerViewControllerImp = self;
    self.viewControlProgress = [[[UIView alloc] initWithFrame:CGRectZero] autorelease];
    self.viewControlSound = [[[UIView alloc] initWithFrame:CGRectZero] autorelease];
    
    self.imageViewControlProgress = [[[UIImageView alloc] initWithImage:[[UIImage imageNamed:@"Control_Up_Background.png"] stretchableImageWithLeftCapWidth:10 topCapHeight:0]] autorelease];
    self.imageViewControlSound = [[[UIImageView alloc] initWithImage:[[UIImage imageNamed:@"Control_Down_Background.png"] stretchableImageWithLeftCapWidth:10 topCapHeight:0]] autorelease];
    
    self.buttonPlay = [UIButton buttonWithType:UIButtonTypeCustom];
    [buttonPlay addTarget:self action:@selector(onTouchUpInsidePlay:) forControlEvents:UIControlEventTouchUpInside];
    self.buttonPause = [UIButton buttonWithType:UIButtonTypeCustom];
    [buttonPause addTarget:self action:@selector(onTouchUpInsidePause:) forControlEvents:UIControlEventTouchUpInside];
    self.buttonBackward = [UIButton buttonWithType:UIButtonTypeCustom];
    [buttonBackward addTarget:self action:@selector(onTouchUpInsideBackword:) forControlEvents:UIControlEventTouchUpInside];
    self.buttonForward = [UIButton buttonWithType:UIButtonTypeCustom];
    [buttonForward addTarget:self action:@selector(onTouchUpInsideForward:) forControlEvents:UIControlEventTouchUpInside];
    self.labelPlayed = [[[UILabel alloc] initWithFrame:CGRectZero] autorelease];
    [labelPlayed setFont:[UIFont systemFontOfSize:12.0f]];
    [labelPlayed setTextColor:[UIColor whiteColor]];
    self.labelLeft = [[[UILabel alloc] initWithFrame:CGRectZero] autorelease];
    [labelLeft setFont:[UIFont systemFontOfSize:12.0f]];
    [labelLeft setTextColor:[UIColor whiteColor]];
    self.uiSliderProgress = [[[UISlider alloc] initWithFrame:CGRectZero] autorelease];
    [uiSliderProgress addTarget:self action:@selector(sliderPosChangeProgress:) forControlEvents:UIControlEventValueChanged];
    [uiSliderProgress addTarget:self action:@selector(OnTouchInPos:) forControlEvents:UIControlEventTouchDown];
    [uiSliderProgress addTarget:self action:@selector(OnTouchOutPos:) forControlEvents:UIControlEventTouchUpInside];
    [uiSliderProgress addTarget:self action:@selector(OnTouchOutPos:) forControlEvents:UIControlEventTouchUpOutside];
    self.uiSliderSound = [[[UISlider alloc] initWithFrame:CGRectZero] autorelease];
    [uiSliderSound addTarget:self action:@selector(sliderPosChangeSound:) forControlEvents:UIControlEventValueChanged];
    self.buttonChangeAspect = [UIButton buttonWithType:UIButtonTypeRoundedRect];
    [buttonChangeAspect addTarget:self action:@selector(onTouchUpInsideChangeAspect:) forControlEvents:UIControlEventTouchUpInside];
    
    self.uiLabelSubTitle = [[[UILabel alloc] initWithFrame:CGRectZero] autorelease];
    CALayer* layer = [uiLabelSubTitle layer];
    layer.shadowRadius = 5.0;
    uiLabelSubTitle.layer.shadowOpacity = 0.7;
    uiLabelSubTitle.layer.shadowColor = [[UIColor orangeColor] CGColor];
    uiLabelSubTitle.layer.shadowOffset = CGSizeMake(2.0, 0.0);
    uiLabelSubTitle.lineBreakMode = UILineBreakModeWordWrap; 
    uiLabelSubTitle.numberOfLines = 0;
    uiLabelSubTitle.backgroundColor = [UIColor clearColor];
    uiLabelSubTitle.textAlignment = UITextAlignmentCenter;
    uiLabelSubTitle.textColor = [UIColor whiteColor];
    uiLabelSubTitle.shadowColor = [UIColor blackColor];
    
    // subviews
    [[self view] addSubview:playerView];
    [[self view] addSubview:uiLabelSubTitle];
    [[self view] addSubview:buttonChangeAspect];
    
    [playerView addSubview:controlControls];
    [playerView addSubview:viewControlProgress];
    [playerView addSubview:viewControlSound];
    
    [viewControlProgress addSubview:imageViewControlProgress];
    [viewControlProgress addSubview:labelPlayed];
    [viewControlProgress addSubview:labelLeft];
    [viewControlProgress addSubview:uiSliderProgress];
    
    [viewControlSound addSubview:imageViewControlSound];
    [viewControlSound addSubview:buttonPlay];
    [viewControlSound addSubview:buttonBackward];
    [viewControlSound addSubview:buttonForward];
    [viewControlSound addSubview:buttonPause];
    [viewControlSound addSubview:uiSliderSound];
    
    // set control constant property
    [buttonPlay setImage:[UIImage imageNamed:@"Play.png"] forState:UIControlStateNormal];
    [buttonPlay setShowsTouchWhenHighlighted:YES];
    [buttonPause setImage:[UIImage imageNamed:@"Pause.png"] forState:UIControlStateNormal];
    [buttonPause setShowsTouchWhenHighlighted:YES];
    [buttonBackward setImage:[UIImage imageNamed:@"Backward.png"] forState:UIControlStateNormal];
    [buttonBackward setShowsTouchWhenHighlighted:YES];
    [buttonForward setImage:[UIImage imageNamed:@"Forward.png"] forState:UIControlStateNormal];
    [buttonForward setShowsTouchWhenHighlighted:YES];
    
    [labelLeft setBackgroundColor:[UIColor clearColor]];
    [labelPlayed setBackgroundColor:[UIColor clearColor]];
}

- (void) setSubViewPos
{
    [viewControlProgress setAlpha:0.0f];
    [viewControlSound setAlpha:0.0f];
    CRect rectRet(0, 0, 0, 0);
    bool bPortrait = UIDeviceOrientationIsPortrait([[UIDevice currentDevice] orientation]);
    const CRect rectScreen(0, 0, bPortrait ? 320 : 480, bPortrait ? 480 : 320);
    
    CGFloat fWidth = rectScreen.Width();
    CGFloat fHeight = rectScreen.Height();
    
    [buttonChangeAspect setFrame:CGRectMake(50, 100, 50, 50)];
    [uiLabelSubTitle setFrame:CGRectMake(10.0f, fHeight - 50.0f, fWidth - 20.0f, 20.0f)];
    [self refreshChangeAspectButton];
    // top controller
    {
        const CGFloat fHeightOfUpControlelr = 40.0f;
        [viewControlProgress setFrame:CGRectMake(0.0f, 0.0f, fWidth, fHeightOfUpControlelr)];
        [imageViewControlSound setFrame:[viewControlProgress bounds]];
        const CGFloat fMarginWidth = 5.0f;
        const CGFloat fHeightOfLabel = 18.0f;
        const CGFloat fWidthOfLabel = 40.0f;
        [labelPlayed setFrame:CGRectMake(fMarginWidth, (fHeightOfUpControlelr-fHeightOfLabel)/2, fWidthOfLabel, fHeightOfLabel)];
        [labelLeft setFrame:CGRectMake(fWidth - fMarginWidth - fWidthOfLabel, (fHeightOfUpControlelr-fHeightOfLabel)/2, fWidthOfLabel, fHeightOfLabel)];
        const CGFloat fHeightOfProgress = 10.0f;
        [uiSliderProgress setFrame:CGRectMake(fMarginWidth + fWidthOfLabel + fMarginWidth, (fHeightOfUpControlelr-fHeightOfProgress)/2, fWidth - 2*(fMarginWidth + fWidthOfLabel + fMarginWidth), fHeightOfProgress)];
        
    }
    
    // bottom controller
    {
        const CGFloat fWidthRate = 0.7f;
        const CGFloat fMarginBottom = 30.0f;
        const CGFloat fHeightOfBottomController = 90.0f;
        [viewControlSound setFrame:CGRectMake((fWidth - fWidth*fWidthRate)/2, fHeight - fMarginBottom - fHeightOfBottomController, fWidth*fWidthRate, fHeightOfBottomController)];
        [imageViewControlSound setFrame:[viewControlSound bounds]];
        const CGFloat fMarginWidth = 5.0f;
        const CGFloat fWidthOfButton = 50.0f;
        const CGFloat fHeightOfButton = 50.0f;
        const CGFloat fMarginTop = 5.0f;
        [buttonPlay setFrame:CGRectMake((fWidth*fWidthRate - fWidthOfButton)/2, fMarginTop, fWidthOfButton, fHeightOfButton)];
        [buttonPause setFrame:CGRectMake((fWidth*fWidthRate - fWidthOfButton)/2, fMarginTop, fWidthOfButton, fHeightOfButton)];
        [buttonBackward setFrame:CGRectMake(fMarginWidth, fMarginTop, fWidthOfButton, fHeightOfButton)];
        [buttonForward setFrame:CGRectMake(fWidth*fWidthRate - fMarginWidth - fWidthOfButton, fMarginTop, fWidthOfButton, fHeightOfButton)];
        const CGFloat fHeightOfProgress = 10.0f;
        [uiSliderSound setFrame:CGRectMake(fMarginWidth, fMarginTop + fMarginTop + fHeightOfButton, fWidth*fWidthRate - 2*fMarginWidth, fHeightOfProgress)];
    }
    [playerView calcOnScreenRect];
    [self setSubTitlePos];
}

// Implement viewDidLoad to do additional setup after loading the view, typically from a nib.
- (void)viewDidLoad {
    [super viewDidLoad];
    [self insertSubViews];
    [self setSubViewPos];
//    [self layoutSubviews];
//    buttonPlay.alpha = 0;
//    buttonPause.alpha = 0;
//    buttonBackward.alpha = 0;
//    buttonForward.alpha = 0;
//	[self SetPauseVisiable:NO];
    NSString* str = [[NSBundle mainBundle] pathForResource:@"Test" ofType:@"rmvb"];
    [self performSelector:@selector(open:) withObject:str afterDelay:2.0f];
}

- (void)willRotateToInterfaceOrientation:(UIInterfaceOrientation)toInterfaceOrientation duration:(NSTimeInterval)duration
{
    [playerView startRotate];
}


- (void)didRotateFromInterfaceOrientation:(UIInterfaceOrientation)fromInterfaceOrientation
{
    [self setSubViewPos];
    [playerView stopRotate];
}


// Override to allow orientations other than the default portrait orientation.
-(BOOL)shouldAutorotateToInterfaceOrientation:(UIInterfaceOrientation)toInterfaceOrientation
{
    return YES;
	if (toInterfaceOrientation != UIInterfaceOrientationPortraitUpsideDown) 
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
    self.playerView = nil;
    self.controlControls = nil;
	self.viewControlProgress = nil;
	self.viewControlSound = nil;
	self.buttonPlay = nil;
	self.buttonPause = nil;
	self.buttonBackward = nil;
	self.buttonForward = nil;
	self.labelPlayed = nil;
	self.labelLeft = nil;
	self.uiSliderProgress = nil;
	self.uiSliderSound  = nil;
    self.buttonChangeAspect = nil;
    self.imageViewControlProgress = nil;
    self.imageViewControlSound = nil;
    self.uiLabelSubTitle = nil;
}


- (void)dealloc {
	[playerView release];
    [controlControls release];
	[viewControlProgress release];
	[viewControlSound release];
	[buttonPlay release];
	[buttonPause release];
	[buttonBackward release];
	[buttonForward release];
	[labelPlayed release];
	[labelLeft release];
	[uiSliderProgress release];
	[uiSliderSound release];
    [buttonChangeAspect release];
	[nsTimer release];
    [imageViewControlSound release];
    [imageViewControlProgress release];
    [uiLabelSubTitle release];
    [super dealloc];
}

-(string) GetSrtFileName
{
    return [[[NSBundle mainBundle] pathForResource:@"1" ofType:@"smi"] UTF8String];
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
	// FIXME close timer
}

-(void) Exit
{
	[self Close];
}

-(void) buttonExit:(id)sender
{
	[self Exit];
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

- (void) onTouchUpInsideControlControls:(id)sender
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


-(void) sliderPosChangeProgress:(id)sender
{
	if (m_pLocalPlayer == NULL)
	{
		NSLog(@"buttonPre but m_pLocalPlayer == NULL");
		return;
	}
	m_iControlLife = m_pLocalPlayer->HasVideo() ? AUTO_CONTROL_HIDDEN_TIME : ALWAYS_SHOW_TIME;
	m_iSeekPos = m_pLocalPlayer->m_iDuration * 1.0 * uiSliderProgress.value;
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
    if (m_pLocalPlayer == NULL)
    {
        return;
    }
    
	if (m_pLocalPlayer->m_bReadEndOfFile)
	{
		[self Close];
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
	if (m_pLocalPlayer->HasVideo() && !m_pLocalPlayer->IsPaused()) 
	{
        //[playerView handleTimer];
	}
	
	//return;
	if (m_bInSeek == YES)
	{
		m_pLocalPlayer->m_iCurrentTime = uiSliderProgress.value * m_pLocalPlayer->m_iDuration;
	}
	else
	{
		float fValue = m_pLocalPlayer->m_iCurrentTime * 1.0 / m_pLocalPlayer->m_iDuration;
		uiSliderProgress.value = fValue;
	}

	char szTime[20] = {0};
    labelPlayed.text =  [NSString stringWithCString:FormatTime(m_pLocalPlayer->m_iCurrentTime, true, szTime, sizeof(szTime)) encoding:NSASCIIStringEncoding];
    labelLeft.text = [NSString stringWithCString:FormatTime(m_pLocalPlayer->m_iDuration - m_pLocalPlayer->m_iCurrentTime, false, szTime, sizeof(szTime)) encoding:NSASCIIStringEncoding];
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


- (void) SetPauseVisiable:(BOOL) bVisiable
{
    buttonPause.alpha = (bVisiable == YES) ? 100 : NO;
}

- (void) SetPlayVisiable:(BOOL) bVisiable
{
    buttonPlay.alpha = (bVisiable == YES) ? 100 : NO;
}

- (void) refreshChangeAspectButton
{
    EnumAspectRatio eAspectRatio = (EnumAspectRatio)[[UserDefaultHelper getValue:USER_DEFAULT_ASPECT_RATIO] intValue];
    eAspectRatio ++;
    if (eAspectRatio > eAspectRadioFullScreen)
    {
        eAspectRatio = eAspectRadioOriginal;
    }
    NSArray* arrayTitle = [NSArray arrayWithObjects:@"1:1", @"4:3", @"16:9", @"FS", nil];
    [buttonChangeAspect setTitle:[arrayTitle objectAtIndex:eAspectRatio] forState:UIControlStateNormal];
}

#pragma mark control events
- (void) onTouchUpInsideChangeAspect:(id)sender
{
    EnumAspectRatio eAspectRatio = (EnumAspectRatio)[[UserDefaultHelper getValue:USER_DEFAULT_ASPECT_RATIO] intValue];
    eAspectRatio ++;
    if (eAspectRatio > eAspectRadioFullScreen)
    {
        eAspectRatio = eAspectRadioOriginal;
    }
    [NSString stringWithFormat:@"%d", eAspectRatio];
    [UserDefaultHelper setValue:USER_DEFAULT_ASPECT_RATIO iValue:eAspectRatio];
    [playerView setAspectRadio:eAspectRatio];
    [self refreshChangeAspectButton];
    [playerView calcOnScreenRect];
    [self setSubTitlePos];
}

- (void) onTouchUpInsidePlay:(id)sender
{
    if (m_pLocalPlayer == NULL)
	{
		NSLog(@"buttonPre but m_pLocalPlayer == NULL");
		return;
	}
	m_iControlLife = m_pLocalPlayer->HasVideo() ? AUTO_CONTROL_HIDDEN_TIME : ALWAYS_SHOW_TIME;
	if (m_pLocalPlayer->IsPaused())
	{
        m_pLocalPlayer->SetPlaySpeed(ePlaySpeedNormal);
		m_pLocalPlayer->Play();
		[self SetPauseVisiable:YES];
		[self SetPlayVisiable:NO];
	}
}
- (void) onTouchUpInsidePause:(id)sender
{
    if (m_pLocalPlayer == NULL)
	{
		NSLog(@"buttonPre but m_pLocalPlayer == NULL");
		return;
	}
	m_iControlLife = m_pLocalPlayer->HasVideo() ? AUTO_CONTROL_HIDDEN_TIME : ALWAYS_SHOW_TIME;
	if (!m_pLocalPlayer->IsPaused())
	{
		m_pLocalPlayer->Pause();
		[self SetPauseVisiable:NO];
		[self SetPlayVisiable:YES];
	}
}
- (void) onTouchUpInsideBackword:(id)sender
{
    if (m_pLocalPlayer == NULL)
    {
        return;
    }
    EnumPlaySpeed ePlaySpeed = m_pLocalPlayer->GetPlaySpeed();
    ePlaySpeed --;
    if (ePlaySpeed < ePlaySpeedHalf)
    {
        ePlaySpeed = ePlaySpeedHalf;
    }
    m_pLocalPlayer->SetPlaySpeed(ePlaySpeed);
}
- (void) onTouchUpInsideForward:(id)sender
{
    if (m_pLocalPlayer == NULL)
    {
        return;
    }
    EnumPlaySpeed ePlaySpeed = m_pLocalPlayer->GetPlaySpeed();
    ePlaySpeed ++;
    if (ePlaySpeed > ePlaySpeedForth)
    {
        ePlaySpeed = ePlaySpeedForth;
    }
    m_pLocalPlayer->SetPlaySpeed(ePlaySpeed);
}

#pragma mark interface imp
- (EnumPlayerStatus) open:(NSString*)strFileName
{
	m_strFileName = [strFileName UTF8String] ;
    // init all controls
	labelPlayed.text = @"";
	labelLeft.text = @"";
	//[[playerView uiLabel] setText:@""];
	uiSliderProgress.value = 0.0f;
	m_bInSeek = NO;
    
    m_pLocalPlayer = new CLocalPlayer;
    if (m_pLocalPlayer == NULL)
    {
        return ePlayerStatusNotEnoughMemory;
    }
    m_pLocalPlayer->m_strSubTitleLocal = [self GetSrtFileName];
    m_pLocalPlayer->m_iCodePage = m_iCodePage;
    
    RETURN_STATUS_IF_ERROR(m_pLocalPlayer->Open(m_strFileName, (int)playerView));
    uiSliderSound.value = [[UserDefaultHelper getValue:USER_DEFAULT_VOLUME] intValue] / 100.0f;
	m_pLocalPlayer->SetVolume([[UserDefaultHelper getValue:USER_DEFAULT_VOLUME] intValue]); 
	m_pLocalPlayer->SetPlaySpeed(ePlaySpeedNormal);
	[self SetPauseVisiable:NO];
	[self SetPlayVisiable:YES];
	
	[self SetControlVisiable:YES];
	bool bHasVideo = m_pLocalPlayer->HasVideo();
    if (!bHasVideo)
    {
        [self Exit];
        return ePlayerStatusError;
    }
	m_iControlLife = bHasVideo ? AUTO_CONTROL_HIDDEN_TIME : ALWAYS_SHOW_TIME;
	m_pLocalPlayer->Start();
	[self SetPauseVisiable:YES];
	[self SetPlayVisiable:NO];
	[self SetControlVisiable:YES];
	
//	nsTimer = [NSTimer scheduledTimerWithTimeInterval:1.0/15.0f 
//                                               target:self
//                                             selector:@selector(handleTimer:)
//                                             userInfo:nil
//                                              repeats:YES];
	

    return ePlayerStatusOk;
}

- (EnumPlayerStatus) play
{
    return [self pause];
}
- (EnumPlayerStatus) pause
{
    m_iControlLife = ALWAYS_SHOW_TIME;
	[self SetControlVisiable:YES];
	if (m_pLocalPlayer == NULL)
	{
		return ePlayerStatusError;
	}
	if (!m_pLocalPlayer->IsPaused())
	{
		m_pLocalPlayer->Pause();
		[self SetPauseVisiable:NO];
		[self SetPlayVisiable:YES];
	}
    return ePlayerStatusOk;
}
- (EnumPlayerStatus) close
{
    [self Exit];
    return ePlayerStatusOk;
}

- (unsigned long) getDuration
{
    if (m_pLocalPlayer == NULL)
	{
		return 0;
	}
    return m_pLocalPlayer->m_iDuration;
}

- (unsigned long) getPlayingTime
{
    if (m_pLocalPlayer == NULL)
	{
		return 0;
	}
    return m_pLocalPlayer->m_iCurrentTime;
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
    if (m_pLocalPlayer == NULL)
	{
		return ePlaySpeedNormal;
	}
    return m_pLocalPlayer->GetPlaySpeed();
}

- (EnumPlayerStatus) setAspectRatio:(EnumAspectRatio)eAspectRatio
{
    return ePlayerStatusNotImp;
}

- (NSString*) dealWithEscapeChars:(NSString*)str;
{
    str = [str stringByReplacingOccurrencesOfString:@"&nbsp;" withString:@" "];
    str = [str stringByReplacingOccurrencesOfString:@"<br>" withString:@"\n"];
    
    return str;
}
- (void) showWithSubTitle:(NSString*)str
{
    [uiLabelSubTitle setText:[self dealWithEscapeChars:str]];
    if (m_pLocalPlayer == NULL)
    {
        return;
    }
    
	if (m_pLocalPlayer->m_bReadEndOfFile)
	{
		[self Close];
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

	
	//return;
	if (m_bInSeek == YES)
	{
		m_pLocalPlayer->m_iCurrentTime = uiSliderProgress.value * m_pLocalPlayer->m_iDuration;
	}
	else
	{
		float fValue = m_pLocalPlayer->m_iCurrentTime * 1.0 / m_pLocalPlayer->m_iDuration;
		uiSliderProgress.value = fValue;
	}
    
	char szTime[20] = {0};
    labelPlayed.text =  [NSString stringWithCString:FormatTime(m_pLocalPlayer->m_iCurrentTime, true, szTime, sizeof(szTime)) encoding:NSASCIIStringEncoding];
    labelLeft.text = [NSString stringWithCString:FormatTime(m_pLocalPlayer->m_iDuration - m_pLocalPlayer->m_iCurrentTime, false, szTime, sizeof(szTime)) encoding:NSASCIIStringEncoding];
}

- (void)setSubTitlePos
{
    bool bPortrait = UIDeviceOrientationIsPortrait([[UIDevice currentDevice] orientation]);
    const CRect rectScreen(0, 0, bPortrait ? 320 : 480, bPortrait ? 450 : 290);
    CGRect rectOnScreen = playerView.m_rectOnScreen;
    if ((int(rectOnScreen.size.width) == 0) || (int(rectOnScreen.size.height) == 0))
    {
        rectOnScreen = CGRectMake(0.0f, 0.0f, rectScreen.Width(), rectScreen.Height());
    }
    CGFloat fTop = rectOnScreen.origin.y + rectOnScreen.size.height;
    CGFloat fBottom = rectScreen.top + rectScreen.Height();
    const CGFloat fHeihgt = 50.0f;
    if (fBottom - fTop < fHeihgt)
    {
        fTop = fBottom - fHeihgt;
    }
    [uiLabelSubTitle setFrame:CGRectMake(0.0f, fTop, rectScreen.Width(), fHeihgt)];
}


@end

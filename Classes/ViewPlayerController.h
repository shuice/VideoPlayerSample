//
//  ViewPlayerController.h
//  Player
//
//  Create by xiaoyi liao  on 11-2-26.
//  Copyright 2011 . All rights reserved.
//

#import <UIKit/UIKit.h>
#import "LocalPlayer.h"
#include <string>
using namespace std;

#import "PlayerView.h"
@interface ViewPlayerController : UIViewController 
{
	IBOutlet PlayerView* playerView;
	IBOutlet UIView* viewControlProgress;
	IBOutlet UIView* viewControlSound;
	IBOutlet UIButton* buttonPlay;
	IBOutlet UIButton* buttonPause;
	IBOutlet UIButton* buttonBackward;
	IBOutlet UIButton* buttonForward;
	IBOutlet UIButton* buttonPlaySmall;
	IBOutlet UIButton* buttonPauseSmall;
	IBOutlet UIButton* buttonBackwardSmall;
	IBOutlet UIButton* buttonForwardSmall;
	IBOutlet UILabel* labelPlayed;
	IBOutlet UILabel* labelLeft;
	IBOutlet UISlider* uiSliderProgress;
	IBOutlet UISlider* uiSliderSound;
	
	IBOutlet UIImageView* uiImageViewLightBackward;
	IBOutlet UIImageView* uiImageViewLightPlay;
	IBOutlet UIImageView* uiImageViewLightForward;
	IBOutlet UIImageView* uiImageViewAudio;
	NSTimer* nsTimer;
	CLocalPlayer* m_pLocalPlayer;
	string m_strFileName;
	int m_iControlLife;
	
	
	unsigned long m_iDuration;
	unsigned long m_iCurrentTime;
	unsigned long m_iInNetSeeking;
	bool m_bIsReadEndOfFile;
	int socketCommand;
	int socketCache;
	BOOL m_bInSeek;
	unsigned long m_iSeekPos;
	EnumPlayerType	m_ePlayerType;
	string m_strSrtPath;
	int m_iCodePage;
}

@property (nonatomic, retain) PlayerView* playerView;
@property (nonatomic, retain) UIView* viewControlProgress;
@property (nonatomic, retain) UIView* viewControlSound;
@property (nonatomic, retain) UIButton* buttonPlay;
@property (nonatomic, retain) UIButton* buttonPause;
@property (nonatomic, retain) UIButton* buttonBackward;
@property (nonatomic, retain) UIButton* buttonForward;
@property (nonatomic, retain) UIButton* buttonPlaySmall;
@property (nonatomic, retain) UIButton* buttonPauseSmall;
@property (nonatomic, retain) UIButton* buttonBackwardSmall;
@property (nonatomic, retain) UIButton* buttonForwardSmall;
@property (nonatomic, retain) UILabel* labelPlayed;
@property (nonatomic, retain) UILabel* labelLeft;

@property (nonatomic, assign) string m_strSrtPath;
@property (nonatomic, assign) int m_iCodePage;


@property (nonatomic, retain) UISlider* uiSliderProgress;
@property (nonatomic, retain) UISlider* uiSliderSound;
@property (nonatomic, retain) UIImageView* uiImageViewLightBackward;
@property (nonatomic, retain) UIImageView* uiImageViewLightPlay;
@property (nonatomic, retain) UIImageView* uiImageViewLightForward;
@property (nonatomic, retain) UIImageView* uiImageViewAudio;
@property (nonatomic, retain) NSTimer* nsTimer;
@property (nonatomic, assign) CLocalPlayer* m_pLocalPlayer;
@property (nonatomic, assign) string m_strFileName;
@property (nonatomic, assign) int m_iControlLife;


@property (nonatomic, assign)unsigned long m_iDuration;
@property (nonatomic, assign)unsigned long m_iCurrentTime;
@property (nonatomic, assign)unsigned long m_iInNetSeeking;
@property (nonatomic, assign)bool m_bIsReadEndOfFile;
@property (nonatomic, assign) int socketCommand;
@property (nonatomic, assign) int socketCache;
@property (nonatomic, assign) EnumPlayerType m_ePlayerType;

-(void) StartPlay:(string)strFileName;
-(BOOL) Open;
-(void) Pause;
-(void) Close;
-(void) onTouchPlayerView:(id)sender;

-(void) buttonExit:(id)sender;

-(void) buttonPlay:(id)sender;
-(void) buttonPause:(id)sender;
-(void) buttonPre:(id)sender;
-(void) buttonNext:(id)sender;
-(void) buttonPlaySmall:(id)sender;
-(void) buttonPauseSmall:(id)sender;
-(void) buttonPreSmall:(id)sender;
-(void) buttonNextSmall:(id)sender;

-(void) sliderPosChangeProgress:(id)sender;
-(void) sliderPosChangeSound:(id)sender;
-(void) SetControlVisiable:(BOOL)bVisiable;
- (void) SetPauseVisiable:(BOOL) bVisiable;
- (void) SetPlayVisiable:(BOOL) bVisiable;
-(void) OnTouchInPos:(id)sender;
-(void) OnTouchOutPos:(id)sender;

@end

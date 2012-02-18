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
#import "UserDefaultHelper.h"
using namespace std;
#import "PlayerViewController.h"
#import "PlayerView.h"

@class PlayerView;
@interface PlayerViewControllerImp : PlayerViewController
{
    PlayerView* playerView;
    UIControl* controlControls;
    UIView* viewControlProgress;
    UIImageView* imageViewControlProgress;
    UIView* viewControlSound;
    UIImageView* imageViewControlSound;
    UIButton* buttonPlay;
    UIButton* buttonPause;
    UIButton* buttonBackward;
    UIButton* buttonForward;
    UILabel* labelPlayed;
    UILabel* labelLeft;
    UISlider* uiSliderProgress;
    UISlider* uiSliderSound;
    UIButton* buttonChangeAspect;
    UILabel* uiLabelSubTitle;
    
    
	NSTimer* nsTimer;
	CLocalPlayer* m_pLocalPlayer;
	string m_strFileName;
	int m_iControlLife;
	
	
	int socketCommand;
	int socketCache;
	BOOL m_bInSeek;
	unsigned long m_iSeekPos;
	string m_strSrtPath;
	int m_iCodePage;
}

@property (nonatomic, retain) PlayerView* playerView;
@property (nonatomic, retain) UIControl* controlControls;
@property (nonatomic, retain) UIView* viewControlProgress;
@property (nonatomic, retain) UIImageView* imageViewControlProgress;
@property (nonatomic, retain) UIView* viewControlSound;
@property (nonatomic, retain) UIImageView* imageViewControlSound;
@property (nonatomic, retain) UIButton* buttonPlay;
@property (nonatomic, retain) UIButton* buttonPause;
@property (nonatomic, retain) UIButton* buttonBackward;
@property (nonatomic, retain) UIButton* buttonForward;
@property (nonatomic, retain) UILabel* labelPlayed;
@property (nonatomic, retain) UILabel* labelLeft;
@property (nonatomic, retain) UIButton* buttonChangeAspect;
@property (nonatomic, retain) UILabel* uiLabelSubTitle;

@property (nonatomic, assign) string m_strSrtPath;
@property (nonatomic, assign) int m_iCodePage;


@property (nonatomic, retain) UISlider* uiSliderProgress;
@property (nonatomic, retain) UISlider* uiSliderSound;
@property (nonatomic, retain) NSTimer* nsTimer;
@property (nonatomic, assign) CLocalPlayer* m_pLocalPlayer;
@property (nonatomic, assign) string m_strFileName;
@property (nonatomic, assign) int m_iControlLife;

-(void) Close;
-(void) onTouchPlayerView:(id)sender;

-(void) buttonExit:(id)sender;

-(void) buttonPlay:(id)sender;
-(void) buttonPause:(id)sender;
-(void) buttonPre:(id)sender;
-(void) buttonNext:(id)sender;

-(void) sliderPosChangeProgress:(id)sender;
-(void) sliderPosChangeSound:(id)sender;
-(void) SetControlVisiable:(BOOL)bVisiable;
- (void) SetPauseVisiable:(BOOL) bVisiable;
- (void) SetPlayVisiable:(BOOL) bVisiable;
-(void) OnTouchInPos:(id)sender;
-(void) OnTouchOutPos:(id)sender;
- (void) showWithSubTitle:(NSString*)str;
@end

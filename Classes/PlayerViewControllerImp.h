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
    UIButton* buttonExit;
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
    id<PlayerViewControllerDelegate> m_delegate;
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
@property (nonatomic, retain) UIButton* buttonExit;
@property (nonatomic, retain) UILabel* labelPlayed;
@property (nonatomic, retain) UILabel* labelLeft;
@property (nonatomic, retain) UIButton* buttonChangeAspect;
@property (nonatomic, retain) UILabel* uiLabelSubTitle;

@property (nonatomic, retain) UISlider* uiSliderProgress;
@property (nonatomic, retain) UISlider* uiSliderSound;
@property (nonatomic, retain) NSTimer* nsTimer;
@property (nonatomic, assign) CLocalPlayer* m_pLocalPlayer;
@property (nonatomic, assign) string m_strFileName;
@property (nonatomic, assign) int m_iControlLife;


- (void) showWithSubTitle:(NSString*)str;
- (void) playFinish;
@end

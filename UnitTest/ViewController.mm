//
//  ViewController.m
//  UnitTest
//
//  Created by xiaoyi on 12-1-11.
//  Copyright (c) 2012年 __MyCompanyName__. All rights reserved.
//

#import "ViewController.h"
#import "../Classes/SmiSubTitleReader.h"
#import "../Classes/PlayerViewController.h"

@implementation ViewController

- (void)didReceiveMemoryWarning
{
    [super didReceiveMemoryWarning];
    // Release any cached data, images, etc that aren't in use.
}

#pragma mark - View lifecycle

- (void)viewDidLoad
{
    [super viewDidLoad];
    [[self view] setBackgroundColor:[UIColor grayColor]];
	// Do any additional setup after loading the view, typically from a nib.
    UIButton* button1 = [UIButton buttonWithType:UIButtonTypeRoundedRect];
    [button1 setFrame:CGRectMake(50, 50, 50, 50)];
    [button1 addTarget:self action:@selector(unitTest1:) forControlEvents:UIControlEventTouchUpInside];
    [button1 setTitle:@"Test" forState:UIControlStateNormal];
    
    UIButton* button2 = [UIButton buttonWithType:UIButtonTypeRoundedRect];
    [button2 setFrame:CGRectMake(110, 50, 50, 50)];
    [button2 addTarget:self action:@selector(unitTest2:) forControlEvents:UIControlEventTouchUpInside];
    [button2 setTitle:@"Test" forState:UIControlStateNormal];

    
    UIButton* button3 = [UIButton buttonWithType:UIButtonTypeRoundedRect];
    [button3 setFrame:CGRectMake(170, 50, 50, 50)];
    [button3 addTarget:self action:@selector(unitTest3:) forControlEvents:UIControlEventTouchUpInside];
    [button3 setTitle:@"Test" forState:UIControlStateNormal];

    [[self view] addSubview:button1];
    [[self view] addSubview:button2];
    [[self view] addSubview:button3];
}

- (void)viewDidUnload
{
    [super viewDidUnload];
    // Release any retained subviews of the main view.
    // e.g. self.myOutlet = nil;
    
}

- (void)viewWillAppear:(BOOL)animated
{
    [super viewWillAppear:animated];
}

- (void)viewDidAppear:(BOOL)animated
{
    [super viewDidAppear:animated];
}

- (void)viewWillDisappear:(BOOL)animated
{
	[super viewWillDisappear:animated];
}

- (void)viewDidDisappear:(BOOL)animated
{
	[super viewDidDisappear:animated];
}

-(BOOL)shouldAutorotateToInterfaceOrientation:(UIInterfaceOrientation)toInterfaceOrientation
{
	if ((toInterfaceOrientation == UIInterfaceOrientationPortrait)
        || (toInterfaceOrientation == UIInterfaceOrientationPortraitUpsideDown)
        || (toInterfaceOrientation == UIInterfaceOrientationLandscapeLeft) 
        || (toInterfaceOrientation == UIInterfaceOrientationLandscapeRight)) 
	{
		return YES;
	}
	return NO;	
}


- (void) unitTest1:(id)sender
{
    PlayerViewController* player = [PlayerViewController sharedPlayer];
    NSString* strFilePath = [[NSBundle mainBundle] pathForResource:@"Test" ofType:@"rmvb"];
    EnumPlayerStatus ePlayerStatus = [player open:strFilePath 
                                      andDelegate:self];
    if (ePlayerStatus == ePlayerStatusOk)
    {
        [[self navigationController] pushViewController:player animated:YES];
        [player play];
    }
}

- (void) unitTest2:(id)sender
{
    PlayerViewController* player = [PlayerViewController sharedPlayer];
    NSString* strFilePath = [[NSBundle mainBundle] pathForResource:@"Test" ofType:@"mpg"];
    NSString* strSubTitle = [[NSBundle mainBundle] pathForResource:@"1" ofType:@"smi"];
    EnumPlayerStatus ePlayerStatus = [player open:strFilePath 
                                      andDelegate:self];
    [player setSubTitle:strSubTitle andCodePage:0];
    if (ePlayerStatus == ePlayerStatusOk)
    {
        [[self navigationController] pushViewController:player animated:YES];
        [player play];
    }
}

- (void) unitTest3:(id)sender
{
    PlayerViewController* player = [PlayerViewController sharedPlayer];
    NSString* strFilePath = [[NSBundle mainBundle] pathForResource:@"Test" ofType:@"flv"];
    NSString* strSubTitle = [[NSBundle mainBundle] pathForResource:@"1" ofType:@"srt"];
    EnumPlayerStatus ePlayerStatus = [player open:strFilePath 
                                        andDelegate:self];
    [player setSubTitle:strSubTitle andCodePage:936];
    if (ePlayerStatus == ePlayerStatusOk)
    {
        [[self navigationController] pushViewController:player animated:YES];
        [player play];
    }
}

- (void) playFinish
{
    [[self navigationController] popViewControllerAnimated:YES];
}

@end

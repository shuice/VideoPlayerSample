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
    UIButton* button = [UIButton buttonWithType:UIButtonTypeRoundedRect];
    [button setFrame:CGRectMake(50, 50, 50, 50)];
    [button addTarget:self action:@selector(unitTest:) forControlEvents:UIControlEventTouchUpInside];
    [button setTitle:@"Test" forState:UIControlStateNormal];
    [[self view] addSubview:button];
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


- (void) unitTest:(id)sender
{
    PlayerViewController* player = [PlayerViewController sharedPlayer];
    
    NSString* strFilePath = [[NSBundle mainBundle] pathForResource:@"Test" ofType:@"rmvb"];
    EnumPlayerStatus ePlayerStatus = [player open:strFilePath];
    if (ePlayerStatus == ePlayerStatusOk)
    {
        [[self navigationController] pushViewController:player animated:YES];
        [player play];
    }
}

@end

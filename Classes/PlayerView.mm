/*
 
 glgif
 
 PlayerView - example view to play the GifVideo.
 
 Copyright (C) 2009 James S Urquhart
 
 Permission is hereby granted, free of charge, to any person
 obtaining a copy of this software and associated documentation
 files (the "Software"), to deal in the Software without
 restriction, including without limitation the rights to use,
 copy, modify, merge, publish, distribute, sublicense, and/or sell
 copies of the Software, and to permit persons to whom the
 Software is furnished to do so, subject to the following
 conditions:
 
 The above copyright notice and this permission notice shall be
 included in all copies or substantial portions of the Software.
 
 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 OTHER DEALINGS IN THE SOFTWARE.
*/

#import "PlayerView.h"

//#import "Video.h"
//#import "GifVideo.h"
//#import "GLGifExampleAppDelegate.h"

#import <QuartzCore/QuartzCore.h>
#import <OpenGLES/EAGLDrawable.h>
#import "ViewPlayerController.h"
// A class extension to declare private methods
@interface PlayerView ()



@end

@implementation PlayerView

@synthesize context;
@synthesize m_wstrSubTitle;
@synthesize uiLabel;


// You must implement this method
+ (Class)layerClass {
    return [CAEAGLLayer class];
}


- (id)initWithFrame:(CGRect)frame {
    if (self = [super initWithFrame:frame]) {
       
    }
	
    return self;
}


- (id)initWithCoder:(NSCoder*)coder {
    pthread_mutex_init(&m_mutexFromView, NULL);
    if ((self = [super initWithCoder:coder])) 
	{
        // Get the layer
        CAEAGLLayer *eaglLayer = (CAEAGLLayer *)self.layer;
        
        eaglLayer.opaque = YES;
        eaglLayer.drawableProperties = [NSDictionary dictionaryWithObjectsAndKeys:
                                        [NSNumber numberWithBool:NO],
										kEAGLDrawablePropertyRetainedBacking,
										kEAGLColorFormatRGBA8, 
										kEAGLDrawablePropertyColorFormat,
										nil];
        context = [[EAGLContext alloc] initWithAPI:kEAGLRenderingAPIOpenGLES1];
        if (!context || ![EAGLContext setCurrentContext:context]) 
		{
            [self release];
            return nil;
        }
        
       [self layoutSubviews];
    }
	
	
    return self;
}

- (BOOL)createFramebuffer
{
	UIInterfaceOrientation uiInterfaceOrientation = [UIApplication sharedApplication].statusBarOrientation;
	if ((uiInterfaceOrientation == UIInterfaceOrientationPortrait) 
		|| (uiInterfaceOrientation == UIInterfaceOrientationPortraitUpsideDown))
	{
		return FALSE;
	}
	NSLog(@"Create Frame buffer");
    glGenFramebuffersOES(1, &viewFramebuffer);
    glGenRenderbuffersOES(1, &viewRenderbuffer);
    
    glBindFramebufferOES(GL_FRAMEBUFFER_OES,  viewFramebuffer);
    glBindRenderbufferOES(GL_RENDERBUFFER_OES, viewRenderbuffer);
    [context renderbufferStorage:GL_RENDERBUFFER_OES 
					fromDrawable:(CAEAGLLayer*)self.layer];
    glFramebufferRenderbufferOES(GL_FRAMEBUFFER_OES, 
								 GL_COLOR_ATTACHMENT0_OES, 
								 GL_RENDERBUFFER_OES, 
								 viewRenderbuffer);
    
    glGetRenderbufferParameterivOES(GL_RENDERBUFFER_OES, 
									GL_RENDERBUFFER_WIDTH_OES, 
									&backingWidth);
    glGetRenderbufferParameterivOES(GL_RENDERBUFFER_OES, 
									GL_RENDERBUFFER_HEIGHT_OES, 
									&backingHeight);
	    if(glCheckFramebufferStatusOES(GL_FRAMEBUFFER_OES) != GL_FRAMEBUFFER_COMPLETE_OES)
	{
        NSLog(@"failed to make complete framebuffer object %x",
			  glCheckFramebufferStatusOES(GL_FRAMEBUFFER_OES));
        return NO;
    }
	
	glViewport(0, 0, backingWidth,  backingHeight);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrthof(0.0f, backingWidth, 0.0f, backingHeight, 0.0f, 1.0f);
	
	widthOfSuitable = backingWidth;
	heightOfSuitable = backingHeight;
	if (widthOfSuitable * heightOfSuitable > 480 * 320)
	{
		widthOfSuitable = 480;
		heightOfSuitable = 320;
	}
	
	// create texture
	glGenTextures(1, &texture);
	glBindTexture( GL_TEXTURE_2D, texture);
	
	// Filters
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
	
	// Wrap
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glEnable(GL_TEXTURE_2D);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	glEnableClientState(GL_VERTEX_ARRAY);		
	
	uiLabel.layer.shadowRadius = 5.0;
	uiLabel.layer.shadowOpacity = 0.7;
	uiLabel.layer.shadowColor = [[UIColor orangeColor] CGColor];
	uiLabel.layer.shadowOffset = CGSizeMake(2.0, 0.0);
	uiLabel.lineBreakMode = UILineBreakModeWordWrap; 
	uiLabel.numberOfLines = 0;
	
    return YES;
}

- (BOOL)createTextureAndBuffer:(int)width height:(int)height
{
	NSLog(@"createTextureAndBuffer");
	widthOfPicture = width;
	heightOfPicture = height;
	
	CRect rectPicture(0, 0, widthOfPicture, heightOfPicture);
	CRect rectWindow(0, 0, backingWidth, backingHeight);
	RectStretchAdapt(&rectPicture, &rectWindow, &rectAdapt);
	

	m_pRGBADataFromView = new unsigned char[widthOfPicture * heightOfPicture * 4];
	return YES;
}

- (void)drawView 
{
    [EAGLContext setCurrentContext:context];
	glClearColor(1.0f, 0.0f, 0.0f, 1.0f);
    [context presentRenderbuffer:GL_RENDERBUFFER_OES];
}

- (void)destroyFramebuffer {
    
	NSLog(@"destroyFramebuffer");
    glDeleteFramebuffersOES(1, &viewFramebuffer);
    viewFramebuffer = 0;
    glDeleteRenderbuffersOES(1, &viewRenderbuffer);
	
    viewRenderbuffer = 0;
	backingWidth = 0;
	backingHeight = 0;
	widthOfSuitable = 0;
	heightOfSuitable = 0;
}

- (void)destroyTexturebuffer 
{
	
	NSLog(@"destroyTexturebuffer");
	
	widthOfPicture = 0;
	heightOfPicture = 0;
	rectAdapt = CRect(0, 0, 0, 0);
	if (m_pRGBADataFromView != NULL)
	{
		delete[] m_pRGBADataFromView;
		m_pRGBADataFromView = NULL;
	}
	
	
//	glDisable(GL_TEXTURE_2D);
//	glDisableClientState(GL_TEXTURE_COORD_ARRAY);
//	glDisableClientState(GL_VERTEX_ARRAY);
	
}

- (void)layoutSubviews {
    [EAGLContext setCurrentContext:context];
    [self destroyFramebuffer];
    [self createFramebuffer];
    [self drawView];
}




- (void)drawRect:(CGRect)rect {
    // Drawing code
}


- (void)dealloc {
    
    if ([EAGLContext currentContext] == context) {
        [EAGLContext setCurrentContext:nil];
    }
    
    [context release]; 
	pthread_mutex_destroy(&m_mutexFromView);
	glDisable(GL_TEXTURE_2D);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);
	glDisableClientState(GL_VERTEX_ARRAY);
	glBindTexture(GL_TEXTURE_2D, 0);
	glDeleteTextures(1, &texture);
	[uiLabel release];
    [super dealloc];
}
extern bool saveBmp(const char* bmpName,unsigned char *imgBuf,int width,int height,int biBitCount);

- (void)eraseBackground
{
	if (m_pRGBADataFromView != NULL)
	{
		memset(m_pRGBADataFromView, 0, widthOfPicture * heightOfPicture * 4);
		for (int i = 0; i < 10; i ++)
		{
			[self handleTimer];

		}
	}
	return;
}

- (void)handleTimer
{
	const GLfloat squareVertices[] = {
		rectAdapt.left, rectAdapt.bottom, 
		rectAdapt.left, rectAdapt.top, 
		rectAdapt.right, rectAdapt.top, 
		rectAdapt.right, rectAdapt.bottom};
	glColor4f(1.0f, 1.0f, 1.0f, 0.0f);
	static GLfloat squareTexcoords[] = {
		0, 1,		// top left
		0, 0,		// bottom left	
		1, 0, 		
		1, 1,	// bottom right	
		
	};
	

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	pthread_mutex_lock(&m_mutexFromView);
	//AddLog("View locked");
	glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA, widthOfPicture, heightOfPicture,
				 0, GL_RGBA, GL_UNSIGNED_BYTE, m_pRGBADataFromView);
	{
		// subtitle;
		NSData* nsData = [NSData dataWithBytes:m_wstrSubTitle.c_str()
										length:m_wstrSubTitle.size()*sizeof(wchar_t)];
		NSString* nsText = [[NSString alloc] initWithData:nsData 
												 encoding:NSUTF32LittleEndianStringEncoding];
		uiLabel.text = nsText;
		[nsText release];
	}	
	pthread_mutex_unlock(&m_mutexFromView);
	//AddLog("View unlocked");
	
	glVertexPointer(2, GL_FLOAT, 0, squareVertices);
	glTexCoordPointer(2, GL_FLOAT, 0, squareTexcoords);
	glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

	[context presentRenderbuffer:GL_RENDERBUFFER_OES];
}

@end

#import "PlayerView.h"
#import <QuartzCore/QuartzCore.h>
#import <OpenGLES/EAGLDrawable.h>
#import "PlayerViewControllerImp.h"



@implementation PlayerView

@synthesize m_wstrSubTitle;



// You must implement this method
+ (Class)layerClass {
    return [CAEAGLLayer class];
}

- (void) resizeData:(int)iSize
{
    if (m_iDataLen >= iSize)
    {
        return;
    }
    if (m_pData != NULL)
    {
        delete m_pData;
        m_pData = NULL;
    }
    m_iDataLen = iSize;
    m_pData = new unsigned char[iSize];
}

- (void) test
{
    CGImageRef inImage = [[UIImage imageNamed:@"1.png"] CGImage];
    
        CGContextRef    context1 = NULL;
        CGColorSpaceRef colorSpace;
        int             bitmapByteCount;
        int             bitmapBytesPerRow;
        
        size_t width = CGImageGetWidth(inImage);
        size_t height = CGImageGetHeight(inImage);
        bitmapBytesPerRow   = (width * 4);
        bitmapByteCount     = bitmapBytesPerRow * height;
        
        colorSpace = CGColorSpaceCreateDeviceRGB();
        size_t bitsPerComponent = 8;
        context1 = CGBitmapContextCreate (NULL, 
                                         width, 
                                         height,	
                                         bitsPerComponent,
                                         bitmapBytesPerRow, 
                                         colorSpace, 
                                         kCGImageAlphaNoneSkipFirst);
        
        
    
    CGRect rect = {{0,0},{PLAYER_FRAME_WIDTH, PLAYER_FRAME_HEIGHT}}; 
    CGContextDrawImage(context1, rect, inImage); 
	void *data = CGBitmapContextGetData (context1);
    memcpy(m_pData, data, PLAYER_FRAME_WIDTH*PLAYER_FRAME_HEIGHT*4);

    CGColorSpaceRelease( colorSpace );
}

- (id)initWithFrame:(CGRect)frame {
    pthread_mutex_init(&m_mutexFromView, NULL);
    if (self = [super initWithFrame:frame]) 
	{
        uiLabel = [[UILabel alloc] initWithFrame:CGRectZero];
        uiLabel.layer.shadowRadius = 5.0;
        uiLabel.layer.shadowOpacity = 0.7;
        uiLabel.layer.shadowColor = [[UIColor orangeColor] CGColor];
        uiLabel.layer.shadowOffset = CGSizeMake(2.0, 0.0);
        uiLabel.lineBreakMode = UILineBreakModeWordWrap; 
        uiLabel.numberOfLines = 0;

        CAEAGLLayer *eaglLayer = (CAEAGLLayer *)self.layer;
        eaglLayer.opaque = YES;
        eaglLayer.drawableProperties = [NSDictionary dictionaryWithObjectsAndKeys:
                                        [NSNumber numberWithBool:NO], kEAGLDrawablePropertyRetainedBacking,
                                        kEAGLColorFormatRGBA8, kEAGLDrawablePropertyColorFormat,
                                        nil];
        
        context = [[EAGLContext alloc] initWithAPI:kEAGLRenderingAPIOpenGLES1];        
        [EAGLContext setCurrentContext:context];
        
        glGenFramebuffersOES(1, &glFramebuffer);
        glGenRenderbuffersOES(1, &glRenderbuffer);
        glBindFramebufferOES(GL_FRAMEBUFFER_OES, glFramebuffer);
        glBindRenderbufferOES(GL_RENDERBUFFER_OES, glRenderbuffer);
        glFramebufferRenderbufferOES(GL_FRAMEBUFFER_OES, GL_COLOR_ATTACHMENT0_OES, GL_RENDERBUFFER_OES, glRenderbuffer); 
        
        [context renderbufferStorage:GL_RENDERBUFFER_OES fromDrawable:(CAEAGLLayer*)self.layer];
        
        glViewport(0, 0, PLAYER_FRAME_WIDTH,  PLAYER_FRAME_HEIGHT);
        glMatrixMode(GL_PROJECTION);
        glLoadIdentity(); 
        glOrthof(0.0f, PLAYER_FRAME_WIDTH, 0.0f, PLAYER_FRAME_HEIGHT, 0.0f, 1.0f); 
        
        // create texture
        glGenTextures(1, &glTexture);
        glBindTexture( GL_TEXTURE_2D, glTexture);
        
        // Filters
        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
        
        // Wrap
        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glEnable(GL_TEXTURE_2D);
        glEnableClientState(GL_TEXTURE_COORD_ARRAY);
        glEnableClientState(GL_VERTEX_ARRAY);	
        
        [self resizeData:PLAYER_FRAME_WIDTH*PLAYER_FRAME_HEIGHT*4];
        [self test];
    }
    return self;
}

- (void)drawView 
{
    [EAGLContext setCurrentContext:context];
	glClearColor(1.0f, 0.0f, 0.0f, 1.0f);
    [context presentRenderbuffer:GL_RENDERBUFFER_OES];
}

- (void)layoutSubviews {
    [self test];
}



- (void)dealloc {
    
    if ([EAGLContext currentContext] == context) {
        [EAGLContext setCurrentContext:nil];
    }
    
    glDeleteFramebuffersOES(1, &glFramebuffer);
    glDeleteRenderbuffersOES(1, &glRenderbuffer);
    
    [context release]; 
	pthread_mutex_destroy(&m_mutexFromView);
	glDisable(GL_TEXTURE_2D);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);
	glDisableClientState(GL_VERTEX_ARRAY);
	glBindTexture(GL_TEXTURE_2D, 0);
	glDeleteTextures(1, &glTexture);
	[uiLabel release];
    [super dealloc];
}
extern bool saveBmp(const char* bmpName,unsigned char *imgBuf,int width,int height,int biBitCount);


- (CGRect) calcOnScreenRect
{
    CRect rectRet(0, 0, 0, 0);
    bool bPortrait = UIDeviceOrientationIsPortrait([[UIDevice currentDevice] orientation]);
    const CRect rectScreen(0, 0, bPortrait ? 320 : 480, bPortrait ? 480 : 320);
    const CRect rect4_3(0, 0, 4, 3);
    const CRect rect16_9(0, 0, 16, 9);
    m_sRenderParam.sizeMovie.width = 480.0f;
    m_sRenderParam.sizeMovie.height = 480.0f;
    const CRect rectMovie(0, 0, m_sRenderParam.sizeMovie.width, m_sRenderParam.sizeMovie.height);
    
    switch (m_sRenderParam.eAspectRatio) 
    {
        case eAspectRadio4_3:
            RectStretchAdapt(&rect4_3, &rectScreen, &rectRet);
            break;
        case eAspectRadio16_9:
            RectStretchAdapt(&rect16_9, &rectScreen, &rectRet);
            break; 
        case eAspectRadioFullScreen:
            AssignRect(rectScreen, rectRet);
            break; 
        case eAspectRadioOriginal:
        default:
            RectStretchAdapt(&rectMovie, &rectScreen, &rectRet);
            break;
    }
    return CGRectMake(rectRet.left, rectRet.top, rectRet.Width(), rectRet.Height());   
}

- (void) updateRenderParam
{
    CGRect rectOnScreen = [self calcOnScreenRect];
    
    m_sRenderParam.arraySquareVertices[0] = rectOnScreen.origin.x;
    m_sRenderParam.arraySquareVertices[1] = PLAYER_FRAME_HEIGHT - rectOnScreen.origin.y;
    m_sRenderParam.arraySquareVertices[2] = rectOnScreen.origin.x;
    m_sRenderParam.arraySquareVertices[3] = PLAYER_FRAME_HEIGHT - rectOnScreen.origin.y - rectOnScreen.size.height;
    m_sRenderParam.arraySquareVertices[4] = rectOnScreen.origin.x + rectOnScreen.size.width;
    m_sRenderParam.arraySquareVertices[5] = PLAYER_FRAME_HEIGHT - rectOnScreen.origin.y - rectOnScreen.size.height;
    m_sRenderParam.arraySquareVertices[6] = rectOnScreen.origin.x + rectOnScreen.size.width;
    m_sRenderParam.arraySquareVertices[7] = PLAYER_FRAME_HEIGHT - rectOnScreen.origin.y;
    
    m_sRenderParam.arraySquareTextureCoords[0] = 0.0f;
    m_sRenderParam.arraySquareTextureCoords[1] = 1.0f;
    m_sRenderParam.arraySquareTextureCoords[2] = 0.0f;
    m_sRenderParam.arraySquareTextureCoords[3] = 0.0f;
    m_sRenderParam.arraySquareTextureCoords[4] = 1.0f;
    m_sRenderParam.arraySquareTextureCoords[5] = 0.0f;
    m_sRenderParam.arraySquareTextureCoords[6] = 1.0f;
    m_sRenderParam.arraySquareTextureCoords[7] = 1.0f;
}

- (void) clearBackground
{
    unsigned long c = 0;
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1,
                 0, GL_RGBA, GL_UNSIGNED_BYTE, &c);
    
    GLfloat arraySquareVertices[] = {0, 480,
        0,  0,
        480, 0,
        480, 480};
    
    GLfloat arraySquareTextureCoords[] = {0.0f, 1.0f,
        0.0f, 0.0f,
        1.0f, 0.0f,
        1.0f, 1.0f};
    
    glVertexPointer(2, GL_FLOAT, 0, arraySquareVertices);
    glTexCoordPointer(2, GL_FLOAT, 0, arraySquareTextureCoords);
    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
}
- (void)handleTimer
{    
    if (bNeedClearBackground)
    {
        [self clearBackground];
        bNeedClearBackground = false;
    }
    [self updateRenderParam];
	glColor4f(1.0f, 1.0f, 1.0f, 0.0f);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	pthread_mutex_lock(&m_mutexFromView);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, PLAYER_FRAME_WIDTH, PLAYER_FRAME_HEIGHT,
				 0, GL_RGBA, GL_UNSIGNED_BYTE, m_pData);
//	{
//		// subtitle;
//		NSData* nsData = [NSData dataWithBytes:m_wstrSubTitle.c_str()
//										length:m_wstrSubTitle.size()*sizeof(wchar_t)];
//		NSString* nsText = [[NSString alloc] initWithData:nsData 
//												 encoding:NSUTF32LittleEndianStringEncoding];
//		//uiLabel.text = nsText;
//		[nsText release];
//	}	
	pthread_mutex_unlock(&m_mutexFromView);
	
	glVertexPointer(2, GL_FLOAT, 0, m_sRenderParam.arraySquareVertices);
	glTexCoordPointer(2, GL_FLOAT, 0, m_sRenderParam.arraySquareTextureCoords);
	glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

	[context presentRenderbuffer:GL_RENDERBUFFER_OES];
}

- (void) setAspectRadio:(EnumAspectRatio)eAspectRatio
{
    m_sRenderParam.eAspectRatio = eAspectRatio;
    [self setNeedEraseBackground];
}

- (void) setNeedEraseBackground
{
    bNeedClearBackground = true;
}

@end

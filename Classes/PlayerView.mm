#import "PlayerView.h"
#import <QuartzCore/QuartzCore.h>
#import <OpenGLES/EAGLDrawable.h>
#import "PlayerViewControllerImp.h"


@implementation PlayerView

@synthesize m_wstrSubTitle;
@synthesize playerViewControllerImp;
@synthesize m_rectOnScreen;

// You must implement this method
+ (Class)layerClass {
    return [CAEAGLLayer class];
}


- (id)initWithFrame:(CGRect)frame {
    pthread_mutex_init(&m_mutexFromView, NULL);
    if (self = [super initWithFrame:frame]) 
	{
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
//        glGenTextures(1, &glTexture2);
//        glBindTexture( GL_TEXTURE_2D, glTexture2);
        // Filters
        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
        
        // Wrap
        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glEnable(GL_TEXTURE_2D);
        glEnableClientState(GL_TEXTURE_COORD_ARRAY);
        glEnableClientState(GL_VERTEX_ARRAY);	
        glDisable(GL_DITHER);
        glDisable(GL_CULL_FACE);
        glDisable(GL_ALPHA_TEST);  
        glDisable(GL_BLEND);  
        glDisable(GL_DEPTH_TEST);  
        glDisable(GL_DITHER);  
        glDisable(GL_FOG);  
        glDisable(GL_LIGHTING);  
        glDisable( GL_SCISSOR_TEST );
        glDisable(GL_STENCIL_TEST); 
        
        m_pDataResized = new unsigned char[(int)(PLAYER_FRAME_WIDTH * PLAYER_FRAME_HEIGHT * 4)];
        m_pDataCorped = new unsigned char[(int)(PLAYER_FRAME_WIDTH * PLAYER_FRAME_HEIGHT * 4)];
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
    delete[] m_pDataCorped;
    delete[] m_pDataResized;
    [super dealloc];
}
extern bool saveBmp(const char* bmpName,unsigned char *imgBuf,int width,int height,int biBitCount);

- (void) crop
{
    int iLineByteFrom = m_sRenderParam.sizeMovie.width * 4;
    int iLineByteOffsetFrom = m_sRenderParam.rectMovieCroped.left * 4;
    int iCropedHeight = m_sRenderParam.rectMovieCroped.Height();
    int iLineByteCroped = m_sRenderParam.rectMovieCroped.Width() * 4;
    for (int iHeight = 0; iHeight < iCropedHeight; iHeight ++)
    {
        unsigned char* pDataFrom = m_pDataResized + (iLineByteFrom * iHeight + iLineByteOffsetFrom);
        unsigned char* pDataTo = m_pDataCorped + (iLineByteCroped * iHeight);
        memcpy(pDataTo, pDataFrom, iLineByteCroped);
    }
}
- (void) calcOnScreenRect
{
    CRect rectRet(0, 0, 0, 0);
    bool bPortrait = UIDeviceOrientationIsPortrait([[UIDevice currentDevice] orientation]);
    const CRect rectScreen(0, 0, bPortrait ? 320 : 480, bPortrait ? 480 : 320);
    const CRect rectScreenMin(0, 0, bPortrait ? 2 : 3, bPortrait ? 3 : 2);
    const CRect rect4_3(0, 0, 4, 3);
    const CRect rect16_9(0, 0, 16, 9);
    const CRect rectMovie(0, 0, m_sRenderParam.sizeMovie.width, m_sRenderParam.sizeMovie.height);
    CRect rectTmp(0, 0, 0, 0);
    
    switch (m_sRenderParam.eAspectRatio) 
    {
        case eAspectRadio4_3:
            RectStretchAdapt(&rect4_3, &rectScreen, &rectRet);
            m_sizeRendered = m_sRenderParam.sizeMovieResized;
            m_pDateRendered = m_pDataResized;
            break;
            
        case eAspectRadio16_9:
            RectStretchAdapt(&rect16_9, &rectScreen, &rectRet);
            m_sizeRendered = m_sRenderParam.sizeMovieResized;
            m_pDateRendered = m_pDataResized;
            break; 
            
        case eAspectRadioFullScreen:
            AssignRect(rectScreen, rectRet);
            RectStretchAdapt(&rectScreenMin, &rectMovie, &m_sRenderParam.rectMovieCroped);
            if ((m_sRenderParam.rectMovieCroped.Width() != rectMovie.Width()) 
                || (m_sRenderParam.rectMovieCroped.Height() != rectMovie.Height()))
            {
                m_sizeRendered = CGSizeMake(m_sRenderParam.rectMovieCroped.Width(), m_sRenderParam.rectMovieCroped.Height());
                m_pDateRendered = m_pDataCorped;
                [self crop];
            }
            else
            {
                m_sizeRendered = m_sRenderParam.sizeMovieResized;
                m_pDateRendered = m_pDataResized;
            }
            break; 
            
        case eAspectRadioOriginal:
        default:
            RectStretchAdapt(&rectMovie, &rectScreen, &rectRet);
            m_sizeRendered = m_sRenderParam.sizeMovieResized;
            m_pDateRendered = m_pDataResized;
            break;
    }
    m_rectOnScreen = CGRectMake(rectRet.left, rectRet.top, rectRet.Width(), rectRet.Height());   
}

- (void) updateRenderParam
{
    [self calcOnScreenRect];
    CGRect rectOnScreen = m_rectOnScreen;
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


- (void) setAspectRadio:(EnumAspectRatio)eAspectRatio
{
    m_sRenderParam.eAspectRatio = eAspectRatio;
}

- (void) setMovieSize:(int)iWidth iHeight:(int)iHeight iWidthResized:(int)iWidthResized iHeightResized:(int)iHeightResized
{
    m_sRenderParam.sizeMovie.width = iWidth;
    m_sRenderParam.sizeMovie.height = iHeight;
    m_sRenderParam.sizeMovieResized.width = iWidthResized;
    m_sRenderParam.sizeMovieResized.height = iHeightResized;
}

- (void) showWithSubTitle:(NSString*)str
{
    if (m_bRoating)
    {
        [self clearBackground];
        [context presentRenderbuffer:GL_RENDERBUFFER_OES];
        return;
    }
    [playerViewControllerImp showWithSubTitle:str];
    //[self clearBackground];
    [self updateRenderParam];
	glColor4f(1.0f, 1.0f, 1.0f, 0.0f);
    
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

        pthread_mutex_lock(&m_mutexFromView);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE, 640, 480,
                     0, GL_LUMINANCE, GL_UNSIGNED_BYTE, m_pDateRendered);
        //	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, m_sizeRendered.width, m_sizeRendered.height,
        //				 0, GL_RGBA, GL_UNSIGNED_BYTE, m_pDateRendered);
        pthread_mutex_unlock(&m_mutexFromView);


	glVertexPointer(2, GL_FLOAT, 0, m_sRenderParam.arraySquareVertices);
	glTexCoordPointer(2, GL_FLOAT, 0, m_sRenderParam.arraySquareTextureCoords);
	glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
    
	[context presentRenderbuffer:GL_RENDERBUFFER_OES];
}

- (void) startRotate
{
    m_bRoating = true;
}
- (void) stopRotate
{
    m_bRoating = false;
}

@end

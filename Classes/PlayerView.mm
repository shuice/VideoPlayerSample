#import "PlayerView.h"
#import <QuartzCore/QuartzCore.h>
#import <OpenGLES/EAGLDrawable.h>
#import "PlayerViewControllerImp.h"
#import "GLSLShader.h"


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
        
        context = [[EAGLContext alloc] initWithAPI:kEAGLRenderingAPIOpenGLES2];        
        if (context == nil)
        {
            context = [[EAGLContext alloc] initWithAPI:kEAGLRenderingAPIOpenGLES1];
        }
        [EAGLContext setCurrentContext:context];
        
        glGenFramebuffers(1, &glFramebuffer);
        glGenRenderbuffers(1, &glRenderbuffer);
        glBindFramebuffer(GL_FRAMEBUFFER, glFramebuffer);
        glBindRenderbuffer(GL_RENDERBUFFER, glRenderbuffer);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER_OES, GL_COLOR_ATTACHMENT0_OES, GL_RENDERBUFFER_OES, glRenderbuffer); 
       
        glEnable(GL_TEXTURE_2D);
		
        glGenTextures(1, &_texture);
		glBindTexture(GL_TEXTURE_2D, _texture);
		glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
#if USE_RGB_TEXTURE
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, TEXTURE_WIDTH, TEXTURE_HEIGHT, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);
#else
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, TEXTURE_WIDTH, TEXTURE_HEIGHT, 0, GL_RGBA, GL_UNSIGNED_SHORT_4_4_4_4, 0);	
        
#endif
        
        glBindTexture(GL_TEXTURE_2D, _texture);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        
        glGenBuffers(2, _bufferObject);

        
#if USE_RGB_TEXTURE
		NSString *fShader = [[NSBundle mainBundle] pathForResource:@"Shader_RGB" ofType:@"fsh"];
#else
		NSString *fShader = [[NSBundle mainBundle] pathForResource:@"Shader" ofType:@"fsh"];
#endif
        NSString *vShader = [[NSBundle mainBundle] pathForResource:@"Shader" ofType:@"vsh"];
        _mainShader = new GLSLShader([vShader fileSystemRepresentation], [fShader fileSystemRepresentation], kShaderDataTypeSourceFile);
        _mainShader->compile();
		_mainShader->beginShader();
		glBindBuffer(GL_ARRAY_BUFFER, _bufferObject[0]);
		_mainShader->setVertexPointer("position", reinterpret_cast<float *> (0), 2);
		glBindBuffer(GL_ARRAY_BUFFER, _bufferObject[1]);
		_mainShader->setVertexPointer("texCoords", reinterpret_cast<float *> (0), 2);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		_mainShader->setTexture("texture", 0);
		_param[0] = _mainShader->parameterAttribute("wp");
		_param[1] = _mainShader->parameterAttribute("wsize");
		_param[2] = _mainShader->vectorAttribute("position");
		_param[3] = _mainShader->vectorAttribute("texCoords");
		_param[4] = _mainShader->parameterAttribute("transformMatrix");
		
		_mainShader->endShader();
        
        glClearColor(0.0, 0.0, 0.0, 1.0);
        
        glBindRenderbuffer(GL_RENDERBUFFER, glRenderbuffer);        
        [context renderbufferStorage:GL_RENDERBUFFER_OES fromDrawable:(CAEAGLLayer*)self.layer];
        glBindFramebuffer(GL_FRAMEBUFFER, glFramebuffer);

        glGetRenderbufferParameteriv(GL_RENDERBUFFER, GL_RENDERBUFFER_WIDTH, &_backingWidth);
        glGetRenderbufferParameteriv(GL_RENDERBUFFER, GL_RENDERBUFFER_HEIGHT, &_backingHeight);
        glViewport(0, 0, _backingWidth,  _backingHeight);
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
    glDeleteTextures(1, &_texture);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glDeleteBuffers(2, _bufferObject);
    if (_mainShader != NULL)
    {
        delete _mainShader;
        _mainShader = NULL;
    }
    [super dealloc];
}

- (void) calcOnScreenRect
{
    if (eAspectRatioFullScreen != m_sRenderParam.eAspectRatio)
    {
        [self setFrame:[[self superview] frame]];
    }
    CGRect rectFrame = [[self superview] frame];
    CRect rectRet(0, 0, 0, 0);
    const CRect rectScreen(0, 0, rectFrame.size.width, rectFrame.size.height);
    const CRect rectScreenMin(0, 0, rectFrame.size.width*10/rectFrame.size.height, 10);
    const CRect rect4_3(0, 0, 4, 3);
    const CRect rect16_9(0, 0, 16, 9);
    const CRect rectMovie(0, 0, m_sRenderParam.sizeMovie.width, m_sRenderParam.sizeMovie.height);
    CRect rectTmp(0, 0, 0, 0);
    m_sRenderParam.rectMovieCroped = CRect(0, 0, 0, 0);
    
    switch (m_sRenderParam.eAspectRatio) 
    {
        case eAspectRatio4_3:
            RectStretchAdapt(&rect4_3, &rectScreen, &rectRet);
            m_sizeRendered = m_sRenderParam.sizeMovieResized;
            break;
            
        case eAspectRatio16_9:
            RectStretchAdapt(&rect16_9, &rectScreen, &rectRet);
            m_sizeRendered = m_sRenderParam.sizeMovieResized;
            break; 
            
        case eAspectRatioFullScreen:
            {
                RectStretchAdapt(&rectMovie, &rectScreen, &rectRet);
                m_sizeRendered = m_sRenderParam.sizeMovieResized;
                float fScale = 1.5f;                
                float fWidthNewFrame = rectScreen.Width() * fScale;
                float fHeightNewFrame = rectScreen.Height() * fScale;
                [self setFrame:CGRectMake(-(fWidthNewFrame - rectScreen.Width()) /2.0f, 
                                          -(fHeightNewFrame - rectScreen.Height()) /2.0f, 
                                          fWidthNewFrame, 
                                          fHeightNewFrame)];
            }
            break; 
            
        case eAspectRatioOriginal:
        default:
            RectStretchAdapt(&rectMovie, &rectScreen, &rectRet);
            m_sizeRendered = m_sRenderParam.sizeMovieResized;
            break;
    }
    m_rectOnScreen = CGRectMake(rectRet.left, rectRet.top, rectRet.Width(), rectRet.Height());   
}

- (void) updateRenderParam
{
    [self calcOnScreenRect];
    CGRect rectOnScreen = m_rectOnScreen;
    
    CGRect rectFrame = [self frame]; 
    //uikit 坐标
    m_sRenderParam.arraySquareVertices[0] = rectOnScreen.origin.x;
    m_sRenderParam.arraySquareVertices[3] = rectFrame.size.height - rectOnScreen.origin.y;
    m_sRenderParam.arraySquareVertices[2] = rectOnScreen.origin.x;
    m_sRenderParam.arraySquareVertices[1] = rectFrame.size.height - rectOnScreen.origin.y - rectOnScreen.size.height;
    m_sRenderParam.arraySquareVertices[4] = rectOnScreen.origin.x + rectOnScreen.size.width;
    m_sRenderParam.arraySquareVertices[5] = rectFrame.size.height - rectOnScreen.origin.y - rectOnScreen.size.height;
    m_sRenderParam.arraySquareVertices[6] = rectOnScreen.origin.x + rectOnScreen.size.width;
    m_sRenderParam.arraySquareVertices[7] = rectFrame.size.height - rectOnScreen.origin.y;
    
    
    m_sRenderParam.arraySquareTextureCoords[0] = 0.0f;
    m_sRenderParam.arraySquareTextureCoords[1] = 1.0f;
    m_sRenderParam.arraySquareTextureCoords[2] = 0.0f;
    m_sRenderParam.arraySquareTextureCoords[3] = 0.0f;
    m_sRenderParam.arraySquareTextureCoords[4] = 1.0f;
    m_sRenderParam.arraySquareTextureCoords[5] = 0.0f;
    m_sRenderParam.arraySquareTextureCoords[6] = 1.0f;
    m_sRenderParam.arraySquareTextureCoords[7] = 1.0f;
    
    // opengles 坐标
    m_sRenderParam.arraySquareVertices[0] /= rectFrame.size.width;
    m_sRenderParam.arraySquareVertices[1] /= rectFrame.size.height;
    m_sRenderParam.arraySquareVertices[2] /= rectFrame.size.width;
    m_sRenderParam.arraySquareVertices[3] /= rectFrame.size.height;
    m_sRenderParam.arraySquareVertices[4] /= rectFrame.size.width;
    m_sRenderParam.arraySquareVertices[5] /= rectFrame.size.height;
    m_sRenderParam.arraySquareVertices[6] /= rectFrame.size.width;
    m_sRenderParam.arraySquareVertices[7] /= rectFrame.size.height;
    
    for (int i = 0; i < 8; i++)
    {
        m_sRenderParam.arraySquareVertices[i] =  (m_sRenderParam.arraySquareVertices[i] - 0.5) * 2;
    } 
	
	glBindBuffer(GL_ARRAY_BUFFER, _bufferObject[0]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 8, m_sRenderParam.arraySquareVertices, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);  
    
    _widthp = (float)m_sizeRendered.width / (float)TEXTURE_WIDTH;
	_heightp = (float)m_sizeRendered.height / (float)TEXTURE_HEIGHT;
    float fLeft = m_sRenderParam.rectMovieCroped.left / (float)TEXTURE_WIDTH;
    float fTop = m_sRenderParam.rectMovieCroped.top / (float)TEXTURE_WIDTH;
	GLfloat squareTexCoords[] = {
		fLeft, fTop+_heightp, 
        fLeft, fTop, 
		fLeft+_widthp, fTop+_heightp,
		fLeft+_widthp, fTop, 
		
	};
    glBindBuffer(GL_ARRAY_BUFFER, _bufferObject[1]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 8, squareTexCoords, GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    

}



- (EnumPlayerStatus) setAspectRadio:(EnumAspectRatio)eAspectRatio
{
    m_sRenderParam.eAspectRatio = eAspectRatio;
    return ePlayerStatusOk;
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
    [playerViewControllerImp showWithSubTitle:str];
    [self updateRenderParam];

	[EAGLContext setCurrentContext:context];
	glBindTexture(GL_TEXTURE_2D, _texture);
    
    int w = m_sRenderParam.sizeMovieResized.width;
    int h = m_sRenderParam.sizeMovieResized.height;
#if USE_RGB_TEXTURE
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, w, h, GL_RGBA, GL_UNSIGNED_BYTE, m_pData[0]);
#else
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, w/2, h, GL_RGBA, GL_UNSIGNED_SHORT_4_4_4_4, m_pData[0]);
    glTexSubImage2D(GL_TEXTURE_2D, 0, w / 2, 0, w / 4, h / 2, GL_RGBA, GL_UNSIGNED_SHORT_4_4_4_4, m_pData[1]);
    glTexSubImage2D(GL_TEXTURE_2D, 0, w / 2 + w / 4, 0, w / 4, h / 2, GL_RGBA, GL_UNSIGNED_SHORT_4_4_4_4, m_pData[2]);
#endif
    
    glBindFramebuffer(GL_FRAMEBUFFER, glFramebuffer);
    glClear(GL_COLOR_BUFFER_BIT);
	_mainShader->beginShader();
    _mainShader->setTexture("texture", 0);
	_mainShader->setParameter(_param[0], (float)_widthp);
	_mainShader->setParameter(_param[1], (float)w);
	glBindBuffer(GL_ARRAY_BUFFER, _bufferObject[0]);
	glBindBuffer(GL_ARRAY_BUFFER, _bufferObject[1]);
    memset(_matrix, 0, sizeof(_matrix));
    _matrix[0] = _matrix[5] = _matrix[10] = _matrix[15] = 1.f;
	_mainShader->setParameterMatrix44(_param[4], _matrix);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	_mainShader->endShader();
    glBindRenderbuffer(GL_RENDERBUFFER, glRenderbuffer);
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

- (void) playFinish:(id)object
{
    [playerViewControllerImp playFinish];
}

@end

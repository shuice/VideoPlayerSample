#import "PlayerView.h"
#import <QuartzCore/QuartzCore.h>
#import <OpenGLES/EAGLDrawable.h>
#import "PlayerViewControllerImp.h"
#import "GLSLShader.h"


static GLSLShader *_mainShader;


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
        [EAGLContext setCurrentContext:context];
        
        glGenFramebuffers(1, &glFramebuffer);
        glGenRenderbuffers(1, &glRenderbuffer);
        glBindFramebuffer(GL_FRAMEBUFFER, glFramebuffer);
        glBindRenderbuffer(GL_RENDERBUFFER, glRenderbuffer);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER_OES, GL_COLOR_ATTACHMENT0_OES, GL_RENDERBUFFER_OES, glRenderbuffer); 
        
        
       
        glEnable(GL_TEXTURE_2D);
        glShadeModel(GL_FLAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glGenTextures(1, _textures);
		glBindTexture(GL_TEXTURE_2D, _textures[0]);
		glPixelStorei(GL_UNPACK_ALIGNMENT, 1);	
        _widthTexture = 512.0f;
        _heightTexture = 512.0f;
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, _widthTexture, _heightTexture, 0, GL_RGBA, GL_UNSIGNED_SHORT_4_4_4_4, 0);	
        
        _texture = _textures[0];
        glBindTexture(GL_TEXTURE_2D, _texture);
        glGenBuffers(2, _bufferObject);

        
        
        
        NSString *vShader = [[NSBundle mainBundle] pathForResource:@"Shader" ofType:@"vsh"];
		NSString *fShader = [[NSBundle mainBundle] pathForResource:@"Shader" ofType:@"fsh"];
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
        glViewport(0, 0, PLAYER_FRAME_WIDTH,  PLAYER_FRAME_HEIGHT);
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
    
    //uikit 坐标
    m_sRenderParam.arraySquareVertices[0] = rectOnScreen.origin.x;
    m_sRenderParam.arraySquareVertices[3] = PLAYER_FRAME_HEIGHT - rectOnScreen.origin.y;
    m_sRenderParam.arraySquareVertices[2] = rectOnScreen.origin.x;
    m_sRenderParam.arraySquareVertices[1] = PLAYER_FRAME_HEIGHT - rectOnScreen.origin.y - rectOnScreen.size.height;
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
    
    // opengles 坐标
    m_sRenderParam.arraySquareVertices[0] /= PLAYER_FRAME_WIDTH;
    m_sRenderParam.arraySquareVertices[1] /= PLAYER_FRAME_HEIGHT;
    m_sRenderParam.arraySquareVertices[2] /= PLAYER_FRAME_WIDTH;
    m_sRenderParam.arraySquareVertices[3] /= PLAYER_FRAME_HEIGHT;
    m_sRenderParam.arraySquareVertices[4] /= PLAYER_FRAME_WIDTH;
    m_sRenderParam.arraySquareVertices[5] /= PLAYER_FRAME_HEIGHT;
    m_sRenderParam.arraySquareVertices[6] /= PLAYER_FRAME_WIDTH;
    m_sRenderParam.arraySquareVertices[7] /= PLAYER_FRAME_HEIGHT;
    
    for (int i = 0; i < 8; i++)
    {
        m_sRenderParam.arraySquareVertices[i] =  (m_sRenderParam.arraySquareVertices[i] - 0.5) * 2;
    } 
	
	glBindBuffer(GL_ARRAY_BUFFER, _bufferObject[0]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 8, m_sRenderParam.arraySquareVertices, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);  
    
    _widthp = (float)m_sizeRendered.width / (float)_widthTexture / 2;
	_heightp = (float)m_sizeRendered.height / (float)_heightTexture;
	GLfloat squareTexCoords[] = {
		0, _heightp, 
        0, 0, 
		_widthp, _heightp,
		_widthp, 0, 
		
	};
    glBindBuffer(GL_ARRAY_BUFFER, _bufferObject[1]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 8, squareTexCoords, GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    

}

- (void) clearBackground
{
    return;
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
    //[playerViewControllerImp showWithSubTitle:str];
    //[self clearBackground];
    [self updateRenderParam];
    _mainShader->beginShader();
	glBindTexture(GL_TEXTURE_2D, _texture);
	_mainShader->setTexture("texture", 0);
	
	_mainShader->endShader();
	[EAGLContext setCurrentContext:context];
    
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, _texture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    
    
    int w = m_sRenderParam.sizeMovieResized.width;
    int h = m_sRenderParam.sizeMovieResized.height;
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, w/2, h, GL_RGBA, GL_UNSIGNED_SHORT_4_4_4_4, m_pData[0]);
    glTexSubImage2D(GL_TEXTURE_2D, 0, w / 2, 0, w / 4, h / 2, GL_RGBA, GL_UNSIGNED_SHORT_4_4_4_4, m_pData[1]);
    glTexSubImage2D(GL_TEXTURE_2D, 0, w / 2 + w / 4, 0, w / 4, h / 2, GL_RGBA, GL_UNSIGNED_SHORT_4_4_4_4, m_pData[2]);
    
    glBindFramebuffer(GL_FRAMEBUFFER, glFramebuffer);
    glClear(GL_COLOR_BUFFER_BIT);
	_mainShader->beginShader();
	_mainShader->setParameter(_param[0], (float)_widthp);
	_mainShader->setParameter(_param[1], (float)w);
	glBindBuffer(GL_ARRAY_BUFFER, _bufferObject[0]);
//	_mainShader->setVertexPointer(_param[2], reinterpret_cast<float *> (0), 2);
	glBindBuffer(GL_ARRAY_BUFFER, _bufferObject[1]);
//	_mainShader->setVertexPointer(_param[3], reinterpret_cast<float *> (0), 2);
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

@end

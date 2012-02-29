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

#import <UIKit/UIKit.h>
#import <OpenGLES/EAGL.h>
#import <OpenGLES/ES1/gl.h>
#import <OpenGLES/ES1/glext.h>
#import "Common.h"
#import "PlayerViewController.h"


#define PLAYER_DEG_TO_RAD				0.017453f
#define PLAYER_FRAME_WIDTH 480.0f
#define PLAYER_FRAME_HEIGHT 480.0f

typedef struct SRenderParam
{
    CGSize sizeMovie;
    CGSize sizeMovieResized;
    CRect  rectMovieCroped;
    EnumAspectRatio eAspectRatio;
    GLfloat arraySquareVertices[8];
    GLfloat arraySquareTextureCoords[8];
}SRenderParam;

@class PlayerViewControllerImp;
@interface PlayerView : UIView {
@public   
	pthread_mutex_t m_mutexFromView;
	wstring m_wstrSubTitle;
    unsigned char* m_pDataResized;
    unsigned char* m_pDataCorped;
    unsigned char* m_pData[3];
@private
    EAGLContext *context;
    GLuint glRenderbuffer;
    GLuint glFramebuffer;
	GLuint glTexture;
    GLuint glTexture2;
    
    SRenderParam m_sRenderParam;
    unsigned char* m_pDateRendered;
    CGSize m_sizeRendered;
    
    PlayerViewControllerImp* playerViewControllerImp;
    bool m_bRoating;
    CGRect m_rectOnScreen;
    GLuint _bufferObject[2];
    GLuint _textures[4];
    GLuint _texture;
    GLuint _param[5];
    float _matrix[16];
}


@property (nonatomic, assign) wstring m_wstrSubTitle;
@property (nonatomic, assign) PlayerViewControllerImp* playerViewControllerImp;
@property (nonatomic, readonly) CGRect m_rectOnScreen;

- (void) setAspectRadio:(EnumAspectRatio)eAspectRatio;
- (void) setMovieSize:(int)iWidth iHeight:(int)iHeight iWidthResized:(int)iWidthResized iHeightResized:(int)iHeightResized;
- (void) startRotate;
- (void) stopRotate;
- (void) calcOnScreenRect;

@end

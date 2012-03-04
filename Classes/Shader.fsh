//
//  Shader.fsh
//  SharePlayer
//
//  Created by mbp on 9/10/09.
//  Copyright occlusion 2009. All rights reserved.
//
// last modified by xiaoyi liao on 3/4/12
precision mediump float;
varying vec2 texCoordShare;
uniform sampler2D texture;

uniform float wp;
uniform float wsize;
void main()
{
	vec2 texCoordShare2 = texCoordShare;
    texCoordShare2.x /= 2.0;
	vec4 yv = texture2D(texture, texCoordShare2);

	texCoordShare2.y/=2.0;
	float now = texCoordShare.x;
	float nowPer = now/wp;
	float per = nowPer*wp/4.0;
	texCoordShare2.x = wp/2.0 + per;
    vec4 uv = texture2D(texture, texCoordShare2);
    
    texCoordShare2.x = wp/2.0+wp/4.0 + per;
    vec4 vv  = texture2D(texture, texCoordShare2);

    // for y
	float flg = floor(mod(nowPer*wsize, 2.0));
    float flguv = floor(mod(nowPer*wsize/2.0, 2.0));
    
    // for uv
	float invFlg = 1.0-flg;
    float invFlguv = 1.0 - flguv;
    
	float y = (yv.r*240.0+yv.g*15.0)/255.0*flg + invFlg*(yv.b*240.0+yv.a*15.0)/255.0;
	float u = (uv.r*240.0+uv.g*15.0)/255.0*flguv + invFlguv*(uv.b*240.0+uv.a*15.0)/255.0;
	float v = (vv.r*240.0+vv.g*15.0)/255.0*flguv + invFlguv*(vv.b*240.0+vv.a*15.0)/255.0;
            
	vec4 tv = vec4(y, u, v, 1.0);
	mat4 tm = mat4(	1.0,  0.0, 1.402,
                   -0.701, 1.0, -0.344,
                   -0.714, 0.529, 1.0,
                   1.772, 0.0,-0.886,
                   0.0, 0.0, 0.0, 1.0);
	vec4 c = tv * tm;
	gl_FragColor = vec4(c.r, c.g, c.b, 1.0);
}


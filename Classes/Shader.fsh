//
//  Shader.fsh
//  SharePlayer
//
//  Created by mbp on 9/10/09.
//  Copyright occlusion 2009. All rights reserved.
//
precision mediump float;
varying vec2 texCoordShare;
uniform sampler2D texture;

uniform float wp;
uniform float wsize;
void main()
{
	vec2 texCoordShare2 = texCoordShare;
	vec4 yv = texture2D(texture, texCoordShare2);
	texCoordShare2.y/=2.0;
	float now = texCoordShare2.x;
	
	float nowPer = now/wp;
	float per = nowPer*wp/2.0;
	texCoordShare2.x = wp + per;
	vec4 uv = texture2D(texture, texCoordShare2);
	texCoordShare2.x = wp+wp/2.0 + per;
	vec4 vv = texture2D(texture, texCoordShare2);
    
	float flg = floor(mod(nowPer*wsize, 2.0));
	float invFlg = 1.0-flg;
	float y = (yv.r*240.0+yv.g*15.0)/255.0*flg + invFlg*(yv.b*240.0+yv.a*15.0)/255.0;
	float u = (uv.r*240.0+uv.g*15.0)/255.0*flg + invFlg*(uv.b*240.0+uv.a*15.0)/255.0;
	float v = (vv.r*240.0+vv.g*15.0)/255.0*flg + invFlg*(vv.b*240.0+vv.a*15.0)/255.0;
	
	
	vec4 tv = vec4(y, u, v, 1.0);
	mat4 tm = mat4(	1.0,  0.0, 1.402,
                   -0.701, 1.0, -0.344,
                   -0.714, 0.529, 1.0,
                   1.772, 0.0,-0.886,0.0, 0.0, 0.0, 1.0);
	vec4 c = tv * tm;
	
	gl_FragColor = vec4(c.r, c.g, c.b, 1.0);
    //gl_FragColor = vec4(0.4,0.4,0.8,1.0);
}


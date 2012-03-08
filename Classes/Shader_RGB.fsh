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
	gl_FragColor = texture2D(texture, texCoordShare);
}


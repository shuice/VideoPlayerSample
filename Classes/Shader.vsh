//
//  Shader.vsh
//  SharePlayer
//
//  Created by mbp on 9/10/09.
//  Copyright occlusion 2009. All rights reserved.
//
// last modified by xiaoyi liao on 3/7/12

attribute vec4 position;
attribute vec2 texCoords;

uniform mat4 transformMatrix;
varying vec2 texCoordShare;

void main()
{
	gl_Position = position * transformMatrix;
	texCoordShare = texCoords;
}

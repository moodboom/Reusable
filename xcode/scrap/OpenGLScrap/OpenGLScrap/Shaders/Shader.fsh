//
//  Shader.fsh
//  OpenGLScrap
//
//  Created by Michael Behrns-Miller on 7/11/14.
//  Copyright (c) 2014 A better Software, Inc. All rights reserved.
//

varying lowp vec4 colorVarying;

void main()
{
    gl_FragColor = colorVarying;
}

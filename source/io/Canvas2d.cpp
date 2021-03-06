/**
 
 Copyright (c) 2013 National Instruments Corp.
 
 Permission is hereby granted, free of charge, to any person obtaining a copy
 of this software and associated documentation files (the "Software"), to deal
 in the Software without restriction, including without limitation the rights
 to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 copies of the Software, and to permit persons to whom the Software is
 furnished to do so, subject to the following conditions:
 
 The above copyright notice and this permission notice shall be included in
 all copies or substantial portions of the Software.
 
 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 THE SOFTWARE.
 
 SDG
 */

#include "TypeDefiner.h"
#include "ExecutionContext.h"
#include "StringUtilities.h"
#include "TDCodecVia.h"

#if kVireoOS_emscripten
    #include "Emscripten.h"
    #include <SDL/SDL.h>
#endif

using namespace Vireo;

typedef Int32 Context2D;

//------------------------------------------------------------
VIREO_FUNCTION_SIGNATURE0(SDLTest)
{
    
#if kVireoOS_emscripten
    SDL_Init(SDL_INIT_VIDEO);
    SDL_Surface *screen = SDL_SetVideoMode(256, 256, 32, SDL_SWSURFACE);
    
    if (SDL_MUSTLOCK(screen)) SDL_LockSurface(screen);
    for (int i = 0; i < 256; i++) {
        for (int j = 0; j < 256; j++) {
            // alpha component is actually ignored, since this is to the screen
            *((Uint32*)screen->pixels + i * 256 + j) = SDL_MapRGBA(screen->format, i, j, 255-i, (i+j) % 255);
        }
    }
    if (SDL_MUSTLOCK(screen)) SDL_UnlockSurface(screen);
    SDL_Flip(screen);
    
    printf("you should see a smoothly-colored square - no sharp lines but the square borders!\n");
    printf("and here is some text that should be HTML-friendly: amp: |&| double-quote: |\"| quote: |'| less-than, greater-than, html-like tags: |<cheez></cheez>|\nanother line.\n");
    
    SDL_Quit();
#endif

    return _NextInstruction();
}
//------------------------------------------------------------
VIREO_FUNCTION_SIGNATURE1(PrintX, Int32)
{
#if kVireoOS_emscripten
    Int32 i = _Param(0);
    EM_ASM( {
        alert(i);
    });
#endif
    return _NextInstruction();
}
//------------------------------------------------------------
VIREO_FUNCTION_SIGNATURE3(MoveTo, Context2D, const Int32, Int32)
{

#if kVireoOS_emscripten
    EM_ASM_( {
        ctx.MoveTo($0,$1);
    }, _Param(0), _Param(1));
#endif
    return _NextInstruction();
}
//------------------------------------------------------------
VIREO_FUNCTION_SIGNATURE3(LineTo, Context2D, Int32, Int32)
{

#if kVireoOS_emscripten
    EM_ASM_( {
        ctx.LineTo($0,$1);
    }, _Param(0), _Param(1));
#endif
    return _NextInstruction();
}
//------------------------------------------------------------
VIREO_FUNCTION_SIGNATURE1(Stroke, Context2D)
{
#if kVireoOS_emscripten
    EM_ASM({
        var theCanvas = document.getElementById('theCanvas');
        alert('canvas?');
        var ctx2 = theCanvas.getContext('2d');
        alert('context?');
        ctx2.moveTo(0,0);
        ctx2.lineTo(200,100);
        ctx2.Stroke();
        alert('drawn?');
    });
#endif
    return _NextInstruction();
}

//------------------------------------------------------------
DEFINE_VIREO_BEGIN(LabVIEW_Canvas2D)

DEFINE_VIREO_TYPE(Context2D, ".Int32")

// Primitives
DEFINE_VIREO_FUNCTION(SDLTest, "p()");
DEFINE_VIREO_FUNCTION(MoveTo, "p(i(.Context2D)i(.Double)i(.Double))");
DEFINE_VIREO_FUNCTION(LineTo, "p(i(.Context2D)i(.Double)i(.Double))");
DEFINE_VIREO_FUNCTION(Stroke, "p(i(.Context2D))");
DEFINE_VIREO_FUNCTION(PrintX, "p(i(.Int32))");

DEFINE_VIREO_END()

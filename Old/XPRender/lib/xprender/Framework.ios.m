#import "Framework.h"
#import "Api.gl.h"

#import <UIKit/UIKit.h>
#import <QuartzCore/QuartzCore.h>

//------------------------------------------------------------------------------
// EAGLView
@interface EAGLView : UIView
{
@private
    BOOL animating;
    BOOL displayLinkSupported;
    NSInteger animationFrameInterval;
    // Use of the CADisplayLink class is the preferred method for controlling your animation timing.
    // CADisplayLink will link to the main display and fire every vsync when added to a given run-loop.
    // The NSTimer class is used only as fallback when running on a pre 3.1 device where CADisplayLink
    // isn't available.
    id displayLink;
    NSTimer *animationTimer;
	EAGLContext *context;
	CFTimeInterval lastTime;
}

@property (readonly, nonatomic, getter=isAnimating) BOOL animating;
@property (nonatomic) NSInteger animationFrameInterval;

- (void)startAnimation;
- (void)stopAnimation;
- (void)drawView:(id)sender;

@end


@implementation EAGLView

@synthesize animating;
@dynamic animationFrameInterval;

// You must implement this method
+ (Class)layerClass
{
    return [CAEAGLLayer class];
}

//The EAGL view is stored in the nib file. When it's unarchived it's sent -initWithCoder:
- (id)initWithCoder:(NSCoder*)coder
{
    if ((self = [super initWithCoder:coder]))
    {
        // Get the layer
        CAEAGLLayer *eaglLayer = (CAEAGLLayer *)self.layer;

        eaglLayer.opaque = TRUE;
        eaglLayer.drawableProperties =
		[NSDictionary dictionaryWithObjectsAndKeys: [NSNumber numberWithBool:FALSE],
			kEAGLDrawablePropertyRetainedBacking,
			kEAGLColorFormatRGBA8,
			kEAGLDrawablePropertyColorFormat,
			nil];

		context = [[EAGLContext alloc] initWithAPI:kEAGLRenderingAPIOpenGLES2];

        if (!context || ![EAGLContext setCurrentContext:context])
        {
            [self release];
            return nil;
        }

		// default FBO
		glGenFramebuffers(1, &xprAPI.defFBOName);
		glBindFramebuffer(GL_FRAMEBUFFER, xprAPI.defFBOName);

		// default Color buffer
        glGenRenderbuffers(1, &xprAPI.defColorBufName);
        glBindRenderbuffer(GL_RENDERBUFFER, xprAPI.defColorBufName);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, xprAPI.defColorBufName);

		// default Color buffer
        glGenRenderbuffers(1, &xprAPI.defDepthBufName);
        glBindRenderbuffer(GL_RENDERBUFFER, xprAPI.defDepthBufName);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, xprAPI.defDepthBufName);

        animating = FALSE;
        displayLinkSupported = FALSE;
        animationFrameInterval = 1;
        displayLink = nil;
        animationTimer = nil;

        // A system version of 3.1 or greater is required to use CADisplayLink. The NSTimer
        // class is used as fallback when it isn't available.
        NSString *reqSysVer = @"3.1";
        NSString *currSysVer = [[UIDevice currentDevice] systemVersion];
        if ([currSysVer compare:reqSysVer options:NSNumericSearch] != NSOrderedAscending)
            displayLinkSupported = TRUE;
    }

    return self;
}

- (void)drawView:(id)sender
{

	CFTimeInterval currTime = CFAbsoluteTimeGetCurrent();
	CFTimeInterval deltaTime = currTime - lastTime;
	lastTime = currTime;

	xprAppUpdate((unsigned int)(deltaTime * 1000));
	xprAppRender();

	glBindRenderbuffer(GL_RENDERBUFFER, xprAPI.defColorBufName);
    [context presentRenderbuffer:GL_RENDERBUFFER];

}

- (void)layoutSubviews
{
    CAEAGLLayer* glLayer = (CAEAGLLayer*)self.layer;

	// color buffer storage
	glBindRenderbuffer(GL_RENDERBUFFER, xprAPI.defColorBufName);
    [context renderbufferStorage:GL_RENDERBUFFER fromDrawable:glLayer];
    glGetRenderbufferParameteriv(GL_RENDERBUFFER, GL_RENDERBUFFER_WIDTH, &xprAppContext.xres);
    glGetRenderbufferParameteriv(GL_RENDERBUFFER, GL_RENDERBUFFER_HEIGHT, &xprAppContext.yres);

	// depth buffer storage
	glBindRenderbuffer(GL_RENDERBUFFER, xprAPI.defDepthBufName);
	glRenderbufferStorage(GL_RENDERBUFFER,GL_DEPTH_COMPONENT16, xprAppContext.xres, xprAppContext.yres);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
    {
        NSLog(@"Failed to make complete framebuffer object %x", glCheckFramebufferStatus(GL_FRAMEBUFFER));
    }

	xprAppInitialize();

	lastTime = CFAbsoluteTimeGetCurrent();

    //[self drawView:nil];
}

- (NSInteger)animationFrameInterval
{
    return animationFrameInterval;
}

- (void)setAnimationFrameInterval:(NSInteger)frameInterval
{
    // Frame interval defines how many display frames must pass between each time the
    // display link fires. The display link will only fire 30 times a second when the
    // frame internal is two on a display that refreshes 60 times a second. The default
    // frame interval setting of one will fire 60 times a second when the display refreshes
    // at 60 times a second. A frame interval setting of less than one results in undefined
    // behavior.
    if (frameInterval >= 1)
    {
        animationFrameInterval = frameInterval;

        if (animating)
        {
            [self stopAnimation];
            [self startAnimation];
        }
    }
}

- (void)startAnimation
{
    if (!animating)
    {
        if (displayLinkSupported)
        {
            // CADisplayLink is API new to iPhone SDK 3.1. Compiling against earlier versions will result in a warning, but can be dismissed
            // if the system version runtime check for CADisplayLink exists in -initWithCoder:. The runtime check ensures this code will
            // not be called in system versions earlier than 3.1.

            displayLink = [NSClassFromString(@"CADisplayLink") displayLinkWithTarget:self selector:@selector(drawView:)];
            [displayLink setFrameInterval:animationFrameInterval];
            [displayLink addToRunLoop:[NSRunLoop currentRunLoop] forMode:NSDefaultRunLoopMode];
        }
        else
            animationTimer = [NSTimer scheduledTimerWithTimeInterval:(NSTimeInterval)((1.0 / 60.0) * animationFrameInterval) target:self selector:@selector(drawView:) userInfo:nil repeats:TRUE];

        animating = TRUE;
    }
}

- (void)stopAnimation
{
    if (animating)
    {
        if (displayLinkSupported)
        {
            [displayLink invalidate];
            displayLink = nil;
        }
        else
        {
            [animationTimer invalidate];
            animationTimer = nil;
        }

        animating = FALSE;
    }
}

- (void)dealloc
{
    //[renderer release];

    [super dealloc];
}

@end



//------------------------------------------------------------------------------
// XprAppDelegate
@class EAGLView;
@interface XprAppDelegate : NSObject <UIApplicationDelegate> {
    UIWindow *window;
	EAGLView *glView;
}

@property (nonatomic, retain) IBOutlet UIWindow *window;
@property (nonatomic, retain) IBOutlet EAGLView *glView;

@end


@implementation XprAppDelegate

@synthesize window;
@synthesize glView;

- (BOOL)application:(UIApplication *)application didFinishLaunchingWithOptions:(NSDictionary *)launchOptions
{
	xprDbgStr("xpr didFinishLaunchingWithOptions\n");
	[glView startAnimation];
    return YES;
}

- (void)applicationWillResignActive:(UIApplication *)application
{
	xprDbgStr("xpr applicationWillResignActive\n");
    [glView stopAnimation];
}

- (void)applicationDidBecomeActive:(UIApplication *)application
{
	xprDbgStr("xpr applicationDidBecomeActive\n");
    [glView startAnimation];
}

- (void)applicationWillTerminate:(UIApplication *)application
{
	xprDbgStr("xpr applicationWillTerminate\n");
	xprAppFinalize();
    [glView stopAnimation];
}

- (void)dealloc
{
    [window release];
    [glView release];

    [super dealloc];
}

@end

//------------------------------------------------------------------------------
// main()
XprAPI xprAPI = {0};

XprAppContext xprAppContext = {
	"xprApp",
	"gles",
	2, 0,
	XprFalse,
	XprFalse,
	480,
	320,
};


int main(int argc, char *argv[]) {

	xprAppConfig();

	NSAutoreleasePool * pool = [[NSAutoreleasePool alloc] init];
    int retVal = UIApplicationMain(argc, argv, nil, nil);
    [pool release];
    return retVal;
}

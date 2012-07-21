
#ifdef __APPLE__

#include "Platform.h"
#include "IPlatformEventHandler.h"
#include "Engine.h"
#include <stdlib.h>

#include <QuartzCore/QuartzCore.h>

#import <UIKit/UIKit.h>
#import <GLKit/GLKit.h>

//---------------------------------------------------------------
// ViewController interface
//---------------------------------------------------------------

@interface ViewController : GLKViewController

    @property (strong, nonatomic) EAGLContext *context;

@end

//---------------------------------------------------------------
// AppDelegate interface
//---------------------------------------------------------------

@interface AppDelegate : UIResponder <UIApplicationDelegate>

    @property (strong, nonatomic) UIWindow *window;
    @property (strong, nonatomic) ViewController *viewController;

@end


//---------------------------------------------------------------
// Global vars
//---------------------------------------------------------------
static ViewController * _controller;
static IPlatformEventHandler * _handler;
static IPlatformEventHandler _dummyHandler;
static Engine * _engine;

//---------------------------------------------------------------
// ViewController implementation
//---------------------------------------------------------------

@implementation ViewController

@synthesize context = _context;

- (void)dealloc
{
    [_context release];
    [super dealloc];
}

- (void)viewDidLoad
{
    [super viewDidLoad];

    self.context = [[[EAGLContext alloc] initWithAPI:kEAGLRenderingAPIOpenGLES2] autorelease];

    if (!self.context) {
        NSLog(@"Failed to create ES context");
    }

    GLKView *view = (GLKView *)self.view;
    view.context = self.context;
    view.drawableStencilFormat = GLKViewDrawableStencilFormat8;
    view.contentScaleFactor = [[UIScreen mainScreen] scale];

    [EAGLContext setCurrentContext:self.context];
    _engine = new Engine;
    Platform::setHandler(_engine);
    _handler->onLoaded();
}

- (void)viewDidUnload
{
    [super viewDidUnload];

    if ([EAGLContext currentContext] == self.context) {
        [EAGLContext setCurrentContext:nil];
    }
    self.context = nil;
}

- (void)didReceiveMemoryWarning
{
    [super didReceiveMemoryWarning];
}

- (BOOL)shouldAutorotateToInterfaceOrientation:(UIInterfaceOrientation)interfaceOrientation
{
    if ([[UIDevice currentDevice] userInterfaceIdiom] == UIUserInterfaceIdiomPhone) {
        return (interfaceOrientation != UIInterfaceOrientationPortraitUpsideDown);
    } else {
        return YES;
    }
    return NO;
}

- (void)update
{

}

- (void)glkView:(GLKView *)view drawInRect:(CGRect)rect
{
    _handler->onUpdate();
}

@end

//---------------------------------------------------------------
// AppDelegate implementation
//---------------------------------------------------------------

@implementation AppDelegate

@synthesize window = _window;
@synthesize viewController = _viewController;

- (void)dealloc
{
    [_window release];
    [_viewController release];
    [super dealloc];
}

- (BOOL)application:(UIApplication *)application didFinishLaunchingWithOptions:(NSDictionary *)launchOptions
{
    self.window = [[[UIWindow alloc] initWithFrame:[[UIScreen mainScreen] bounds]] autorelease];
    // Override point for customization after application launch.
    self.viewController = [[ViewController alloc] init];
    _controller = self.viewController;
    self.window.rootViewController = self.viewController;
    [self.window makeKeyAndVisible];
    return YES;
}

- (void)applicationWillResignActive:(UIApplication *)application
{
    // Sent when the application is about to move from active to inactive state. This can occur for certain types of temporary interruptions (such as an incoming phone call or SMS message) or when the user quits the application and it begins the transition to the background state.
    // Use this method to pause ongoing tasks, disable timers, and throttle down OpenGL ES frame rates. Games should use this method to pause the game.
}

- (void)applicationDidEnterBackground:(UIApplication *)application
{
    // Use this method to release shared resources, save user data, invalidate timers, and store enough application state information to restore your application to its current state in case it is terminated later.
    // If your application supports background execution, this method is called instead of applicationWillTerminate: when the user quits.
}

- (void)applicationWillEnterForeground:(UIApplication *)application
{
    // Called as part of the transition from the background to the inactive state; here you can undo many of the changes made on entering the background.
}

- (void)applicationDidBecomeActive:(UIApplication *)application
{
    // Restart any tasks that were paused (or not yet started) while the application was inactive. If the application was previously in the background, optionally refresh the user interface.
}

- (void)applicationWillTerminate:(UIApplication *)application
{
    _handler->onUnload();
    // Called when the application is about to terminate. Save data if appropriate. See also applicationDidEnterBackground:.
}

- (void)touchesBegan:(NSSet*)touches withEvent:(UIEvent*)event 
{
    for(UITouch* touch in touches) 
    {
        CGPoint point = [touch locationInView:self.viewController.view];
        _handler->onTouchEvent(IPlatformEventHandler::TOUCH_DOWN, point.x, point.y, [touch hash]);
    }
}

- (void)touchesEnded:(NSSet*)touches withEvent:(UIEvent*)event 
{
    for(UITouch* touch in touches) 
    {
        CGPoint point = [touch locationInView:self.viewController.view];
        _handler->onTouchEvent(IPlatformEventHandler::TOUCH_UP, point.x, point.y, [touch hash]);
    }
}

- (void)touchesCancelled:(NSSet*)touches withEvent:(UIEvent*)event 
{
    [self touchesEnded:touches withEvent:event];
}

- (void)touchesMoved:(NSSet*)touches withEvent:(UIEvent*)event 
{
    for(UITouch* touch in touches) 
    {
        CGPoint point = [touch locationInView:self.viewController.view];
        _handler->onTouchEvent(IPlatformEventHandler::TOUCH_MOVE, point.x, point.y, [touch hash]);
    }
}

@end

//---------------------------------------------------------------
// Platform Implementation
//---------------------------------------------------------------

void Platform::setHandler(IPlatformEventHandler * handler)
{
    _handler = handler;
}

double Platform::getTime()
{
    return CACurrentMediaTime();
}

void Platform::readFile(const std::string& path, std::string * out)
{
    NSString * str = [NSString stringWithUTF8String:path.c_str()];
    NSString * name = [[str lastPathComponent] stringByDeletingPathExtension];
    NSString * extension = [str pathExtension];
    NSString * filePath = [[NSBundle mainBundle] pathForResource:name
                                                ofType:extension];
    NSData *data = [NSData dataWithContentsOfFile:filePath];
    out->assign((const char *)[data bytes], [data length]);
}

int Platform::width()
{
    return _controller.view.bounds.size.width * [[UIScreen mainScreen] scale];
}

int Platform::height()
{
    return _controller.view.bounds.size.height * [[UIScreen mainScreen] scale];
}

//---------------------------------------------------------------
// main
//---------------------------------------------------------------

int main(int argc, char *argv[])
{
    _handler = &_dummyHandler;
    @autoreleasepool {
        return UIApplicationMain(argc, argv, nil, NSStringFromClass([AppDelegate class]));
    }
}


#endif

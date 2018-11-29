#include "dslr_capture.h"

#import <Cocoa/Cocoa.h>

/*
#if defined __cplusplus
class Foo;    // forward class declaration
#else
typedef struct Foo Foo;   // forward struct declaration
#endif
*/

@interface MyOCClass : NSObject
{
@private
    Foo* cppObject;
} 
@end

int main() {
    //[[NSApplication sharedApplication] run];
    //start a pool
    NSAutoreleasePool * pool = [[NSAutoreleasePool alloc] init];

    MyOCClass *obj = [[MyOCClass alloc] init];
 
    NSDate *now = [[NSDate alloc] init];
    NSTimer *timer = [[NSTimer alloc] initWithFireDate:now
                      interval:.01
                      target:obj
                      selector:@selector(this_main:)
                      userInfo:nil
                      repeats:YES];

    NSRunLoop *runLoop = [NSRunLoop currentRunLoop];
    [runLoop addTimer:timer forMode:NSDefaultRunLoopMode];
    [runLoop run];

    [pool release];
    NSLog(@"Finished Everything, now closing");
    return 0;
}

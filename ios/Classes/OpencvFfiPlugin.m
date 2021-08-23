#import "OpencvFfiPlugin.h"
#if __has_include(<opencv_ffi/opencv_ffi-Swift.h>)
#import <opencv_ffi/opencv_ffi-Swift.h>
#else
// Support project import fallback if the generated compatibility header
// is not copied when this plugin is created as a library.
// https://forums.swift.org/t/swift-static-libraries-dont-copy-generated-objective-c-header/19816
#import "opencv_ffi-Swift.h"
#endif

@implementation OpencvFfiPlugin
+ (void)registerWithRegistrar:(NSObject<FlutterPluginRegistrar>*)registrar {
  [SwiftOpencvFfiPlugin registerWithRegistrar:registrar];
}
@end

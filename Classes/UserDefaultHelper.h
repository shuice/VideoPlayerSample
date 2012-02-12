#import <UIKit/UIKit.h>
@interface UserDefaultHelper : NSObject
+(NSString*) getValue:(NSString*)strKey;
+(void) setValue:(NSString*)strKey strValue:(NSString*)strValue;
+(void) setValue:(NSString*)strKey iValue:(unsigned long)iValue;
@end

#define USER_DEFAULT_ASPECT_RATIO @"AspectRatio"
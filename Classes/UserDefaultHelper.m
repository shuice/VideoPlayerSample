#import "UserDefaultHelper.h"
#import "PlayerViewDefine.h"
@implementation UserDefaultHelper


+(NSString*) getValue:(NSString*)strKey
{
    NSString* strRet = [[NSUserDefaults standardUserDefaults] stringForKey:strKey];
    return strRet;
}
+(void) setValue:(NSString*)strKey strValue:(NSString*)strValue
{
    [[NSUserDefaults standardUserDefaults] setObject:strValue forKey:strKey];
}
+(void) setValue:(NSString*)strKey iValue:(unsigned long)iValue
{
    [UserDefaultHelper setValue:strKey strValue:[NSString stringWithFormat:@"%u", iValue]];
}

+ (void) init
{
    NSMutableDictionary* dic = [NSMutableDictionary dictionary];
    [dic setValue:[NSNumber numberWithInt:eAspectRadioOriginal] forKey:USER_DEFAULT_ASPECT_RATIO];
    [dic setValue:[NSNumber numberWithInt:75] forKey:USER_DEFAULT_VOLUME];
    [[NSUserDefaults standardUserDefaults] registerDefaults:dic];
}
@end
#import "UserDefaultHelper.h"

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
@end
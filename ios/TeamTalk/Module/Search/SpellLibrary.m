//
//  SpellLibrary.m
//  Duoduo
//
//  Created by 独嘉 on 14-2-27.
//  Copyright (c) 2015年 MoguIM All rights reserved.
//

#import "SpellLibrary.h"
#import "NSString+Additions.h"

@implementation SpellLibrary
{
    NSMutableDictionary* _spellLibrary;
    NSMutableDictionary *_spellLibrary_Deparment;
    NSDictionary* _saucerManDic;
    
}
+ (SpellLibrary*)instance
{
    static SpellLibrary* g_spellLibrary;
    static dispatch_once_t onceToken;
    dispatch_once(&onceToken, ^{
        g_spellLibrary = [[SpellLibrary alloc] init];
    });
    return g_spellLibrary;
}

- (id)init
{
    self = [super init];
    if (self)
    {
        _spellLibrary = [[NSMutableDictionary alloc] init];
        _spellLibrary_Deparment = [NSMutableDictionary new];
        _saucerManDic = @{@"长卿" : @"chang qing",
                          @"朝夕" : @"zhao xi"};
    }
    return self;
}

- (void)clearAllSpell
{
    
}
-(BOOL)isEmpty
{
    return ![[_spellLibrary allKeys] count];
}
- (void)addDeparmentSpellForObject:(id)sender
{
    NSString* word = nil;
    if ([sender isKindOfClass:[MTTUserEntity class]])
    {
        word = [(MTTUserEntity*)sender department];
    }
    else
    {
        return;
    }
    if (!word)
    {
        return;
    }
    
    NSMutableString* spell = _saucerManDic[word];
    if (spell.length != 0)
    {
        spell = [NSMutableString stringWithString:word];
        CFRange range = CFRangeMake(0, spell.length);
        CFStringTransform((CFMutableStringRef)spell, &range, kCFStringTransformMandarinLatin, NO);
        CFStringTransform((CFMutableStringRef)spell, &range, kCFStringTransformStripCombiningMarks, NO);
    }
    NSString* key = [spell lowercaseString];
    if(key.length == 0){
        key = @"0";
    }
    if (![[_spellLibrary_Deparment allKeys] containsObject:spell])
    {
        NSMutableArray* objects = [[NSMutableArray alloc] init];
        
        [objects addObject:sender];
        [_spellLibrary_Deparment setObject:objects forKey:key];
    }
    else
    {
        NSMutableArray* objects = _spellLibrary_Deparment[key];
        if (![objects containsObject:sender])
        {
            [objects addObject:sender];
        }
    }
}
-(void)clearSpellById:(NSString *)objctid
{
    [_spellLibrary removeObjectForKey:objctid];
}
- (void)addSpellForObject:(id)sender
{
    NSString* word = nil;
    if ([sender isKindOfClass:[MTTUserEntity class]])
    {
        word = [(MTTUserEntity*)sender nick];
    }
    else if ([sender isKindOfClass:[MTTGroupEntity class]])
    {
        word = [(MTTGroupEntity*)sender name];
    }
    else
    {
        return;
    }
    if (!word)
    {
        return;
    }
    
    NSMutableString* spell = _saucerManDic[word];
    if (!spell)
    {
        spell = [NSMutableString stringWithString:word];
        CFRange range = CFRangeMake(0, spell.length);
        CFStringTransform((CFMutableStringRef)spell, &range, kCFStringTransformMandarinLatin, NO);
        CFStringTransform((CFMutableStringRef)spell, &range, kCFStringTransformStripCombiningMarks, NO);
    }
    NSString* key = [spell lowercaseString];
    if (![[_spellLibrary allKeys] containsObject:spell])
    {
        NSMutableArray* objects = [[NSMutableArray alloc] init];
        
        [objects addObject:sender];
        [_spellLibrary setObject:objects forKey:key];
    }
    else
    {
        NSMutableArray* objects = _spellLibrary[key];
        if (![objects containsObject:sender])
        {
            [objects addObject:sender];
        }
    }
}

- (NSMutableArray*)checkoutForWordsForSpell:(NSString*)spell
{
    NSMutableArray* result = [[NSMutableArray alloc] init];
    
    [_spellLibrary enumerateKeysAndObjectsUsingBlock:^(id key, id obj, BOOL *stop) {
        //
        NSString* withoutSpaceSpellKey = [(NSString*)key removeAllSpace];
        if ([withoutSpaceSpellKey rangeOfString:spell].length > 0)
        {
            [result addObjectsFromArray:(NSArray*)obj];
        }
        
        //拼音简写搜索
        NSArray* spellWords = [(NSString*)key componentsSeparatedByString:@" "];
        for (int index = 0; index < [spellWords count]; index ++)
        {
            NSString* briefSpell = [self briefSpellWordFromSpellArray:spellWords fullWord:index];
            if ([briefSpell rangeOfString:spell].length > 0)
            {
                [(NSArray*)obj enumerateObjectsUsingBlock:^(id obj, NSUInteger idx, BOOL *stop) {
                    if (![result containsObject:obj])
                    {
                        [result addObject:obj];
                    }
                }];
            }
        }
    }];
    return result;
}
- (NSMutableArray*)checkoutForWordsForSpell_Deparment:(NSString*)spell
{
    NSMutableArray* result = [[NSMutableArray alloc] init];
    
    [_spellLibrary_Deparment enumerateKeysAndObjectsUsingBlock:^(id key, id obj, BOOL *stop) {
        //
        NSString* withoutSpaceSpellKey = [(NSString*)key removeAllSpace];
        if ([withoutSpaceSpellKey rangeOfString:spell].length > 0)
        {
            [result addObjectsFromArray:(NSArray*)obj];
        }
        
        //拼音简写搜索
        NSArray* spellWords = [(NSString*)key componentsSeparatedByString:@" "];
        for (int index = 0; index < [spellWords count]; index ++)
        {
            NSString* briefSpell = [self briefSpellWordFromSpellArray:spellWords fullWord:index];
            if ([briefSpell rangeOfString:spell].length > 0)
            {
                [(NSArray*)obj enumerateObjectsUsingBlock:^(id obj, NSUInteger idx, BOOL *stop) {
                    if (![result containsObject:obj])
                    {
                        [result addObject:obj];
                    }
                }];
            }
        }
    }];
    return result;

}
- (NSString*)getSpellForWord:(NSString*)word
{
    NSMutableString *spell = [NSMutableString stringWithString:word];
    CFRange range = CFRangeMake(0, spell.length);
    CFStringTransform((CFMutableStringRef)spell, &range, kCFStringTransformMandarinLatin, NO);
    CFStringTransform((CFMutableStringRef)spell, &range, kCFStringTransformStripCombiningMarks, NO);
    spell = (NSMutableString*)[spell removeAllSpace];
    return spell;
}

- (NSString*)briefSpellWordFromSpellArray:(NSArray*)sender fullWord:(int)count
{
    NSMutableString* briefSpell = [[NSMutableString alloc] init];
    for (int index = 0; index < [sender count]; index ++)
    {
        NSString* fullSpell = sender[index];
        if ([fullSpell length] == 0)
        {
            continue;
        }
        if (index < count)
        {
            [briefSpell appendString:fullSpell];
        }
        else
        {
            NSString* briefSpellAtIndex = [fullSpell substringToIndex:1];
            [briefSpell appendString:briefSpellAtIndex];
        }
    }
    return briefSpell;
}

@end

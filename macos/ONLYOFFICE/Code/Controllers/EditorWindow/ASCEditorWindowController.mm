/*
 * (c) Copyright Ascensio System SIA 2010-2019
 *
 * This program is a free software product. You can redistribute it and/or
 * modify it under the terms of the GNU Affero General Public License (AGPL)
 * version 3 as published by the Free Software Foundation. In accordance with
 * Section 7(a) of the GNU AGPL its Section 15 shall be amended to the effect
 * that Ascensio System SIA expressly excludes the warranty of non-infringement
 * of any third-party rights.
 *
 * This program is distributed WITHOUT ANY WARRANTY; without even the implied
 * warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR  PURPOSE. For
 * details, see the GNU AGPL at: http://www.gnu.org/licenses/agpl-3.0.html
 *
 * You can contact Ascensio System SIA at 20A-12 Ernesta Birznieka-Upisha
 * street, Riga, Latvia, EU, LV-1050.
 *
 * The  interactive user interfaces in modified source and object code versions
 * of the Program must display Appropriate Legal Notices, as required under
 * Section 5 of the GNU AGPL version 3.
 *
 * Pursuant to Section 7(b) of the License you must retain the original Product
 * logo when distributing the program. Pursuant to Section 7(e) we decline to
 * grant you any rights under trademark law for use of our trademarks.
 *
 * All the Product's GUI elements, including illustrations and icon sets, as
 * well as technical writing content are licensed under the terms of the
 * Creative Commons Attribution-ShareAlike 4.0 International. See the License
 * terms at http://creativecommons.org/licenses/by-sa/4.0/legalcode
 *
*/

//
//  ASCEditorWindowController.m
//  ONLYOFFICE
//
//  Copyright (c) 2025 Ascensio System SIA. All rights reserved.
//

#import "ASCEditorWindowController.h"
#import "ASCEditorWindow.h"
#import "AppDelegate.h"
#import "ASCConstants.h"
#import "ASCHelper.h"
#import "NSCefView.h"
#import "NSCefData.h"

@interface ASCEditorWindowController () <NSWindowDelegate>
@end

@implementation ASCEditorWindowController

+ (instancetype)initWithFrame:(NSRect)frame {
    NSStoryboard *storyboard = [NSStoryboard storyboardWithName:StoryboardNameEditor bundle:nil];
    ASCEditorWindowController * controller = [storyboard instantiateControllerWithIdentifier:@"EditorWindowController"];
    [controller.window setFrame:frame display:NO];
    return controller;
}

- (void)windowDidLoad {
    NSString * productName = [ASCHelper appName];
    self.window.title = productName;
    
    [super windowDidLoad];
    self.window.delegate = self;
}

- (void)windowDidMove:(NSNotification *)notification {
    [[NSNotificationCenter defaultCenter] postNotificationName:ASCEventNameEditorWindowMoving object:self.window];
}

- (void)windowWillClose:(NSNotification *)notification {
    AppDelegate *app = (AppDelegate *)[NSApp delegate];
    [app.windowControllers removeObject:self];
}

- (BOOL)windowShouldClose:(NSWindow *)sender {
    return [self shouldTerminateApplication];
}

- (BOOL)shouldTerminateApplication {
    BOOL hasUnsaved = NO;
    
    [[NSNotificationCenter defaultCenter] postNotificationName:CEFEventNameFullscreen
                                                        object:nil
                                                      userInfo:@{@"fullscreen" : @(NO),
                                                                 @"terminate"  : @(YES)
                                                               }];
    ASCEditorWindow *window = (ASCEditorWindow *)self.window;
    NSCefView *cefView = (NSCefView *)window.webView;
    if ([cefView.data hasChanges]) {
        hasUnsaved = YES;
    }
    
    // Blockchain check
    if ([cefView checkCloudCryptoNeedBuild]) {
        return NO;
    } else {
        if ([cefView isSaveLocked]) {
            hasUnsaved = YES;
        }
    }
    
    if (hasUnsaved) {
        NSString * productName = [ASCHelper appName];
        
        NSAlert *alert = [[NSAlert alloc] init];
        [alert addButtonWithTitle:NSLocalizedString(@"Review Changes...", nil)];
        [[alert addButtonWithTitle:NSLocalizedString(@"Cancel", nil)] setKeyEquivalent:@"\e"];
        [alert addButtonWithTitle:NSLocalizedString(@"Delete and Quit", nil)];
        [alert setMessageText:[NSString stringWithFormat:NSLocalizedString(@"You have %@ document with unconfirmed changes. Do you want to review these changes before quitting?", nil), productName]];
        [alert setInformativeText:NSLocalizedString(@"If you don't review your documents, all your changeses will be lost.", nil)];
        [alert setAlertStyle:NSAlertStyleInformational];
        
        NSInteger result = [alert runModal];
        if (result == NSAlertFirstButtonReturn) {
            // "Review Changes..." clicked
            // TODO: add impl safeCloseTabsWithChanges;
        } else
        if (result == NSAlertSecondButtonReturn) {
            // "Cancel" clicked
            return NO;
        } else {
            // "Delete and Quit" clicked
            // TODO: add impl removeTab;
        }
        return NO;
    }
    return YES;
}

@end

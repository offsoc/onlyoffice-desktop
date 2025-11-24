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
#import "AnalyticsHelper.h"
#import "ASCHelper.h"
#import "NSCefView.h"
#import "NSCefData.h"
#import "NSString+Extensions.h"
#import "ASCSavePanelWithFormatController.h"

@interface ASCEditorWindowController () <NSWindowDelegate>
@property (nonatomic) BOOL waitingForClose;
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
    
    void (^addObserverFor)(_Nullable NSNotificationName, SEL) = ^(_Nullable NSNotificationName name, SEL selector) {
        [[NSNotificationCenter defaultCenter] addObserver:self
                                                 selector:selector
                                                     name:name
                                                   object:nil];
    };
    
    addObserverFor(CEFEventNameModifyChanged, @selector(onCEFModifyChanged:));
    addObserverFor(CEFEventNameSave, @selector(onCEFSave:));
    addObserverFor(CEFEventNameStartSaveDialog, @selector(onCEFStartSave:));
    addObserverFor(CEFEventNameSaveLocal, @selector(onCEFSaveLocalFile:));
    addObserverFor(CEFEventNameDocumentFragmentBuild, @selector(onCEFDocumentFragmentBuild:));
}

- (void)windowDidMove:(NSNotification *)notification {
    [[NSNotificationCenter defaultCenter] postNotificationName:ASCEventNameEditorWindowMoving object:self.window];
}

- (void)windowWillClose:(NSNotification *)notification {
    ASCEditorWindow *window = (ASCEditorWindow *)self.window;
    NSCefView *cefView = (NSCefView *)window.webView;
    if (cefView) {
        CAscApplicationManager * appManager = [NSAscApplicationWorker getAppManager];
        appManager->DestroyCefView((int)cefView.uuid);
        [cefView internalClean];
        window.webView = nil;
    }
    
    AppDelegate *app = (AppDelegate *)[NSApp delegate];
    [app.windowControllers removeObject:self];
}

- (BOOL)windowShouldClose:(NSWindow *)sender {
    if (self.waitingForClose)
        return NO;
    
    return [self shouldCloseWindow];
}

- (BOOL)shouldCloseWindow {
    BOOL hasUnsaved = NO;
    
    [[NSNotificationCenter defaultCenter] postNotificationName:CEFEventNameFullscreen
                                                        object:nil
                                                      userInfo:@{@"fullscreen" : @(NO),
                                                                 @"terminate"  : @(YES)
                                                               }];
    ASCEditorWindow *window = (ASCEditorWindow *)self.window;
    NSCefView *cefView = (NSCefView *)window.webView;
    if (!cefView) {
        return YES;
    }
    
    if ([cefView.data hasChanges]) {
        hasUnsaved = YES;
    }
    
    // Blockchain check
    if ([cefView checkCloudCryptoNeedBuild]) {
        self.waitingForClose = YES;
        return NO;
        
    } else {
        if ([cefView isSaveLocked]) {
            hasUnsaved = YES;
        }
    }
    
    if (hasUnsaved) {
        NSAlert *alert = [[NSAlert alloc] init];
        [alert addButtonWithTitle:NSLocalizedString(@"Save", nil)];
        [alert addButtonWithTitle:NSLocalizedString(@"Don't Save", nil)];
        [[alert addButtonWithTitle:NSLocalizedString(@"Cancel", nil)] setKeyEquivalent:@"\e"];
        [alert setMessageText:[NSString stringWithFormat:NSLocalizedString(@"Do you want to save the changes made to the document \"%@\"?", nil), [cefView.data title:YES]]];
        [alert setInformativeText:NSLocalizedString(@"Your changes will be lost if you don't save them.", nil)];
        [alert setAlertStyle:NSAlertStyleWarning];

        NSInteger returnCode = [alert runModal];
        if (returnCode == NSAlertFirstButtonReturn) {
            // Save
            NSEditorApi::CAscMenuEvent * pEvent = new NSEditorApi::CAscMenuEvent(ASC_MENU_EVENT_TYPE_CEF_SAVE);
            [cefView apply:pEvent];
            
            self.waitingForClose = YES;
            return NO;

        } else
        if (returnCode == NSAlertSecondButtonReturn) {
            // Don't Save

        } else
        if (returnCode == NSAlertThirdButtonReturn) {
            // Cancel
            return NO;
        }
    }
    return YES;
}

- (BOOL)hasViewId:(NSString *)viewId {
    ASCEditorWindow *window = (ASCEditorWindow *)self.window;
    NSCefView *cefView = (NSCefView *)window.webView;
    if (cefView) {
        NSString *uuid = [NSString stringWithFormat:@"%ld", cefView.uuid];
        if ([viewId isEqualToString:uuid])
            return YES;
    }
    return NO;
}

- (NSCefData *)cefData {
    ASCEditorWindow *window = (ASCEditorWindow *)self.window;
    NSCefView *cefView = (NSCefView *)window.webView;
    if (cefView) {
        return cefView.data;
    }
    return nil;
}

- (NSMutableDictionary *)params {
    if (nil == _params) {
        _params = [NSMutableDictionary dictionary];
    }
    
    return _params;
}

#pragma mark -
#pragma mark Notification handlers

- (void)onWindowLoaded:(NSNotification *)notification {
    if (notification && notification.object) {
        
        // Create CEF event listener
        // [ASCEventsController sharedInstance];
    }
}

#pragma mark -
#pragma mark CEF events handlers

- (void)onCEFModifyChanged:(NSNotification *)notification {
    if (notification && notification.userInfo) {
        NSDictionary * params = (NSDictionary *)notification.userInfo;
        NSString * viewId = params[@"viewId"];
        BOOL changed = [params[@"сhanged"] boolValue];
        if ([self hasViewId:viewId]) {
            [self.cefData setChanged:changed];
        }
    }
}

- (void)onCEFSave:(NSNotification *)notification {
    if (notification && notification.userInfo) {
        NSDictionary * params = (NSDictionary *)notification.userInfo;
        if ( ![params[@"cancel"] boolValue] ) {
            NSString * viewId = params[@"viewId"];
            if ([self hasViewId:viewId] && self.waitingForClose) {
                [self.window close];
            }
        }
    }
}

- (void)onCEFStartSave:(NSNotification *)notification {
    if (notification && notification.userInfo) {
        NSString * fileName = notification.userInfo[@"fileName"];
        NSNumber * idx      = notification.userInfo[@"idx"];
        
        /*NSSavePanel * savePanel = [NSSavePanel savePanel];
        //        [savePanel setDirectoryURL:[NSURL URLWithString:[NSSearchPathForDirectoriesInDomains(NSDownloadsDirectory, NSUserDomainMask, YES) firstObject]]];
        if (fileName && fileName.length > 0) {
            [savePanel setAllowedFileTypes:@[fileName.pathExtension]];
            [savePanel setNameFieldStringValue:[fileName lastPathComponent]];
        }
        
        CAscApplicationManager * appManager = [NSAscApplicationWorker getAppManager];
        
        [savePanel beginSheetModalForWindow:[NSApp mainWindow] completionHandler:^(NSInteger result){
            [savePanel orderOut:self];
            
            if (result == NSFileHandlingPanelOKButton) {
                appManager->EndSaveDialog([[[savePanel URL] path] stdwstring], [idx unsignedIntValue]);
            } else {
                appManager->EndSaveDialog(L"", [idx unsignedIntValue]);
            }
        }];*/
    }
}



- (void)saveLocalFileWithParams:(NSDictionary *)params {
    if (params) {
        NSString * path         = params[@"path"];
        NSString * viewId       = params[@"viewId"];
        NSArray * formats       = params[@"supportedFormats"];
                
        //        __block NSInteger fileType = [params[@"fileType"] intValue];
        
        __block ASCSavePanelWithFormatController * saveController = [ASCSavePanelWithFormatController new];
        
        NSSavePanel * savePanel = [saveController savePanel];
        
        saveController.filters = formats;
        saveController.original = params[@"original"];
        //        saveController.filterType = fileType;
        
        if (!path || path.length < 1) {
            path = [NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDomainMask, YES) firstObject];
        }
        
        BOOL isDir;
        if (![[NSFileManager defaultManager] fileExistsAtPath:path isDirectory:&isDir]) {
            NSString * savedPath = [[NSUserDefaults standardUserDefaults] objectForKey:ASCUserLastSavePath];
            
            if (savedPath && savedPath.length > 0) {
                path = [savedPath stringByAppendingPathComponent:path];
            } else {
                path = [[NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDomainMask, YES) firstObject] stringByAppendingPathComponent:path];
            }
        }
        
        savePanel.directoryURL = [NSURL fileURLWithPath:[path stringByDeletingLastPathComponent]];
        savePanel.canCreateDirectories = YES;
        savePanel.nameFieldStringValue = [[path lastPathComponent] stringByDeletingPathExtension];
        
        [savePanel beginSheetModalForWindow:[NSApp mainWindow] completionHandler:^(NSInteger result){
            [savePanel orderOut:self];
            
            NSEditorApi::CAscLocalSaveFileDialog * saveData = new NSEditorApi::CAscLocalSaveFileDialog();
            CAscApplicationManager * appManager = [NSAscApplicationWorker getAppManager];
            
            if (result == NSFileHandlingPanelOKButton) {
                [[NSUserDefaults standardUserDefaults] setObject:[[savePanel directoryURL] path] forKey:ASCUserLastSavePath];
                [[NSUserDefaults standardUserDefaults] synchronize];
                
                NSString * path = [NSString stringWithFormat:@"%@", [[savePanel URL] path]];
                
                saveData->put_Path([path stdwstring]);
                saveData->put_Id([viewId intValue]);
                saveData->put_FileType((int)[saveController filterType]);
            } else {
                saveData->put_Id([viewId intValue]);
                saveData->put_Path(L"");
            }
            
            NSEditorApi::CAscMenuEvent* pEvent = new NSEditorApi::CAscMenuEvent(ASC_MENU_EVENT_TYPE_CEF_LOCALFILE_SAVE_PATH);
            pEvent->m_pData = saveData;
            
            appManager->Apply(pEvent);
        }];
    }
}



- (void)onCEFSaveLocalFile:(NSNotification *)notification {
    if (notification && notification.userInfo) {
        NSDictionary * params = (NSDictionary *)notification.userInfo;
        NSString * viewId = params[@"viewId"];
        if ([self hasViewId:viewId]) {
            [self.params addEntriesFromDictionary:params];
            [self saveLocalFileWithParams:params];
            
            [[AnalyticsHelper sharedInstance] recordCachedEventWithCategory:ASCAnalyticsCategoryApplication
                                                                     action:@"Save local file"
                                                                      label:nil
                                                                      value:nil];
        }
    }
}

- (void)onCEFDocumentFragmentBuild:(NSNotification *)notification {
    if (notification && notification.userInfo) {
        id json = notification.userInfo;
        
        NSString * viewId = json[@"viewId"];
        int error = [json[@"error"] intValue];
                
        if ([self hasViewId:viewId]) {
            if (error == 0 && self.waitingForClose) {
                [self.window close];
            }
        }
    }
}

@end

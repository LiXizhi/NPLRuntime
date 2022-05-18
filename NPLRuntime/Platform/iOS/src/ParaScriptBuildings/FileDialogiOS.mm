//-----------------------------------------------------------------------------
// Class: OpenFileDialog.h
// Authors: big
// Emails: onedous@gmail.com
// CreateDate: 2022.05.12
//-----------------------------------------------------------------------------

#import "AppDelegate.h"
#import <UIKit/UIKit.h>
#import <Foundation/Foundation.h>

#include "ParaEngine.h"
#include "NPLRuntime.h"
#include "NPLScriptingState.h"
#include "FileDialogiOS.h"
#include <sstream>
#include <iostream>

// MARK: - Category AppDelegate

@interface AppDelegate (FileDialog)
    <UIDocumentPickerDelegate,
     UIImagePickerControllerDelegate,
     UINavigationControllerDelegate>

@end

@implementation AppDelegate(FileDialog)

- (void)documentPicker:(UIDocumentPickerViewController *)controller didPickDocumentsAtURLs:(NSArray<NSURL *> *)urls
{
    BOOL fileUrlAuthozied = [urls.firstObject startAccessingSecurityScopedResource];

    if (fileUrlAuthozied) {
        NSFileCoordinator *fileCoordinator = [[NSFileCoordinator alloc] init];
        NSError *error;

        [fileCoordinator coordinateReadingItemAtURL:urls.firstObject options:0 error:&error byAccessor:^(NSURL *newURL) {

            FileDialogiOS::openCallback([[newURL absoluteString] UTF8String]);

            auto appDelegate = (AppDelegate*)[[UIApplication sharedApplication] delegate];
            [appDelegate.viewController dismissViewControllerAnimated:YES completion:NULL];
        }];

        [urls.firstObject stopAccessingSecurityScopedResource];
    }
}

- (void)documentPickerWasCancelled:(UIDocumentPickerViewController *)controller
{
}

- (void)imagePickerController:(UIImagePickerController *)picker didFinishPickingMediaWithInfo:(NSDictionary<UIImagePickerControllerInfoKey,id> *)info
{
    [picker dismissViewControllerAnimated:YES completion:nil];
    
    NSString *type = [info objectForKey:UIImagePickerControllerMediaType];
    
    if ([type isEqualToString:@"public.image"]) {
        FileDialogiOS::openCallback([[[info objectForKey:UIImagePickerControllerImageURL] absoluteString] UTF8String]);
    }
}

- (void)imagePickerControllerDidCancel:(UIImagePickerController *)picker
{
    [picker dismissViewControllerAnimated:YES completion:nil];
}

@end

// MARK: - FileDialog

@interface FileDialog : NSObject

- (void)open;

@end

@implementation FileDialog

- (void)open
{
    auto appDelegate = (AppDelegate*)[[UIApplication sharedApplication] delegate];
    
    NSArray *types = @[
        @"public.content",
        @"public.text",
        @"public.image",
        @"com.adobe.pdf",
        @"com.microsoft.word.doc",
        @"com.microsoft.excel.xls",
        @"com.microsoft.powerpoint.ppt"
    ];

    UIDocumentPickerViewController *uiDocumentPickerViewController =
        [[UIDocumentPickerViewController alloc] initWithDocumentTypes:types inMode:UIDocumentPickerModeOpen];

    uiDocumentPickerViewController.delegate = appDelegate;
    uiDocumentPickerViewController.modalPresentationStyle = UIModalPresentationFormSheet;

    [appDelegate.viewController presentViewController:uiDocumentPickerViewController animated:YES completion:nil];
}

@end

// MARK: - PhotoDialog

@interface PhotoDialog : NSObject
{
    UIImagePickerController *imagePickerController;
}


- (void)open;

@end

@implementation PhotoDialog

- (void)open
{
    imagePickerController = [[UIImagePickerController alloc] init];
    auto appDelegate = (AppDelegate*)[[UIApplication sharedApplication] delegate];

    imagePickerController.delegate = appDelegate;

    [appDelegate.viewController presentViewController:imagePickerController animated:YES completion:nil];
}

@end

// MARK: - FileDialogiOS

int FileDialogiOS::callbackId = 0;
string FileDialogiOS::activateFile = "";

void FileDialogiOS::open(const string &filter, const int callbackId, const string &activateFile)
{
    FileDialogiOS::callbackId = callbackId;
    FileDialogiOS::activateFile = activateFile;

    if (filter == "image/*") {
        [[PhotoDialog alloc] open];
    } else {
        [[FileDialog alloc] open];
    }
}

void FileDialogiOS::openCallback(const string &filepath) {    
    string _filepath = filepath;
    const string ori = "file:///";
    const string dest = "/";

    string::size_type pos = 0;
    string::size_type oriSize = ori.size();
    string::size_type destSize = dest.size();

    while ((pos = _filepath.find(ori, pos)) != string::npos) {
        _filepath.replace(pos, oriSize, dest);
        pos += destSize;
    }

    stringstream ss;

    ss << "msg={_callbackIdx=" << callbackId << ",filepath=[[" << _filepath << "]]}" << endl;
    NPL::NPLRuntimeState_ptr rsptr = NPL::CNPLRuntime::GetInstance()->GetRuntimeState(activateFile);
    NPL::CNPLRuntime::GetInstance()->NPL_Activate(rsptr, activateFile.c_str(), ss.str().c_str());
}

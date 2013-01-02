//
//  KegCommunicatorMasterViewController.h
//  KegCommunicator
//
//  Created by Ken Lahm on 1/2/13.
//  Copyright (c) 2013 Ken Lahm. All rights reserved.
//

#import <UIKit/UIKit.h>

@class KegCommunicatorDetailViewController;

@interface KegCommunicatorMasterViewController : UITableViewController

@property (strong, nonatomic) KegCommunicatorDetailViewController *detailViewController;

@end

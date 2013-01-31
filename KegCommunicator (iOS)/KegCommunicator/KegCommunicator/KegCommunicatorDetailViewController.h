//
//  KegCommunicatorDetailViewController.h
//  KegCommunicator
//
//  Created by Ken Lahm on 1/2/13.
//  Copyright (c) 2013 Ken Lahm. All rights reserved.
//

#import <UIKit/UIKit.h>

@interface KegCommunicatorDetailViewController : UIViewController <UISplitViewControllerDelegate>

@property (strong, nonatomic) id detailItem;

@property (weak, nonatomic) IBOutlet UILabel *detailDescriptionLabel;
@end

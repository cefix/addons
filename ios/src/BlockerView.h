//
//  BlockerView.h
//  cefix_presentation_ios
//
//  Created by Stephan Huber on 30.07.12.
//
//

#import <UIKit/UIKit.h>

@interface BlockerView : UIView

- (void)touchesBegan:(NSSet *)touches withEvent:(UIEvent *)event;
- (void)touchesMoved:(NSSet *)touches withEvent:(UIEvent *)event;
- (void)touchesEnded:(NSSet *)touches withEvent:(UIEvent *)event;


@end

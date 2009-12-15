// Copyright (c) 2009 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#import "hover_close_button.h"

#include "base/nsimage_cache_mac.h"

namespace  {

const NSString* kNormalImageString = @"close_bar.pdf";
const NSString* kHoverImageString = @"close_bar_h.pdf";

}

@implementation HoverCloseButton

- (void)awakeFromNib {
  [self setTrackingEnabled:YES];
}

- (void)dealloc {
  [self setTrackingEnabled:NO];
  [super dealloc];
}

- (void)mouseEntered:(NSEvent*)theEvent {
  [self setImage:nsimage_cache::ImageNamed(kHoverImageString)];
}

- (void)mouseExited:(NSEvent*)theEvent {
  [self setImage:nsimage_cache::ImageNamed(kNormalImageString)];
}

- (void)setTrackingEnabled:(BOOL)enabled {
  if (enabled) {
    closeTrackingArea_.reset(
        [[NSTrackingArea alloc] initWithRect:[self bounds]
                                     options:NSTrackingMouseEnteredAndExited |
                                             NSTrackingActiveAlways
                                       owner:self
                                    userInfo:nil]);
    [self addTrackingArea:closeTrackingArea_.get()];
  } else {
    if (closeTrackingArea_.get()) {
      [self removeTrackingArea:closeTrackingArea_.get()];
      closeTrackingArea_.reset(nil);
    }
  }
}

- (void)updateTrackingAreas {
  [super updateTrackingAreas];
  if (closeTrackingArea_.get()) {
    // Update the close buttons if the tab has moved.
    NSPoint mouseLoc = [[self window] mouseLocationOutsideOfEventStream];
    mouseLoc = [self convertPointFromBase:mouseLoc];
    NSString* name = NSPointInRect(mouseLoc, [self frame]) ?
        kHoverImageString : kNormalImageString;
    NSImage* newImage = nsimage_cache::ImageNamed(name);
    NSImage* buttonImage = [self image];
    if (![buttonImage isEqual:newImage]) {
      [self setImage:newImage];
    }
  }
}
@end
#!/usr/bin/python

# Copyright (c) 2009 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

"""Makes sure that the app/ code is cpplint clean."""

INCLUDE_CPP_FILES_ONLY = (
  r'.*\.cc$', r'.*\.h$'
)

EXCLUDE = (
  # Autogenerated window resources files are off limits
  r'.*resource.h$',
)

def CheckChangeOnUpload(input_api, output_api):
  results = []
  black_list = input_api.DEFAULT_BLACK_LIST + EXCLUDE
  sources = lambda x: input_api.FilterSourceFile(
    x, white_list=INCLUDE_CPP_FILES_ONLY, black_list=black_list)
  results.extend(input_api.canned_checks.CheckChangeLintsClean(
      input_api, output_api, sources))
  return results
// Copyright (c) 2009 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROME_BROWSER_AUTOFILL_AUTOFILL_TYPE_H_
#define CHROME_BROWSER_AUTOFILL_AUTOFILL_TYPE_H_

#include <map>
#include <set>
#include <string>

#include "base/string16.h"
#include "chrome/browser/autofill/field_types.h"

// The high-level description of AutoFill types, used to categorize form fields
// and for associating form fields with form values in the Web Database.
class AutoFillType {
 public:
  enum FieldTypeGroup {
    NO_GROUP,
    CONTACT_INFO,
    ADDRESS_HOME,
    ADDRESS_BILLING,
    PHONE_HOME,
    PHONE_FAX,
    CREDIT_CARD,
  };

  enum FieldTypeSubGroup {
    NO_SUBGROUP,
    // Address subgroups.
    ADDRESS_LINE1,
    ADDRESS_LINE2,
    ADDRESS_APPT_NUM,
    ADDRESS_CITY,
    ADDRESS_STATE,
    ADDRESS_ZIP,
    ADDRESS_COUNTRY,

    // Phone subgroups.
    PHONE_NUMBER,
    PHONE_CITY_CODE,
    PHONE_COUNTRY_CODE,
    PHONE_CITY_AND_NUMBER,
    PHONE_WHOLE_NUMBER
  };

  struct AutoFillTypeDefinition {
    AutoFillFieldType field_type;
    FieldTypeGroup group;
    FieldTypeSubGroup subgroup;
    std::string name;
  };

  AutoFillType() {}
  explicit AutoFillType(AutoFillTypeDefinition* definition);
  explicit AutoFillType(AutoFillFieldType field_type);

  AutoFillFieldType field_type() const {
    return autofill_type_definition_->field_type;
  }
  FieldTypeGroup group() const { return autofill_type_definition_->group; }
  FieldTypeSubGroup subgroup() const {
    return autofill_type_definition_->subgroup;
  }

 private:
  static void StaticInitialize();
  static void InitializeFieldTypeMap();

  static bool initialized_;
  static AutoFillType types_[MAX_VALID_FIELD_TYPE + 1];

  const AutoFillTypeDefinition* autofill_type_definition_;
};

typedef AutoFillType::FieldTypeGroup FieldTypeGroup;
typedef AutoFillType::FieldTypeSubGroup FieldTypeSubGroup;
typedef std::set<AutoFillFieldType> FieldTypeSet;
typedef std::map<string16, AutoFillFieldType> FieldTypeMap;

#endif  // CHROME_BROWSER_AUTOFILL_AUTOFILL_TYPE_H_
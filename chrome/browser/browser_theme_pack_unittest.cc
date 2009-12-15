// Copyright (c) 2009 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/browser/browser_theme_pack.h"

#include "app/gfx/color_utils.h"
#include "app/theme_provider.h"
#include "base/file_util.h"
#include "base/json/json_reader.h"
#include "base/path_service.h"
#include "base/scoped_temp_dir.h"
#include "base/values.h"
#include "chrome/browser/theme_resources_util.h"
#include "chrome/browser/browser_theme_provider.h"
#include "chrome/common/chrome_paths.h"
#include "chrome/common/json_value_serializer.h"
#include "grit/theme_resources.h"
#include "testing/gtest/include/gtest/gtest.h"

class BrowserThemePackTest : public ::testing::Test {
 public:
  BrowserThemePackTest() : theme_pack_(new BrowserThemePack) { }

  // Transformation for link underline colors.
  SkColor BuildThirdOpacity(SkColor color_link) {
    return SkColorSetA(color_link, SkColorGetA(color_link) / 3);
  }

  void GenerateDefaultFrameColor(std::map<int, SkColor>* colors,
                                 int color, int tint) {
    (*colors)[color] = HSLShift(
        BrowserThemeProvider::GetDefaultColor(
            BrowserThemeProvider::COLOR_FRAME),
        BrowserThemeProvider::GetDefaultTint(tint));
  }

  // Returns a mapping from each COLOR_* constant to the default value for this
  // constant. Callers get this map, and then modify expected values and then
  // run the resulting thing through VerifyColorMap().
  std::map<int, SkColor> GetDefaultColorMap() {
    std::map<int, SkColor> colors;
    for (int i = BrowserThemeProvider::COLOR_FRAME;
         i <= BrowserThemeProvider::COLOR_BUTTON_BACKGROUND; ++i) {
      colors[i] = BrowserThemeProvider::GetDefaultColor(i);
    }

    GenerateDefaultFrameColor(&colors, BrowserThemeProvider::COLOR_FRAME,
                              BrowserThemeProvider::TINT_FRAME);
    GenerateDefaultFrameColor(&colors,
                              BrowserThemeProvider::COLOR_FRAME_INACTIVE,
                              BrowserThemeProvider::TINT_FRAME_INACTIVE);
    GenerateDefaultFrameColor(&colors,
                              BrowserThemeProvider::COLOR_FRAME_INCOGNITO,
                              BrowserThemeProvider::TINT_FRAME_INCOGNITO);
    GenerateDefaultFrameColor(
        &colors,
        BrowserThemeProvider::COLOR_FRAME_INCOGNITO_INACTIVE,
        BrowserThemeProvider::TINT_FRAME_INCOGNITO_INACTIVE);

    return colors;
  }

  void VerifyColorMap(const std::map<int, SkColor>& color_map) {
    for (std::map<int, SkColor>::const_iterator it = color_map.begin();
         it != color_map.end(); ++it) {
      SkColor color = BrowserThemeProvider::GetDefaultColor(it->first);
      theme_pack_->GetColor(it->first, &color);
      EXPECT_EQ(it->second, color) << "Color id = " << it->first;
    }
  }

  void LoadColorJSON(const std::string& json) {
    scoped_ptr<Value> value(base::JSONReader::Read(json, false));
    ASSERT_TRUE(value->IsType(Value::TYPE_DICTIONARY));
    theme_pack_->BuildColorsFromJSON(
        static_cast<DictionaryValue*>(value.get()));
  }

  void LoadTintJSON(const std::string& json) {
    scoped_ptr<Value> value(base::JSONReader::Read(json, false));
    ASSERT_TRUE(value->IsType(Value::TYPE_DICTIONARY));
    theme_pack_->BuildTintsFromJSON(
        static_cast<DictionaryValue*>(value.get()));
  }

  void LoadDisplayPropertiesJSON(const std::string& json) {
    scoped_ptr<Value> value(base::JSONReader::Read(json, false));
    ASSERT_TRUE(value->IsType(Value::TYPE_DICTIONARY));
    theme_pack_->BuildDisplayPropertiesFromJSON(
        static_cast<DictionaryValue*>(value.get()));
  }

  void ParseImageNames(const std::string& json,
                       std::map<int, FilePath>* out_file_paths) {
    scoped_ptr<Value> value(base::JSONReader::Read(json, false));
    ASSERT_TRUE(value->IsType(Value::TYPE_DICTIONARY));
    theme_pack_->ParseImageNamesFromJSON(
        static_cast<DictionaryValue*>(value.get()),
        FilePath(), out_file_paths);
  }

  FilePath GetStarGazingPath() {
    FilePath test_path;
    if (!PathService::Get(chrome::DIR_TEST_DATA, &test_path)) {
      NOTREACHED();
      return test_path;
    }

    test_path = test_path.AppendASCII("profiles");
    test_path = test_path.AppendASCII("complex_theme");
    test_path = test_path.AppendASCII("Default");
    test_path = test_path.AppendASCII("Extensions");
    test_path = test_path.AppendASCII("mblmlcbknbnfebdfjnolmcapmdofhmme");
    test_path = test_path.AppendASCII("1.1");
    return FilePath(test_path);
  }

  // Verifies the data in star gazing. We do this multiple times for different
  // BrowserThemePack objects to make sure it works in generated and mmapped
  // mode correctly.
  void VerifyStarGazing(BrowserThemePack* pack) {
    // First check that values we know exist, exist.
    SkColor color;
    EXPECT_TRUE(pack->GetColor(BrowserThemeProvider::COLOR_BOOKMARK_TEXT,
                               &color));
    EXPECT_EQ(SK_ColorBLACK, color);

    EXPECT_TRUE(pack->GetColor(BrowserThemeProvider::COLOR_NTP_BACKGROUND,
                               &color));
    EXPECT_EQ(SkColorSetRGB(57, 137, 194), color);

    color_utils::HSL expected = { 0.6, 0.553, 0.5 };
    color_utils::HSL actual;
    EXPECT_TRUE(pack->GetTint(BrowserThemeProvider::TINT_BUTTONS, &actual));
    EXPECT_DOUBLE_EQ(expected.h, actual.h);
    EXPECT_DOUBLE_EQ(expected.s, actual.s);
    EXPECT_DOUBLE_EQ(expected.l, actual.l);

    int val;
    EXPECT_TRUE(pack->GetDisplayProperty(
        BrowserThemeProvider::NTP_BACKGROUND_ALIGNMENT, &val));
    EXPECT_EQ(BrowserThemeProvider::ALIGN_TOP, val);

    EXPECT_TRUE(pack->HasCustomImage(IDR_THEME_FRAME));

    // Make sure we don't have phantom data.
    EXPECT_FALSE(pack->GetColor(BrowserThemeProvider::COLOR_CONTROL_BACKGROUND,
                                &color));
    EXPECT_FALSE(pack->GetTint(BrowserThemeProvider::TINT_FRAME, &actual));
  }

  scoped_refptr<BrowserThemePack> theme_pack_;
};


TEST_F(BrowserThemePackTest, DeriveUnderlineLinkColor) {
  // If we specify a link color, but don't specify the underline color, the
  // theme provider should create one.
  std::string color_json = "{ \"ntp_link\": [128, 128, 128],"
                           "  \"ntp_section_link\": [128, 128, 128] }";
  LoadColorJSON(color_json);

  std::map<int, SkColor> colors = GetDefaultColorMap();
  SkColor link_color = SkColorSetRGB(128, 128, 128);
  colors[BrowserThemeProvider::COLOR_NTP_LINK] = link_color;
  colors[BrowserThemeProvider::COLOR_NTP_LINK_UNDERLINE] =
      BuildThirdOpacity(link_color);
  colors[BrowserThemeProvider::COLOR_NTP_SECTION_LINK] = link_color;
  colors[BrowserThemeProvider::COLOR_NTP_SECTION_LINK_UNDERLINE] =
      BuildThirdOpacity(link_color);

  VerifyColorMap(colors);
}

TEST_F(BrowserThemePackTest, ProvideUnderlineLinkColor) {
  // If we specify the underline color, it shouldn't try to generate one.x
  std::string color_json = "{ \"ntp_link\": [128, 128, 128],"
                           "  \"ntp_link_underline\": [255, 255, 255],"
                           "  \"ntp_section_link\": [128, 128, 128],"
                           "  \"ntp_section_link_underline\": [255, 255, 255]"
                           "}";
  LoadColorJSON(color_json);

  std::map<int, SkColor> colors = GetDefaultColorMap();
  SkColor link_color = SkColorSetRGB(128, 128, 128);
  SkColor underline_color = SkColorSetRGB(255, 255, 255);
  colors[BrowserThemeProvider::COLOR_NTP_LINK] = link_color;
  colors[BrowserThemeProvider::COLOR_NTP_LINK_UNDERLINE] = underline_color;
  colors[BrowserThemeProvider::COLOR_NTP_SECTION_LINK] = link_color;
  colors[BrowserThemeProvider::COLOR_NTP_SECTION_LINK_UNDERLINE] =
      underline_color;

  VerifyColorMap(colors);
}

TEST_F(BrowserThemePackTest, UseSectionColorAsNTPHeader) {
  std::string color_json = "{ \"ntp_section\": [190, 190, 190] }";
  LoadColorJSON(color_json);

  std::map<int, SkColor> colors = GetDefaultColorMap();
  SkColor ntp_color = SkColorSetRGB(190, 190, 190);
  colors[BrowserThemeProvider::COLOR_NTP_HEADER] = ntp_color;
  colors[BrowserThemeProvider::COLOR_NTP_SECTION] = ntp_color;
  VerifyColorMap(colors);
}

TEST_F(BrowserThemePackTest, ProvideNtpHeaderColor) {
  std::string color_json = "{ \"ntp_header\": [120, 120, 120], "
                           "  \"ntp_section\": [190, 190, 190] }";
  LoadColorJSON(color_json);

  std::map<int, SkColor> colors = GetDefaultColorMap();
  SkColor ntp_header = SkColorSetRGB(120, 120, 120);
  SkColor ntp_section = SkColorSetRGB(190, 190, 190);
  colors[BrowserThemeProvider::COLOR_NTP_HEADER] = ntp_header;
  colors[BrowserThemeProvider::COLOR_NTP_SECTION] = ntp_section;
  VerifyColorMap(colors);
}

TEST_F(BrowserThemePackTest, CanReadTints) {
  std::string tint_json = "{ \"buttons\": [ 0.5, 0.5, 0.5 ] }";
  LoadTintJSON(tint_json);

  color_utils::HSL expected = { 0.5, 0.5, 0.5 };
  color_utils::HSL actual = { -1, -1, -1 };
  EXPECT_TRUE(theme_pack_->GetTint(
      BrowserThemeProvider::TINT_BUTTONS, &actual));
  EXPECT_DOUBLE_EQ(expected.h, actual.h);
  EXPECT_DOUBLE_EQ(expected.s, actual.s);
  EXPECT_DOUBLE_EQ(expected.l, actual.l);
}

TEST_F(BrowserThemePackTest, CanReadDisplayProperties) {
  std::string json = "{ \"ntp_background_alignment\": \"bottom\", "
                     "  \"ntp_background_repeat\": \"repeat-x\", "
                     "  \"ntp_logo_alternate\": 0 }";
  LoadDisplayPropertiesJSON(json);

  int out_val;
  EXPECT_TRUE(theme_pack_->GetDisplayProperty(
      BrowserThemeProvider::NTP_BACKGROUND_ALIGNMENT, &out_val));
  EXPECT_EQ(BrowserThemeProvider::ALIGN_BOTTOM, out_val);

  EXPECT_TRUE(theme_pack_->GetDisplayProperty(
      BrowserThemeProvider::NTP_BACKGROUND_TILING, &out_val));
  EXPECT_EQ(BrowserThemeProvider::REPEAT_X, out_val);

  EXPECT_TRUE(theme_pack_->GetDisplayProperty(
      BrowserThemeProvider::NTP_LOGO_ALTERNATE, &out_val));
  EXPECT_EQ(0, out_val);
}

TEST_F(BrowserThemePackTest, CanParsePaths) {
  std::string tint_json = "{ \"theme_button_background\": \"one\", "
                          "  \"theme_toolbar\": \"two\" }";
  std::map<int, FilePath> out_file_paths;
  ParseImageNames(tint_json, &out_file_paths);

  EXPECT_EQ(2u, out_file_paths.size());
  EXPECT_TRUE(FilePath(FILE_PATH_LITERAL("one")) == out_file_paths[
      ThemeResourcesUtil::GetId("theme_button_background")]);
  EXPECT_TRUE(FilePath(FILE_PATH_LITERAL("two")) ==
            out_file_paths[ThemeResourcesUtil::GetId("theme_toolbar")]);
}

TEST_F(BrowserThemePackTest, InvalidColors) {
  std::string invalid_color = "{ \"toolbar\": [\"dog\", \"cat\", [12]], "
                              "  \"sound\": \"woof\" }";
  LoadColorJSON(invalid_color);
  std::map<int, SkColor> colors = GetDefaultColorMap();
  VerifyColorMap(colors);
}

TEST_F(BrowserThemePackTest, InvalidTints) {
  std::string invalid_tints = "{ \"buttons\": [ \"dog\", \"cat\", [\"x\"]], "
                              "  \"invalid\": \"entry\" }";
  LoadTintJSON(invalid_tints);

  // We shouldn't have a buttons tint, as it was invalid.
  color_utils::HSL actual = { -1, -1, -1 };
  EXPECT_FALSE(theme_pack_->GetTint(BrowserThemeProvider::TINT_BUTTONS,
                                    &actual));
}

TEST_F(BrowserThemePackTest, InvalidDisplayProperties) {
  std::string invalid_properties = "{ \"ntp_background_alignment\": [15], "
                                   "  \"junk\": [15.3] }";
  LoadDisplayPropertiesJSON(invalid_properties);

  int out_val;
  EXPECT_FALSE(theme_pack_->GetDisplayProperty(
      BrowserThemeProvider::NTP_BACKGROUND_ALIGNMENT, &out_val));
}

// TODO(erg): This test should actually test more of the built resources from
// the extension data, but for now, exists so valgrind can test some of the
// tricky memory stuff that BrowserThemePack does.
TEST_F(BrowserThemePackTest, CanBuildAndReadPack) {
  ScopedTempDir dir;
  ASSERT_TRUE(dir.CreateUniqueTempDir());
  FilePath file = dir.path().Append(FILE_PATH_LITERAL("data.pak"));

  // Part 1: Build the pack from an extension.
  {
    FilePath star_gazing_path = GetStarGazingPath();
    Extension extension(star_gazing_path);

    FilePath manifest_path =
        star_gazing_path.AppendASCII("manifest.json");
    std::string error;
    JSONFileValueSerializer serializer(manifest_path);
    scoped_ptr<DictionaryValue> valid_value(
        static_cast<DictionaryValue*>(serializer.Deserialize(&error)));
    EXPECT_EQ("", error);
    ASSERT_TRUE(valid_value.get());
    ASSERT_TRUE(extension.InitFromValue(*valid_value, true, &error));
    ASSERT_EQ("", error);

    scoped_refptr<BrowserThemePack> pack =
        BrowserThemePack::BuildFromExtension(&extension);
    ASSERT_TRUE(pack.get());
    ASSERT_TRUE(pack->WriteToDisk(file));
    VerifyStarGazing(pack.get());
  }

  // Part 2: Try to read back the data pack that we just wrote to disk.
  {
    scoped_refptr<BrowserThemePack> pack =
        BrowserThemePack::BuildFromDataPack(
            file, "mblmlcbknbnfebdfjnolmcapmdofhmme");
    ASSERT_TRUE(pack.get());
    VerifyStarGazing(pack.get());
  }
}
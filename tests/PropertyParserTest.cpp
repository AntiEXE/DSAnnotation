#include <gtest/gtest.h>
#include "PropertyParser.h"

class PropertyParserTest : public ::testing::Test {
protected:
    PropertyParser parser;
};

TEST_F(PropertyParserTest, ParsesSimpleKeyValuePair) {
    nlohmann::json result = parser.parse("key=value");
    EXPECT_EQ(result["key"], "value");
}

TEST_F(PropertyParserTest, ParsesMultipleProperties) {
    nlohmann::json result = parser.parse("key1=value1,key2=value2");
    EXPECT_EQ(result["key1"], "value1");
    EXPECT_EQ(result["key2"], "value2");
}

TEST_F(PropertyParserTest, ParsesBooleanValues) {
    nlohmann::json result = parser.parse("flag1=true,flag2=false");
    EXPECT_EQ(result["flag1"], true);
    EXPECT_EQ(result["flag2"], false);
}

TEST_F(PropertyParserTest, ParsesArrayValues) {
    nlohmann::json result = parser.parse("array=[item1,item2,item3]");
    EXPECT_EQ(result["array"], nlohmann::json::array({"item1", "item2", "item3"}));
}

TEST_F(PropertyParserTest, HandlesEmptyInput) {
    nlohmann::json result = parser.parse("");
    EXPECT_TRUE(result.empty());
}

TEST_F(PropertyParserTest, HandlesMalformedInput) {
    nlohmann::json result = parser.parse("invalidformat");
    EXPECT_TRUE(result.empty());
}
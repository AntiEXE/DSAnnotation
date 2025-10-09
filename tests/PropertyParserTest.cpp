#include <gtest/gtest.h>

#include "DSAnnotation/Parsing/PropertyParser.h"
#include "nlohmann/json.hpp"

class PropertyParserTest : public ::testing::Test {
protected:
    dsannotation::parsing::PropertyParser parser;
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

TEST_F(PropertyParserTest, ParsesCardinalityAsString) {
    nlohmann::json result = parser.parse("cardinality=0..1");
    EXPECT_EQ(result["cardinality"], "0..1");
    EXPECT_TRUE(result["cardinality"].is_string());
}

TEST_F(PropertyParserTest, ParsesVariousCardinalityPatterns) {
    nlohmann::json result = parser.parse("card1=1..1,card2=0..n,card3=1..n,card4=0..0");
    EXPECT_EQ(result["card1"], "1..1");
    EXPECT_EQ(result["card2"], "0..n");
    EXPECT_EQ(result["card3"], "1..n");
    EXPECT_EQ(result["card4"], "0..0");
    
    // Verify all are strings, not numbers
    EXPECT_TRUE(result["card1"].is_string());
    EXPECT_TRUE(result["card2"].is_string());
    EXPECT_TRUE(result["card3"].is_string());
    EXPECT_TRUE(result["card4"].is_string());
}

TEST_F(PropertyParserTest, ParsesRegularNumbersAsNumbers) {
    nlohmann::json result = parser.parse("int=42,float=3.14,negative=-5");
    EXPECT_EQ(result["int"], 42);
    EXPECT_EQ(result["float"], 3.14);
    EXPECT_EQ(result["negative"], -5);
    
    // Verify types
    EXPECT_TRUE(result["int"].is_number_integer());
    EXPECT_TRUE(result["float"].is_number_float());
    EXPECT_TRUE(result["negative"].is_number_integer());
}
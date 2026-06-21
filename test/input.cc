#include "gtest/gtest.h"

#include "input.h"


namespace {

TEST(CLI_INPUT, NonExistingSubcommands) {
  char const *argv1[] = {"rcs", "sca"};
  EXPECT_FALSE(parse_program_input(4, argv1));
  char const *argv2[] = {"rcs", "captu"};
  EXPECT_FALSE(parse_program_input(4, argv2));
  char const *argv3[] = {"rcs", "hellp"};
  EXPECT_FALSE(parse_program_input(4, argv3));
  char const *argv4[] = {"rcs", "h"};
  EXPECT_FALSE(parse_program_input(4, argv4));
  char const *argv5[] = {"rcs", "frequency"};
  EXPECT_FALSE(parse_program_input(4, argv5));
}

// Tests an invalid frequency range input
TEST(CLI_INPUT, InvalidFrequencyRange) {
  char const *argv1[] = {"rcs", "scan", "e105", "wbfm"};
  EXPECT_FALSE(parse_program_input(4, argv1));
  char const *argv2[] = {"rcs", "scan", "105e", "wbfm"};
  EXPECT_FALSE(parse_program_input(4, argv2));
  char const *argv3[] = {"rcs", "scan", "105m", "wbfm"};
  EXPECT_FALSE(parse_program_input(4, argv3));
  char const *argv4[] = {"rcs", "scan", "105--106", "wbfm"};
  EXPECT_FALSE(parse_program_input(4, argv4));
  char const *argv5[] = {"rcs", "scan", "105-106a", "wbfm"};
  EXPECT_FALSE(parse_program_input(4, argv5));
  char const *argv6[] = {"rcs", "scan", "105..6", "wbfm"};
  EXPECT_FALSE(parse_program_input(4, argv6));
  char const *argv7[] = {"rcs", "scan", "105k-198m", "wbfm"};
  EXPECT_FALSE(parse_program_input(4, argv7));
  char const *argv8[] = {"rcs", "scan", "-108M", "wbfm"};
  EXPECT_FALSE(parse_program_input(4, argv8));
  char const *argv9[] = {"rcs", "scan", "105k--198m", "wbfm"};
  EXPECT_FALSE(parse_program_input(4, argv9));
  char const *argv10[] = {"rcs", "scan", "0", "wbfm"};
  EXPECT_FALSE(parse_program_input(4, argv10));
  char const *argv11[] = {"rcs", "scan", " ", "wbfm"};
  EXPECT_FALSE(parse_program_input(4, argv11));
  char const *argv12[] = {"rcs", "scan", "105K", "wbfm"};
  EXPECT_FALSE(parse_program_input(4, argv12));
  char const *argv13[] = {"rcs", "scan", "105e9", "wbfm"};
  EXPECT_FALSE(parse_program_input(4, argv13));
  char const *argv14[] = {"rcs", "scan", "INF", "wbfm"};
  EXPECT_FALSE(parse_program_input(4, argv14));
  char const *argv15[] = {"rcs", "scan", "inf", "wbfm"};
  EXPECT_FALSE(parse_program_input(4, argv15));
  char const *argv16[] = {"rcs", "scan", "INFINITY", "wbfm"};
  EXPECT_FALSE(parse_program_input(4, argv16));
  char const *argv17[] = {"rcs", "scan", "infinity", "wbfm"};
  EXPECT_FALSE(parse_program_input(4, argv17));
  char const *argv18[] = {"rcs", "scan", "NAN", "wbfm"};
  EXPECT_FALSE(parse_program_input(4, argv18));
  char const *argv19[] = {"rcs", "scan", "nan", "wbfm"};
  EXPECT_FALSE(parse_program_input(4, argv19));
  char const *argv20[] = {"rcs", "scan", "0x", "wbfm"};
  EXPECT_FALSE(parse_program_input(4, argv20));
}

// Tests a valid frequency range input
TEST(CLI_INPUT, ValidFrequencyRange) {
  char const *argv1[] = {"rcs", "scan", "105", "wbfm"};
  EXPECT_FALSE(parse_program_input(4, argv1));
  char const *argv2[] = {"rcs", "scan", "105M", "wbfm"};
  EXPECT_FALSE(parse_program_input(4, argv2));
  char const *argv3[] = {"rcs", "scan", "105k", "wbfm"};
  EXPECT_FALSE(parse_program_input(4, argv3));
  char const *argv4[] = {"rcs", "scan", "105-106", "wbfm"};
  EXPECT_FALSE(parse_program_input(4, argv4));
  char const *argv5[] = {"rcs", "scan", "105.1-105.2", "wbfm"};
  EXPECT_FALSE(parse_program_input(4, argv5));
  char const *argv6[] = {"rcs", "scan", "105.123890k-105.2k", "wbfm"};
  EXPECT_FALSE(parse_program_input(4, argv6));
  char const *argv7[] = {"rcs", "scan", "105.1M-105.2k", "wbfm"};
  EXPECT_FALSE(parse_program_input(4, argv7));
  char const *argv8[] = {"rcs", "scan", "105.1k-105.2M", "wbfm"};
  EXPECT_FALSE(parse_program_input(4, argv8));
  char const *argv9[] = {"rcs", "scan", "105.1M-105.2", "wbfm"};
  EXPECT_FALSE(parse_program_input(4, argv9));
  char const *argv10[] = {"rcs", "scan", "105.1-105.2M", "wbfm"};
  EXPECT_FALSE(parse_program_input(4, argv10));
}


}  // namespace

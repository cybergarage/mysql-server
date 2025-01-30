/* Copyright (c) 2016, 2024, Oracle and/or its affiliates.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License, version 2.0,
   as published by the Free Software Foundation.

   This program is designed to work with certain software (including
   but not limited to OpenSSL) that is licensed under separate terms,
   as designated in a particular file or component or in included license
   documentation.  The authors of MySQL hereby grant you an additional
   permission to link the program and your derivative works with the
   separately licensed software that they have either included with
   the program or referenced in the documentation.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License, version 2.0, for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301  USA */

#include <gtest/gtest.h>

#include "my_inttypes.h"
#include "my_sys.h"
#include "mysql/strings/m_ctype.h"
#include "unittest/gunit/benchmark.h"

namespace strings_valid_check_unittest {
// Benchmark testing character valid check function of utf8 charset
static void BM_UTF8_Valid_Check(size_t num_iterations) {
  StopBenchmarkTiming();

  const char *content =
      "MySQL は 1億以上のダウンロード数を誇る、世界"
      "でもっとも普及しているオープンソースデータベースソフトウェアです。"
      "抜群のスピードと信頼性、使いやすさが備わった MySQL は、ダウンタイム"
      "、メンテナンス、管理、サポートに関するさまざまな問題を解決することが"
      "できるため、Web、Web2.0、SaaS、ISV、通信関連企業の 先見的なIT 責任者"
      "の方々から大変な好評を博しています。";
  const int len = strlen(content);
  const CHARSET_INFO *cs = get_charset_by_name("utf8mb4_0900_ai_ci", MYF(0));
  int error = 0;

  StartBenchmarkTiming();
  for (size_t i = 0; i < num_iterations; ++i) {
    cs->cset->well_formed_len(cs, content, content + len, len, &error);
  }
  StopBenchmarkTiming();

  ASSERT_EQ(0, error);
}
BENCHMARK(BM_UTF8_Valid_Check)

static void BM_utf8_Convert_Check(size_t num_iterations) {
  StopBenchmarkTiming();
  // There is a non-ascii minus sign here:
  const char *content =
      "MEDIUMBLOB\n\nA BLOB column with a maximum length of "
      "16,777,215 (224 − 1) bytes.";
  // Visual Studio did not like \u2122, warning C4566: character represented
  //   by universal-character-name '\u2212' cannot be represented
  //   in the current code page (1252)
  // "16,777,215 (224 \u2212 1) bytes.";
  const size_t content_len = strlen(content);
  const CHARSET_INFO *from_cs =
      get_charset_by_name("utf8mb4_0900_ai_ci", MYF(0));
  const CHARSET_INFO *to_cs = get_charset_by_name("utf8mb3_general_ci", MYF(0));

  char buf[4096];
  uint errors;
  StartBenchmarkTiming();
  for (size_t i = 0; i < num_iterations; ++i) {
    size_t len [[maybe_unused]] = my_convert(buf, sizeof(buf), to_cs, content,
                                             content_len, from_cs, &errors);
  }
  StopBenchmarkTiming();
  ASSERT_EQ(0, errors);
}
BENCHMARK(BM_utf8_Convert_Check)

}  // namespace strings_valid_check_unittest

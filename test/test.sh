#!/bin/sh
#
# test_script
# 20220424 Takakazu NAKAMURA
#
# テストプログラムを一通り実行し、結果が想定通りであることを確認します。
#
# 使い方
# 全てのテスト
# ./test.sh
# １つのテストのみ(mtable, calendar, taylor, mpa, 8-queens, mines, mandel からひとつ)
# ./test.sh mtable

TEST_TOTAL=0
TEST_TOTAL_OK=0

TEST_DIR=.
TEST_PROGRAM_DIR=../examples
CMD_DIR=../bin
#CMD_DIR=../bin/macos_x64
#CMD_DIR=../bin/windows_wsl_x64
#CMD_DIR=../bin/raspberry_pi_os
CMD=cb

#############################################

for TEST_NAME in \
	mtable calendar taylor mpa 8-queens mines mandel

do

if [ $# -eq 1 ] ;
then
  if [ $1 != ${TEST_NAME} ];
  then
   continue
  fi
fi


case "${TEST_NAME}" in

"calendar" )
EXPECT_TIME=10
PATTERN="
expect \"yyyymm = \"
send 202204\n
"
;;

"mines" )
EXPECT_TIME=10
PATTERN="
expect \"seed = \"
send 99\n
expect \"xy = \"
send 99\n
expect \"xy = \"
send 11\n
expect \"xy = \"
send 55\n
expect \"xy = \"
send 87\n
expect \"xy = \"
send 77\n
expect \"xy = \"
send 41\n
"
;;

"mandel" )
EXPECT_TIME=300
PATTERN=
;;

* )
EXPECT_TIME=60
PATTERN=
;;

esac

#############################################

#TEST_NAME="mtable"
TEST_TOTAL=$((TEST_TOTAL + 1));

echo "##############################"
echo "#### test ${TEST_TOTAL}. ${TEST_NAME}"

expect -c "
set timeout ${EXPECT_TIME}
spawn ${CMD_DIR}/${CMD} ${TEST_PROGRAM_DIR}/${TEST_NAME}.txt
log_file -noappend ${TEST_DIR}/test_${TEST_NAME}.txt
${PATTERN}
expect \"$\"
expect
log_file
"

#diff -q ${TEST_DIR}/test_${TEST_NAME}.txt ${TEST_DIR}/ans_${TEST_NAME}.txt> /dev/null 2>&1
diff -q ${TEST_DIR}/test_${TEST_NAME}.txt ${TEST_DIR}/ans_${TEST_NAME}.txt
if [ $? -eq 0 ]; then
  echo "test ok."
  TEST_TOTAL_OK=$((TEST_TOTAL_OK + 1));
else
  echo "test ng. test_file = test_${TEST_NAME}.txt"
  diff ${TEST_DIR}/test_${TEST_NAME}.txt ${TEST_DIR}/ans_${TEST_NAME}.txt
fi

echo

#############################################

done


echo "##############################"
echo "all tests had done."
echo "total ok = ${TEST_TOTAL_OK}/${TEST_TOTAL}"
if [ ${TEST_TOTAL_OK} -eq ${TEST_TOTAL} ]; then
  echo "test success."
else
  echo "test failure."
  exit 1
fi

exit 0

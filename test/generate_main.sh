#!/bin/sh

# Note: OS X regex library does not support \+ or \?
TEST_DECL_PATTERN="^[ \t]*\(int\)[ \t]*\(test[a-zA-Z0-9_]*\)[ \t]*[(][ \t]*\(void\)*[ \t]*[)][ \t]*.*"

MAIN_C="main.c"

while [ -n "$1" ]; do
  if [ "$1" == "-o" ]; then
    shift 1
    MAIN_C="$1"
  elif [ -f "$1" ]; then
    FILES="${FILES} $1"
  else
    echo "$0: no such file or directory: $1" >&2
  fi
  shift 1
done

if [ -z "${FILES}" ]; then
  FILES="$(find . -name "*.c" -not -name "${MAIN_C}")"
fi

exec >"${MAIN_C}"

echo "#include <stdio.h>"
sed "s/${TEST_DECL_PATTERN}/extern \1 \2( void );/p;d" ${FILES}
echo "int main(void) {"
echo "  int failures = 0;"
sed "s/${TEST_DECL_PATTERN}/  if( \2() ) { failures++; printf( \"\2 failed\\\\n\" ); } else { printf( \"\2 passed\\\\n\" ); }/p;d" ${FILES}
echo "  return failures;"
echo "}"

exec >&-

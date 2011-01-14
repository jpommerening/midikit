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
echo "int main( int argc, char *argv[] ) {"
echo "  int i, j, failures = 0;"
echo "  struct {"
echo "    char * name;"
echo "    int (*func)(void);"
echo "  } tests[] = {"
sed "s/${TEST_DECL_PATTERN}/    { \"\2\", \&\2 },/p;d" ${FILES}
echo "  };"
echo "  for( i=0; i<(sizeof(tests)/sizeof(tests[0])); i++ ) {"
echo "    printf( \"> Running %s\\\\n\", tests[i].name );"
echo "    if( (tests[i].func)() ) {"
echo "      failures++;"
echo "      printf( \"> Test %s failed.\\\\n\", tests[i].name );"
echo "    } else {"
echo "      printf( \"> Test %s passed.\\\\n\", tests[i].name );"
echo "    }"
echo "  }"
echo "  return failures;"
echo "}"

exec >&-

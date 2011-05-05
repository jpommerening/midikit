#!/bin/bash

STYLESHEET="doxyxml2wiki.xslt"
INDEX="wiki/index.wiki"
CONTENTS="wiki/contents.wiki"

if ! test -x "$(which xml)"; then
  exit
fi

if ! test -d "xml"; then
  exit 1
fi

mkdir -p wiki
rm -f "${CONTENTS}" "${INDEX}"

FILES="$(find xml -name "struct_*.xml")"
for FILE in ${FILES}; do
   OUTPUT="${FILE%.xml}.wiki"
   OUTPUT="wiki/${OUTPUT#xml/}"
   xml tr "${STYLESHEET}" -s "contents=contents" "${FILE}" > "${OUTPUT}"
done

FILES="$(find xml -name "group_*.xml")"
for FILE in ${FILES}; do
   OUTPUT="${FILE%.xml}.wiki"
   OUTPUT="wiki/${OUTPUT#xml/}"
   xml tr "${STYLESHEET}" -s "contents=contents" "${FILE}" > "${OUTPUT}"
   xml tr "${STYLESHEET}" -s "display=contents"  "${FILE}" >> "${CONTENTS}"
done

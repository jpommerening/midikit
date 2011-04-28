#!/bin/bash

STYLESHEET="doxyxml2wiki.xslt"

if ! [ -x "$(which xml)" ]; then
  exit
fi

if ! [ -d "xml" ]; then
  exit 1
fi

FILES="$(find xml -name "struct*.xml")"

mkdir -p wiki

for FILE in ${FILES}; do
   OUTPUT="${FILE%.xml}.wiki"
   OUTPUT="wiki/${OUTPUT#xml/}"
   xml tr "${STYLESHEET}" "${FILE}" > "${OUTPUT}"
done

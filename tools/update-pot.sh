#!/bin/sh
# Regenerate Locale/template.pot from the sources.
#
# This used to be run by hand in Poedit, whose configuration lived in the .pot
# header.  That missed wxTRANSLATE entirely and excluded UpdateChecker.cpp, so
# strings drifted out of the catalogues without anyone noticing.
set -eu

cd "$(dirname "$0")/.."

xgettext \
	--language=C++ \
	--from-code=UTF-8 \
	--keyword=_ \
	--keyword=wxTRANSLATE \
	--keyword=wxPLURAL:1,2 \
	--keyword=wxGETTEXT_IN_CONTEXT:1c,2 \
	--keyword=wxGETTEXT_IN_CONTEXT_PLURAL:1c,2,3 \
	--add-comments=TRANSLATORS \
	--sort-by-file \
	--package-name=Kainote \
	--copyright-holder='Marcin Drob' \
	--msgid-bugs-address='https://github.com/bjakja/Kainote/issues' \
	--output=Locale/template.pot \
	Kainote/*.cpp Kainote/*.h

# xgettext writes placeholder boilerplate into the header and marks it fuzzy,
# which would make Weblate treat the template itself as needing review.
python3 - <<'PY'
import datetime, pathlib, re

p = pathlib.Path("Locale/template.pot")
t = p.read_text(encoding="utf-8")
head, sep, body = t.partition('\n\n')
head = """# Kainote, a subtitle editor.
# Copyright (C) %d Marcin Drob
# This file is distributed under the GNU General Public License v3.0.
#
msgid ""
msgstr ""%s""" % (datetime.date.today().year, head.split('msgstr ""', 1)[1])
head = head.replace('"Language: \\n"', '"Language: en\\n"')
head = head.replace('"Plural-Forms: nplurals=INTEGER; plural=EXPRESSION;\\n"',
                    '"Plural-Forms: nplurals=2; plural=(n != 1);\\n"')
head = re.sub(r'"(PO-Revision-Date|Last-Translator|Language-Team): [^"]*\\n"\n', "", head)
p.write_text(head + sep + body, encoding="utf-8")
PY

echo "Locale/template.pot: $(grep -c '^msgid ' Locale/template.pot) entries"

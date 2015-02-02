#! /bin/sh

# UCLA CS 111 Lab 1 - Test that valid syntax is processed correctly.

# Copyright 2012-2014 Paul Eggert.

# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.

# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.

# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.

tmp=$0-$$.tmp
mkdir "$tmp" || exit

(
cd "$tmp" || exit

cat >test.sh <<'EOF'
echo hello
(sleep 1)
echo hi
EOF

cat >test.exp <<'EOF'
hello
hi
EOF

../profsh -p profile_times test.sh >test.out 2>test.err || exit
cat profile_times | sed '/[0-9]*\.[0-9]\{9\}\ [0-9]*\.[0-9]\{9\}\ [0-9]*\.[0-9]\{6\}\ [0-9]*\.[0-9]\{6\}\ /d' | sed '/\n/d'> output 

test ! -s output || exit

diff -u test.exp test.out || exit
test ! -s test.err || {
  cat test.err
  exit 1
}

) || exit

rm -fr "$tmp"

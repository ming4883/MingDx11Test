#!/bin/sh
cd "$(dirname "$0")"

OPTIONS="--style=allman --preserve-date --recursive --suffix=none --lineend=linux --align-pointer=type --pad-first-paren-out --unpad-paren"
EXCLUDES="--exclude=Builds --exclude=JuceLibraryCode"


./bin/astyle $OPTIONS $EXCLUDES ../../MCDr1/*.h ../../MCDr1/*.inl ../../MCDr1/*.cpp


@echo off
@set OPTIONS=--style=allman --preserve-date --recursive --suffix=none --lineend=linux --align-pointer=type --pad-first-paren-out --unpad-paren
@rem set EXCLUDES=--exclude=Builds --exclude=JuceLibraryCode
@set EXCLUDES=--exclude=Media

.\bin\AStyle.exe %OPTIONS% %EXCLUDES% ..\..\MDK\*.h ..\..\MDK\*.inl ..\..\MDK\*.cpp

@pause

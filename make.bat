@ECHO OFF

:: Try to use lldb
:: Compile incrementally
:: Use precompiled header files

SET CC=clang
SET FLAGS=-Wall -Wextra -Wpedantic -Werror -std=c11

%CC% -c %FLAGS% -o TokenList.o Project\includes\TokenList.c

%CC% -c %FLAGS% -o Scanner.o Project\includes\Scanner.c

%CC% -c %FLAGS% -o Parser.o Project\includes\Parser.c

%CC% -c %FLAGS% -o main.o Project\main.c
%CC% -o Project\main TokenList.o Scanner.o Parser.o main.o

DEL *.o

Project\main

ECHO: && ECHO: && ECHO:

IF ERRORLEVEL 1 (
    ECHO PROGRAM ERROR %ERRORLEVEL%
) ELSE IF ERRORLEVEL 0 (
    ECHO SUCCESS 0
) ELSE (
    ECHO SYSTEM ERROR %ERRORLEVEL%
)
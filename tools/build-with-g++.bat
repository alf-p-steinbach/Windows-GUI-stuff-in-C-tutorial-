@echo off & setlocal
set standard=-std=c++17 -pedantic-errors
g++ %1 -mwindows %standard% -Wall -s -o %~n1

@echo off
cloc "%_home_%\code" --exclude-dir="third_party,generated" --quiet --hide-rate 

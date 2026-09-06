@echo off
pushd "%_config_%"
start 4coder.exe "%_apps_%\4coder\bindings.4coder" "%_apps_%\4coder\config.4coder" -f 16 -F
popd
@shift /0
@echo off
title DC1固件生成、刷写工具箱

set PYTHONPATH=%cd%\Prerequisites\Python27
set PYTHON_SCRIPTS_PATH=%cd%\Prerequisites\Python27\Scripts
set GITPATH=%cd%\Prerequisites\git\cmd

>"%tmp%\t.t" echo;WSH.Echo(/[\u4E00-\u9FFF]/.test(WSH.Arguments(0)))
for /f %%a in ('cscript -nologo -e:jscript "%tmp%\t.t" "%PYTHONPATH%"') do if %%a neq 0 (
Color cf&echo;当前运行环境 %PYTHONPATH%&echo;!!!请不要在中文目录下使用!!!&echo;&pause&EXIT /B) else (Goto Start)

:Start
set PATH=%PYTHONPATH%;%PYTHON_SCRIPTS_PATH%;%GITPATH%;%PATH%
Color f5
MODE con: Cols=100 Lines=30
Set var=0

:Menu
cls
echo.
echo 「 DC1固件生成、刷写工具箱 」
echo.
echo    此工具用于DC1固件的生成与刷写，请将固件配置文件存放于config_yaml
echo    目录内。按照自己的情况修改配置文件中substitutions下面的参数。
echo    然后执行菜单(1)，确认无报错后执行菜单(3)。
echo.
echo    初次使用必须TTL线刷，后续可以通过OTA升级。
echo    菜单(3)升级DC1固件 支持TTL和OTA双模式选择。
echo    TTL刷固件的接线及进入刷写模式方法请参考文档说明。
echo.
echo  菜单:
echo.
echo        (1)  编译固件          (2)  升级编译环境
echo.
echo        (3)  升级DC1固件       (4)  查看日志
echo.
echo        (q)  退出
echo.
echo.
if %var% neq 0 echo 输入无效请重新输入！
Set choice=
Set /p choice=请输入： 
Set "choice=%choice:"=%"
if "%choice:~-1%"=="=" Goto Menu
if "%choice%"=="" Goto Menu
if /i "%choice%" == "1" Set go=Compile&cls&Goto Check
if /i "%choice%" == "2" Set go=Update&cls&Goto Update
if /i "%choice%" == "3" Set go=Upload&cls&Goto Check
if /i "%choice%" == "4" Set go=Logs&cls&Goto Check
if /i "%choice%" == "q" Popd&Exit
Set var=1
Goto Menu
 
:Check
set var2=0
Goto Check1

:Check1
cls
if %var2% neq 0 echo 输入无效请重新输入！
set /p input_source=请输入配置文件名字，比如：livingroom，不需要输入.yaml后缀： 
echo 当前选择的配置文件为「 %input_source%.yaml 」
Set choice1=
Set /p choice1=确认：y  取消：n 返回上级菜单：b  请选择： 
Set "choice1=%choice1:"=%"
if "%choice1:~-1%"=="=" Goto Check1
if "%choice1%"=="" Goto Check1
if /i "%choice1%" == "y" cls&Goto %go%
if /i "%choice1%" == "n" cls&Goto Check
if /i "%choice1%" == "b" cls&Goto Start
Set var2=1
Goto Check1

:Update
::python -m pip uninstall esphome
cd Prerequisites
if exist esphome (echo y|cacls esphome /p everyone:f >nul 2>nul &&rd /s /q esphome) else echo esphome文件夹不存在,跳过...
::rd /s /q esphome
git clone https://github.com/Samuel-0-0/esphome.git
cd esphome
python setup.py build
python setup.py install
python -m pip list
echo.
echo ***更新完成。请重新打开工具箱***
if exist esphome (echo y|cacls esphome /p everyone:f >nul 2>nul &&rd /s /q esphome)
echo.&Pause&EXIT

:Compile
python -m esphome %cd%\config_yaml\%input_source%.yaml compile
echo.&Pause
Goto End

:Logs
python -m esphome %cd%\config_yaml\%input_source%.yaml logs
echo.&Pause
Goto End

:Upload
python -m esphome %cd%\config_yaml\%input_source%.yaml upload
echo.&Pause
Goto End


:End
if "%choice%" neq "" (
    cls
    if "%choice%" neq "3" (
        echo 操作完成 ! 正在返回主菜单...
        if exist %WINDIR%\System32\timeout.exe (timeout /t 2) else (if exist %WINDIR%\System32\choice.exe (choice /t 2 /d y /n >nul) else (ping 127.1 -n 2 >nul))
    )
    Goto Start
)
@shift /0
@echo off
title DC1�̼����ɡ�ˢд������

set PYTHONPATH=%cd%\Prerequisites\Python27
set PYTHON_SCRIPTS_PATH=%cd%\Prerequisites\Python27\Scripts
set GITPATH=%cd%\Prerequisites\git\cmd

>"%tmp%\t.t" echo;WSH.Echo(/[\u4E00-\u9FFF]/.test(WSH.Arguments(0)))
for /f %%a in ('cscript -nologo -e:jscript "%tmp%\t.t" "%PYTHONPATH%"') do if %%a neq 0 (
Color cf&echo;��ǰ���л��� %PYTHONPATH%&echo;!!!�벻Ҫ������Ŀ¼��ʹ��!!!&echo;&pause&EXIT /B) else (Goto Start)

:Start
set PATH=%PYTHONPATH%;%PYTHON_SCRIPTS_PATH%;%GITPATH%;%PATH%
Color f5
MODE con: Cols=100 Lines=30
Set var=0

:Menu
cls
echo.
echo �� DC1�̼����ɡ�ˢд������ ��
echo.
echo    �˹�������DC1�̼���������ˢд���뽫�̼������ļ������config_yaml
echo    Ŀ¼�ڡ������Լ�������޸������ļ���substitutions����Ĳ�����
echo    Ȼ��ִ�в˵�(1)��ȷ���ޱ����ִ�в˵�(3)��
echo.
echo    ����ʹ�ñ���TTL��ˢ����������ͨ��OTA������
echo    �˵�(3)����DC1�̼� ֧��TTL��OTA˫ģʽѡ��
echo    TTLˢ�̼��Ľ��߼�����ˢдģʽ������ο��ĵ�˵����
echo.
echo  �˵�:
echo.
echo        (1)  ����̼�          (2)  �������뻷��
echo.
echo        (3)  ����DC1�̼�       (4)  �鿴��־
echo.
echo        (q)  �˳�
echo.
echo.
if %var% neq 0 echo ������Ч���������룡
Set choice=
Set /p choice=�����룺 
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
if %var2% neq 0 echo ������Ч���������룡
set /p input_source=�����������ļ����֣����磺livingroom������Ҫ����.yaml��׺�� 
echo ��ǰѡ��������ļ�Ϊ�� %input_source%.yaml ��
Set choice1=
Set /p choice1=ȷ�ϣ�y  ȡ����n �����ϼ��˵���b  ��ѡ�� 
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
if exist esphome (echo y|cacls esphome /p everyone:f >nul 2>nul &&rd /s /q esphome) else echo esphome�ļ��в�����,����...
::rd /s /q esphome
git clone https://github.com/Samuel-0-0/esphome.git
cd esphome
python setup.py build
python setup.py install
python -m pip list
echo.
echo ***������ɡ������´򿪹�����***
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
        echo ������� ! ���ڷ������˵�...
        if exist %WINDIR%\System32\timeout.exe (timeout /t 2) else (if exist %WINDIR%\System32\choice.exe (choice /t 2 /d y /n >nul) else (ping 127.1 -n 2 >nul))
    )
    Goto Start
)
@echo off
title Installation Process
echo Starting installation...

REM Check if Python is installed
python --version >nul 2>&1
if %errorlevel% neq 0 (
    echo Python is not installed. Please install Python before running this script.
    pause
    exit /b
)

REM If Python is installed, proceed with upgrading pip
echo Python is installed. Upgrading pip...
python -m pip install --upgrade pip

REM Navigate to the project directory
cd /d "C:\Users\yadhu\Downloads\sadhads\LMS-nosql-main"

REM Install the required Python libraries from requirements.txt
echo Installing Python libraries...
pip install -r requirements.txt

REM Check if library installation was successful
if %errorlevel% neq 0 (
    echo Python libraries installation failed. Exiting...
    pause
    exit /b %errorlevel%
)

REM Optional: Install drivers silently (using a loop if multiple .inf files are present)
cd /d "D:\LMS-nosql-main\dependencies"
echo Installing drivers...
for %%I in (*.inf) do (
    pnputil /add-driver %%I /install
)

REM Check if the drivers installation was successful
if %errorlevel% neq 0 (
    echo Driver installation failed. Exiting...
    pause
    exit /b %errorlevel%
)

REM Optional: Run the server script if needed
echo Starting the server...
python server.py

REM If everything succeeded, show a success message
echo.
echo Installation Completed Successfully!
pause

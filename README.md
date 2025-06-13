# To Compile C++ on Windows:

## Install MinGW-w64 toolchain: Copied from this link which has pictures (https://code.visualstudio.com/docs/cpp/config-mingw#_prerequisites)

1. You can download the latest installer from the MSYS2 page or use this direct link to the [installer](https://github.com/msys2/msys2-installer/releases/download/2024-12-08/msys2-x86_64-20241208.exe).

2. Run the installer and follow the steps of the installation wizard. Note that MSYS2 requires 64 bit Windows 8.1 or newer.

3. In the wizard, choose your desired Installation Folder. Record this directory for later. In most cases, the recommended directory is acceptable. The same applies when you get to setting the start menu shortcuts step. When complete, ensure the Run MSYS2 now box is checked and select Finish. This will open a MSYS2 terminal window for you.

4. In this terminal, install the MinGW-w64 toolchain by running the following command:

   `pacman -S --needed base-devel mingw-w64-ucrt-x86_64-toolchain`

6. Accept the default number of packages in the toolchain group by pressing Enter.

7. Enter `Y` when prompted whether to proceed with the installation.

8. Add the path of your MinGW-w64 bin folder to the Windows PATH environment variable by using the following steps:
    1. In the Windows search bar, type Settings to open your Windows Settings.
    2. Search for Edit environment variables for your account.
    3. In your User variables, select the Path variable and then select Edit.
    4. Select New and add the MinGW-w64 destination folder you recorded during the installation process to the list. If you used the default settings above, then this will be the path: `C:\msys64\ucrt64\bin`.
    5. Select OK, and then select OK again in the Environment Variables window to update the PATH environment variable. You have to reopen any console windows for the updated PATH environment variable to be available.

## Check your MinGW installation

1. To check that your MinGW-w64 tools are correctly installed and available, open a NEW Command Prompt and type:

   `g++ --version`

## Compile:

1. Use a command prompt and navigate to the directory containing RSChecksumCalculator.cpp
2. Execute:

   `g++ -std=c++17 -O3 -o RSChecksumCalculator RSChecksumCalculator.cpp ThreadPool.cpp`

## Run:
In a command prompt, navigate to the directory containing RSChecksumCalculator.exe and call 

`.\RSChecksumCalculator.exe <first TID to calculate> <last TID to calculate> <number of frames to calculate> <number of threads to use, default = 1, max = num cores in your processor>`

Example that calculates frames 0 to 3999 for TID 3575 and 3576 using 4 threads: 

`.\RSChecksumCalculator.exe 3575 3576 4000 4`

Example that calculates frames 0 to 3999 for TID 1 to 10000 using maximum threads (automatically defaults to num cores you have): 

`.\RSChecksumCalculator.exe 1 10000 4000 1000`

Your results should appear in a csv file named combinedMatches.csv and combinedAces.csv. Subsequent runs will overwrite an existing file, so be careful to save your results.

# [If you want to edit using Visual Studio](https://code.visualstudio.com/docs/languages/cpp)

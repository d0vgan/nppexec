import os
import shutil
import subprocess
import tempfile
from datetime import datetime

WINMERGE_EXE = "C:/Progs/Progs/WinMerge64/WinMergeU.exe"
RELATIVE_NPP_FILES_PATH = "NppExec/src/npp_files"
RELATIVE_NPP_PATH = "../notepad-plus-plus-git/master.notepad-plus-plus"
NPP_FILES = [
    "PowerEditor/src/WinControls/DockingWnd/Docking.h",
    "PowerEditor/src/menuCmdID.h",
    "PowerEditor/src/MISC/PluginsManager/Notepad_plus_msgs.h",
    "PowerEditor/src/MISC/PluginsManager/PluginInterface.h",
    "scintilla/include/Sci_Position.h",
    "scintilla/include/Scintilla.h",
    "scintilla/include/BoostRegexSearch.h"
]


def run():
    winmergeExe = os.path.normpath(WINMERGE_EXE)
    if not os.path.isfile(winmergeExe):
        showWarning("WinMerge.exe does not exist: {0}".format(winmergeExe))
        return
    nppFilesDir = os.path.normpath(os.path.join(getPyFileDir(), RELATIVE_NPP_FILES_PATH))
    if not os.path.isdir(nppFilesDir):
        showWarning("Directory does not exist: {0}".format(nppFilesDir))
        return
    tempDir = copyNppFilesToTempDir()
    if tempDir:
        try:
            showMessage("Working directory: {0}".format(tempDir))
            showMessage("Running WinMerge")
            runWinMerge(winmergeExe, tempDir, nppFilesDir)
        finally:
            showMessage("Clean-up")
            cleanup(tempDir)


def copyNppFilesToTempDir() -> str:
    nppDir = os.path.normpath(os.path.join(getPyFileDir(), RELATIVE_NPP_PATH))
    if not os.path.isdir(nppDir):
        showWarning("Directory does not exist: {0}".format(nppDir))
        return ""
    tempDir = ""
    nppFilePaths = []
    for nppFile in NPP_FILES:
        nppFilePath = os.path.normpath(os.path.join(nppDir, nppFile))
        if os.path.isfile(nppFilePath):
            nppFilePaths.append(nppFilePath)
        else:
            showWarning("File does not exist: {0}".format(nppFilePath))
    if len(nppFilePaths) == len(NPP_FILES):
        tempDir = getTempDirForNppFiles()
        os.makedirs(tempDir)
        for nppFilePath in nppFilePaths:
            tempFilePath = os.path.join(tempDir, os.path.basename(nppFilePath))
            shutil.copyfile(nppFilePath, tempFilePath)
    return tempDir


def cleanup(tempDir: str):
    shutil.rmtree(tempDir)


def runWinMerge(winmergeExe: str, srcDir: str, dstDir: str):
    subprocess.run([winmergeExe, "/r", srcDir, dstDir])


def getTempDirForNppFiles() -> str:
    now = datetime.now()
    date_time = now.strftime("%Y%m%d_%H%M%S")
    tempDir = tempfile.gettempdir()
    return os.path.join(tempDir, "npp_files", date_time)


def getPyFileDir() -> str:
    py_file_path = os.path.realpath(__file__)
    return os.path.dirname(py_file_path)


def showMessage(message: str):
    print(message)


def showWarning(message: str):
    print("Warning: {0}".format(message))


run()

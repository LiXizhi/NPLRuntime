---
agent: agent
---
The user may specifiy a list of recently edited files or they are just the reference files or the user may want to sync files in recent git commits. Get the list of these filenames, and search them in following directories:
C:\lxzsrc\NPLRuntime\Client\trunk\ParaEngineClient\
C:\lxzsrc\NPLRuntimeCpOld\NPLRuntime\ParaEngine\
C:\lxzsrc\NPLRuntimeDev\Client\trunk\ParaEngineClient\

one of them may be the current workspace directory (you can auto skip). Use the files under the current workspace directory as the single source of truth. And try to update all files with exact same name under the other directories to keep them in sync. search by filenames in other directories to locate the files to be updated.

Please note, sometimes the two files are almost identical and the diff parts are truly cross-platform code, and we can just copy the file from one directory to the another without any modification. Sometimes, there are platform specific code differences between the two files (since they belong to different cross-platform branches). If you can make both files identical and truly cross-platform, do so. Otherwise, only sync the sections that are truly cross-platform and leave other platform specific code unchanged, so that the differences between the two files in different directories are minimized.




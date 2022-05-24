//-----------------------------------------------------------------------------
// Class: FileDialog.h
// Authors: big
// Emails: onedous@gmail.com
// CreateDate: 2022.05.12
//-----------------------------------------------------------------------------

#include <string>

using namespace std;

class FileDialogiOS
{
public:
    static int callbackId;
    static string activateFile;

    static void open(const string &filter, const int callbackId, const string &activateFile);
    static void openCallback(const string &filepath);
};

//=====================================================================================================
// OnReadyStateChange

// [id(0xfffffd9f)]
//         void OnReadyStateChange(long newState);

struct SFPCOnReadyStateChangeInfoStruct
{    
    NMHDR hdr;
    // [in] newState
    long newState;
};

#define FPCN_ONREADYSTATECHANGE                                (FPCN_FIRST - 255)

//=====================================================================================================

//=====================================================================================================
// OnProgress

// [id(0x000007a6)]
//         void OnProgress(long percentDone);

struct SFPCOnProgressInfoStruct
{    
    NMHDR hdr;
    // [in] percentDone
    long percentDone;
};

#define FPCN_ONPROGRESS                                        (FPCN_FIRST - 256)

//=====================================================================================================

//=====================================================================================================
// FSCommand

// [id(0x00000096)]
//         void FSCommand(
//                         [in] BSTR command, 
//                         [in] BSTR args);

struct SFPCFSCommandInfoStructA
{    
    NMHDR hdr;
    // [in] command
    LPCSTR command;
    // [in] args
    LPCSTR args;
};

struct SFPCFSCommandInfoStructW
{    
    NMHDR hdr;
    // [in] command
    LPCWSTR command;
    // [in] args
    LPCWSTR args;
};

#ifdef UNICODE
    #define SFPCFSCommandInfoStruct                            SFPCFSCommandInfoStructW
#else
    #define SFPCFSCommandInfoStruct                            SFPCFSCommandInfoStructA
#endif // UNICODE

#define FPCN_FSCOMMANDA                       (FPCN_FIRST - 257)
#define FPCN_FSCOMMANDW                       (FPCN_FIRST - 258)

#ifdef UNICODE
    #define FPCN_FSCOMMAND                                         FPCN_FSCOMMANDW
#else
    #define FPCN_FSCOMMAND                                         FPCN_FSCOMMANDA
#endif // UNICODE

//=====================================================================================================

